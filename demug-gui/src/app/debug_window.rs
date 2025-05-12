use crate::app::AppResources;
use crate::debug::{DebuggerControls, DemugDebugData};
use egui::{Align, Button, Color32, Direction, Layout};
use egui_extras::{Column, TableBuilder};
use libdemug::hardware::cpu::CpuDebugInfo;
use libdemug::hardware::memory::MemoryDebugInfo;
use libdemug::{BusDebugInfo, Demug};
use std::sync::{Arc, RwLock};
use winit::dpi::PhysicalSize;
use winit::event::WindowEvent;
use winit::window::Window;

const COLOR_RED: Color32 = Color32::from_rgb(0xFF, 0x00, 0x33);
const COLOR_GREEN: Color32 = Color32::from_rgb(0x53, 0xF9, 0x8D);

pub struct DebuggerWindowState {
    demug: Arc<RwLock<Demug>>,
    app_resources: Arc<AppResources>,
    egui_renderer: egui_wgpu::Renderer,
    egui_state: egui_winit::State,
    surface: egui_wgpu::wgpu::Surface<'static>,
    surface_config: egui_wgpu::wgpu::SurfaceConfiguration,

    demug_debug_data: DemugDebugData,
    dbg_controls: Arc<RwLock<DebuggerControls>>,

    pub window: Arc<Window>,
}

impl DebuggerWindowState {
    pub async fn new(
        window: Arc<Window>, demug: Arc<RwLock<Demug>>, app_resources: Arc<AppResources>, dbg_controls: Arc<RwLock<DebuggerControls>>
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
            desired_maximum_frame_latency: 2,
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
        ); // FIXME: Why ? (Apparently tht is default dimension ?
        let egui_renderer =
            egui_wgpu::Renderer::new(&app_resources.device, surface_config.format, None, 1, true);

        let demug_debug_data = DemugDebugData::new_empty();

        Self {
            demug,
            app_resources,
            egui_renderer,
            egui_state,
            surface,
            surface_config,

            demug_debug_data,
            dbg_controls,

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

    fn render_egui(&self) {
        egui::TopBottomPanel::top("top panel").exact_height(50.0).show(self.egui_state.egui_ctx(), |ui| {
            ui.horizontal_centered(|ui| {
                if ui.add_sized([40.0, 40.0], Button::new("▶")).clicked() {
                    self.dbg_controls.write().unwrap().pause ^= true;
                }
                if ui.add_sized([40.0, 40.0], Button::new("▶▶")).clicked() {
                    self.dbg_controls.write().unwrap().goto_next = true;
                }
            });
        });
        egui::Window::new("Program")
            .resizable(true)
            .vscroll(true)
            .default_open(true)
            .default_pos([0.0, 0.0])
            .default_height(self.window.inner_size().height as f32)
            .show(self.egui_state.egui_ctx(), |ui| {
                ui.visuals_mut().faint_bg_color = Color32::from_rgb(0x2B, 0x2B, 0x2B);
                TableBuilder::new(ui)
                    .striped(true)
                    .column(Column::auto())
                    .column(Column::remainder())
                    .cell_layout(Layout::centered_and_justified(Direction::LeftToRight))
                    .scroll_to_row(self.demug_debug_data.cpu.registers.pc as usize, Some(Align::Min))
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
                            row.col(|ui| {
                                if self.demug_debug_data.cpu.registers.pc as usize == row_index {
                                    ui.painter().rect_filled(ui.max_rect(), 2, Color32::from_rgba_unmultiplied(COLOR_GREEN.r(), COLOR_GREEN.g(), COLOR_GREEN.b(), 8));
                                }
                                ui.label(format!("{:#06X}", row_index));
                            });
                            row.col(|ui| {
                                //ui.painter().rect_filled(ui.max_rect(), 2, bg_color);
                                ui.label("NOP");
                            });
                        });
                    });
            });
        egui::Window::new("CPU Registers").default_open(true).show(
            self.egui_state.egui_ctx(),
            |ui| {
                ui.horizontal(|ui| {
                    ui.label("A:  ");
                    ui.add(egui::Label::new(format!("{:#04X}", self.demug_debug_data.cpu.registers.a)));
                });
                ui.horizontal(|ui| {
                    ui.label("B:  ");
                    ui.add(egui::Label::new(format!("{:#04X}", self.demug_debug_data.cpu.registers.b)));
                });
                ui.horizontal(|ui| {
                    ui.label("C:  ");
                    ui.add(egui::Label::new(format!("{:#04X}", self.demug_debug_data.cpu.registers.c)));
                });
                ui.horizontal(|ui| {
                    ui.label("D:  ");
                    ui.add(egui::Label::new(format!("{:#04X}", self.demug_debug_data.cpu.registers.d)));
                });
                ui.horizontal(|ui| {
                    ui.label("E:  ");
                    ui.add(egui::Label::new(format!("{:#04X}", self.demug_debug_data.cpu.registers.e)));
                });
                ui.horizontal(|ui| {
                    ui.label("H:  ");
                    ui.add(egui::Label::new(format!("{:#04X}", self.demug_debug_data.cpu.registers.h)));
                });
                ui.horizontal(|ui| {
                    ui.label("L:  ");
                    ui.add(egui::Label::new(format!("{:#04X}", self.demug_debug_data.cpu.registers.l)));
                });
                ui.horizontal(|ui| {
                    ui.label("SP:  ");
                    ui.add(egui::Label::new(format!("{:#04X}", self.demug_debug_data.cpu.registers.sp)));
                });
                ui.horizontal(|ui| {
                    ui.label("PC: ");
                    ui.add(egui::Label::new(format!("{:#06X}", self.demug_debug_data.cpu.registers.pc)));
                });
            },
        );
        egui::Window::new("CPU Flags").default_open(true).show(
            self.egui_state.egui_ctx(),
            |ui| {
                let c = self.demug_debug_data.cpu.registers.f >> 4 & 0x1;
                let h = self.demug_debug_data.cpu.registers.f >> 5 & 0x1;
                let n = self.demug_debug_data.cpu.registers.f >> 6 & 0x1;
                let z = self.demug_debug_data.cpu.registers.f >> 7 & 0x1;

                ui.horizontal(|ui| {
                    ui.label(egui::RichText::new(" C ").color(if c == 0b1 { COLOR_GREEN } else { COLOR_RED }));
                    ui.label(egui::RichText::new(" H ").color(if h == 0b1 { COLOR_GREEN } else { COLOR_RED }));
                    ui.label(egui::RichText::new(" N ").color(if n == 0b1 { COLOR_GREEN } else { COLOR_RED }));
                    ui.label(egui::RichText::new(" Z ").color(if z == 0b1 { COLOR_GREEN } else { COLOR_RED }));
                });
            },
        );
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
}