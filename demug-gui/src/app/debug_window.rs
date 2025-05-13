use crate::app::AppResources;
use crate::debug::{DebuggerControls, DemugDebugData};
use egui::{Align, Button, Color32, Direction, Layout, Sense, TextEdit};
use egui_extras::{Column, TableBuilder};
use libdemug::hardware::cpu::opcodes::OPCODES_STRING;
use libdemug::hardware::cpu::CpuDebugInfo;
use libdemug::hardware::memory::MemoryDebugInfo;
use libdemug::BusDebugInfo;
use std::sync::{Arc, RwLock};
use winit::dpi::PhysicalSize;
use winit::event::WindowEvent;
use winit::window::Window;

const COLOR_RED: Color32 = Color32::from_rgb(0xFF, 0x00, 0x33);
const COLOR_GREEN: Color32 = Color32::from_rgb(0x53, 0xF9, 0x8D);
const COLOR_BLUE: Color32 = Color32::from_rgb(0x1C, 0x71, 0xD8);

pub struct DebuggerWindowState {
    app_resources: Arc<AppResources>,
    egui_renderer: egui_wgpu::Renderer,
    egui_state: egui_winit::State,
    surface: egui_wgpu::wgpu::Surface<'static>,
    surface_config: egui_wgpu::wgpu::SurfaceConfiguration,

    demug_debug_data: DemugDebugData,
    dbg_controls: Arc<RwLock<DebuggerControls>>,
    goto_address_string: String,
    goto_address: bool,
    goto_pc: bool,

    pub window: Arc<Window>,
}

impl DebuggerWindowState {
    pub async fn new(
        window: Arc<Window>, app_resources: Arc<AppResources>,
        dbg_controls: Arc<RwLock<DebuggerControls>>,
    ) -> Self {
        let surface = app_resources.instance.create_surface(window.clone()).unwrap();

        let swapchain_capabilities = surface.get_capabilities(&app_resources.adapter);
        let selected_format = egui_wgpu::wgpu::TextureFormat::Bgra8UnormSrgb;
        let swapchain_format =
            swapchain_capabilities.formats.iter().find(|d| **d == selected_format).unwrap();
        let surface_config = egui_wgpu::wgpu::SurfaceConfiguration {
            usage: egui_wgpu::wgpu::TextureUsages::RENDER_ATTACHMENT,
            format: *swapchain_format,
            width: window.inner_size().width,
            height: window.inner_size().height,
            present_mode: egui_wgpu::wgpu::PresentMode::AutoNoVsync,
            desired_maximum_frame_latency: 0,
            alpha_mode: swapchain_capabilities.alpha_modes[0],
            view_formats: vec![],
        };

        surface.configure(&app_resources.device, &surface_config);

        // Handle egui stuff
        let egui_ctx = egui::Context::default();
        let egui_state = egui_winit::State::new(
            egui_ctx,
            egui::viewport::ViewportId::ROOT,
            &window,
            Some(window.scale_factor() as f32),
            None,
            Some(2 * 1024),
        );
        let egui_renderer =
            egui_wgpu::Renderer::new(&app_resources.device, surface_config.format, None, 1, true);

        let demug_debug_data = DemugDebugData::new_empty();

        Self {
            app_resources,
            egui_renderer,
            egui_state,
            surface,
            surface_config,

            demug_debug_data,
            dbg_controls,
            goto_address_string: String::from(""),
            goto_address: false,
            goto_pc: false,

            window,
        }
    }

    pub fn render(&mut self) {
        let screen_descriptor = egui_wgpu::ScreenDescriptor {
            size_in_pixels: [self.surface_config.width, self.surface_config.height],
            pixels_per_point: self.window.scale_factor() as f32 * 1.0,
        };
        let surface_texture = self.surface.get_current_texture();

        match surface_texture {
            Err(egui_wgpu::wgpu::SurfaceError::Outdated) => {
                // Ignoring outdated to allow resizing and minimization
                println!("wgpu surface outdated");
                return;
            }
            Err(_) => {
                surface_texture.expect("Failed to acquire next swap chain texture");
                return;
            }
            Ok(_) => {}
        };

        let surface_texture = surface_texture.unwrap();
        let surface_view =
            surface_texture.texture.create_view(&egui_wgpu::wgpu::TextureViewDescriptor::default());
        let mut encoder = self
            .app_resources
            .device
            .create_command_encoder(&egui_wgpu::wgpu::CommandEncoderDescriptor { label: None });

        // Drawing egui stuff
        {
            let raw_input = self.egui_state.take_egui_input(&self.window);
            self.egui_state.egui_ctx().begin_pass(raw_input);
            self.render_egui();
            self.egui_state.egui_ctx().set_pixels_per_point(screen_descriptor.pixels_per_point);

            let full_output = self.egui_state.egui_ctx().end_pass();

            self.egui_state.handle_platform_output(&self.window, full_output.platform_output);

            let tris = self
                .egui_state
                .egui_ctx()
                .tessellate(full_output.shapes, self.egui_state.egui_ctx().pixels_per_point());

            for (id, image_deltas) in &full_output.textures_delta.set {
                self.egui_renderer.update_texture(
                    &self.app_resources.device,
                    &self.app_resources.queue,
                    *id,
                    image_deltas,
                );
            }
            self.egui_renderer.update_buffers(
                &self.app_resources.device,
                &self.app_resources.queue,
                &mut encoder,
                &tris,
                &screen_descriptor,
            );

            let render_pass = encoder.begin_render_pass(&egui_wgpu::wgpu::RenderPassDescriptor {
                label: Some("Egui main render pass"),
                color_attachments: &[Some(egui_wgpu::wgpu::RenderPassColorAttachment {
                    view: &surface_view,
                    resolve_target: None,
                    ops: egui_wgpu::wgpu::Operations {
                        load: egui_wgpu::wgpu::LoadOp::Clear(egui_wgpu::wgpu::Color {
                            r: 0.025,
                            g: 0.025,
                            b: 0.025,
                            a: 1.0,
                        }),
                        store: egui_wgpu::wgpu::StoreOp::Store,
                    },
                })],
                depth_stencil_attachment: None,
                timestamp_writes: None,
                occlusion_query_set: None,
            });

            self.egui_renderer.render(
                &mut render_pass.forget_lifetime(),
                &tris,
                &screen_descriptor,
            );
            for tid in &full_output.textures_delta.free {
                self.egui_renderer.free_texture(tid);
            }
        }

        self.app_resources.queue.submit(Some(encoder.finish()));
        surface_texture.present();
    }

    pub fn resize(&mut self, new_size: PhysicalSize<u32>) {
        self.surface_config.width = new_size.width;
        self.surface_config.height = new_size.height;
        self.surface.configure(&self.app_resources.device, &self.surface_config);
    }

    pub fn handle_events(&mut self, event: &WindowEvent) {
        let _ = self.egui_state.on_window_event(&self.window, event);
    }

    pub fn push_info(&mut self, dbg_data: (CpuDebugInfo, BusDebugInfo, MemoryDebugInfo)) {
        self.demug_debug_data = DemugDebugData::from(dbg_data);
    }

    fn render_egui(&mut self) {
        self.control_bar();
        self.program_window();
        self.cpu_window();
    }

    fn control_bar(&mut self) {
        egui::TopBottomPanel::top("top panel").exact_height(50.0).show(
            self.egui_state.egui_ctx(),
            |ui| {
                ui.horizontal_centered(|ui| {
                    let pause = self.dbg_controls.read().unwrap().pause;

                    let play_button = Button::new(if pause { "▶" } else { "⏸" }).fill(
                        Color32::from_rgba_unmultiplied(
                            if pause { COLOR_RED } else { COLOR_GREEN }.r(),
                            if pause { COLOR_RED } else { COLOR_GREEN }.g(),
                            if pause { COLOR_RED } else { COLOR_GREEN }.b(),
                            60,
                        ),
                    );
                    if ui.add_sized([40.0, 40.0], play_button).clicked() {
                        self.dbg_controls.write().unwrap().pause ^= true;
                        self.goto_pc = true;
                    }

                    if ui
                        .add_sized(
                            [40.0, 40.0],
                            Button::new("▶▶").fill(Color32::from_rgba_unmultiplied(
                                COLOR_BLUE.r(),
                                COLOR_BLUE.g(),
                                COLOR_BLUE.b(),
                                60,
                            )),
                        )
                        .clicked()
                    {
                        self.dbg_controls.write().unwrap().goto_next = true;
                        self.goto_pc = true;
                    }
                });
            },
        );
    }

    fn program_window(&mut self) {
        let pc = self.demug_debug_data.cpu.registers.pc as usize;

        egui::Window::new("Program")
            .resizable(true)
            .vscroll(true)
            .default_open(true)
            .default_height(self.window.inner_size().height as f32 - 100.0)
            .default_pos([0.0, 50.0])
            .show(self.egui_state.egui_ctx(), |ui| {
                ui.columns(3, |ui| {
                    ui[0].vertical_centered(|ui| {
                        ui.label("Goto address : ");
                    });
                    ui[1].vertical_centered(|ui| {
                        ui.add(TextEdit::singleline(&mut self.goto_address_string).char_limit(4));
                    });
                    ui[2].vertical_centered(|ui| {
                        if ui.button("Go").clicked() {
                            self.goto_address = true;
                        }
                    });
                });
                
                ui.visuals_mut().faint_bg_color = Color32::from_rgb(0x2B, 0x2B, 0x2B);
                ui.style_mut().interaction.selectable_labels = false;

                let mut table = TableBuilder::new(ui)
                    .sense(Sense::click())
                    .striped(true)
                    .column(Column::auto())
                    .column(Column::remainder())
                    .cell_layout(Layout::centered_and_justified(Direction::LeftToRight));

                if !self.dbg_controls.read().unwrap().pause
                    || self.goto_pc
                {
                    table = table.scroll_to_row(pc, Some(Align::Center));
                    self.goto_pc = false;
                } else if self.goto_address {
                    self.goto_address = false;
                    let str_usize = usize::from_str_radix(self.goto_address_string.as_str(), 16);
                    if let Ok(n) = str_usize {
                        table = table.scroll_to_row(n, Some(Align::Center));
                    }
                }

                table
                    .header(20.0, |mut header| {
                        header.col(|ui| {
                            ui.heading("Address");
                        });
                        header.col(|ui| {
                            ui.heading("Instruction");
                        });
                    })
                    .body(|body| {
                        body.rows(25.0, 0xFFFF, |mut row| {
                            let row_index = row.index();
                            
                            if row.col(|ui| {
                                
                                let str_usize = usize::from_str_radix(self.goto_address_string.as_str(), 16);
                                if let Ok(n) = str_usize {
                                    if n == row_index {
                                        ui.painter().rect_filled(
                                            ui.max_rect(),
                                            2,
                                            Color32::from_rgba_unmultiplied(
                                                COLOR_BLUE.r(),
                                                COLOR_BLUE.g(),
                                                COLOR_BLUE.b(),
                                                15,
                                            ),
                                        );
                                    }
                                }
                                if pc == row_index {
                                    ui.painter().rect_filled(
                                        ui.max_rect(),
                                        2,
                                        Color32::from_rgba_unmultiplied(
                                            COLOR_GREEN.r(),
                                            COLOR_GREEN.g(),
                                            COLOR_GREEN.b(),
                                            8,
                                        ),
                                    );
                                }
                                ui.label(format!("{:#06X}", row_index));
                            }).1.clicked() {println!("Row {row_index} clicked!")};
                            row.col(|ui| {
                                let op = if !self.demug_debug_data.bus.boot_rom_disabled
                                    && row_index < 0x100
                                {
                                    self.demug_debug_data.mem.boot_rom[row_index]
                                } else if row_index < 0x4000 {
                                    self.demug_debug_data.mem.game_rom[row_index]
                                } else {
                                    0x00
                                };
                                if op == 0xCB {
                                    ui.label("PREFIX");
                                } else {
                                    ui.label(OPCODES_STRING[op as usize](0x00));
                                }
                            });
                        });
                    });
            });
    }

    fn cpu_window(&self) {
        egui::Window::new("CPU").default_open(true).show(self.egui_state.egui_ctx(), |ui| {
            let r = &self.demug_debug_data.cpu.registers;
            let reg = |ui: &mut egui::Ui, name: &str, value: u8| {
                ui.horizontal(|ui| {
                    ui.label(format!("{:>3}:", name));
                    ui.label(format!("{:#04X}", value));
                });
            };
            reg(ui, "A", r.a);
            reg(ui, "B", r.b);
            reg(ui, "C", r.c);
            reg(ui, "D", r.d);
            reg(ui, "E", r.e);
            reg(ui, "H", r.h);
            reg(ui, "L", r.l);

            ui.horizontal(|ui| {
                ui.label("SP:");
                ui.label(format!("{:#04X}", r.sp));
            });

            ui.horizontal(|ui| {
                ui.label("PC:");
                ui.label(format!("{:#06X}", r.pc));
            });

            let flags = [("C", 4u8), ("H", 5u8), ("N", 6u8), ("Z", 7u8)];
            ui.horizontal(|ui| {
                for (label, bit) in flags.iter() {
                    let active = (r.f >> bit) & 1 == 1;
                    ui.label(egui::RichText::new(format!(" {} ", label)).color(if active {
                        COLOR_GREEN
                    } else {
                        COLOR_RED
                    }));
                }
            });
        });
    }
}
