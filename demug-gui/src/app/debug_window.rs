use libdemug::Demug;
use std::sync::{Arc, RwLock};
use winit::dpi::PhysicalSize;
use winit::window::Window;

pub struct DebuggerWindowState {
    demug: Arc<RwLock<Demug>>,
    device: egui_wgpu::wgpu::Device,
    queue: egui_wgpu::wgpu::Queue,
    surface: egui_wgpu::wgpu::Surface<'static>,
    surface_config: egui_wgpu::wgpu::SurfaceConfiguration,
    egui_state: egui_winit::State,
    egui_renderer: egui_wgpu::Renderer,

    pub window: Arc<Window>,
}

impl DebuggerWindowState {
    pub async fn new(window: Arc<Window>, demug: Arc<RwLock<Demug>>) -> Self {
        let instance =
            egui_wgpu::wgpu::Instance::new(&egui_wgpu::wgpu::InstanceDescriptor::default());
        let adapter = instance
            .request_adapter(&egui_wgpu::wgpu::RequestAdapterOptions::default())
            .await
            .unwrap();
        let (device, queue) = adapter
            .request_device(&egui_wgpu::wgpu::DeviceDescriptor::default(), None)
            .await
            .unwrap();
        let surface = instance.create_surface(window.clone()).unwrap();

        let swapchain_capabilities = surface.get_capabilities(&adapter);
        let selected_format = egui_wgpu::wgpu::TextureFormat::Bgra8UnormSrgb;
        let swapchain_format =
            swapchain_capabilities.formats.iter().find(|d| **d == selected_format).unwrap();
        let surface_config = egui_wgpu::wgpu::SurfaceConfiguration {
            usage: egui_wgpu::wgpu::TextureUsages::RENDER_ATTACHMENT,
            format: *swapchain_format,
            width: window.inner_size().width,
            height: window.inner_size().height,
            present_mode: egui_wgpu::wgpu::PresentMode::AutoVsync,
            desired_maximum_frame_latency: 2,
            alpha_mode: swapchain_capabilities.alpha_modes[0],
            view_formats: vec![],
        };

        surface.configure(&device, &surface_config);

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
        let egui_renderer = egui_wgpu::Renderer::new(&device, surface_config.format, None, 1, true);

        Self {
            demug,
            device,
            queue,
            surface,
            surface_config,
            egui_state,
            egui_renderer,
            window,
        }
    }

    pub fn render(&mut self) {
        let screen_descriptor = egui_wgpu::ScreenDescriptor {
            size_in_pixels: [self.surface_config.width, self.surface_config.height],
            pixels_per_point: self.window.scale_factor() as f32,
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
            .device
            .create_command_encoder(&egui_wgpu::wgpu::CommandEncoderDescriptor { label: None });

        // Drawing egui stuff
        let raw_input = self.egui_state.take_egui_input(&self.window);
        self.egui_state.egui_ctx().begin_pass(raw_input);

        egui::Window::new("DeMuGGER").resizable(true).vscroll(true).default_open(false).show(
            self.egui_state.egui_ctx(),
            |ui| {
                ui.label("Label!");
            },
        );

        self.egui_state.egui_ctx().set_pixels_per_point(screen_descriptor.pixels_per_point);

        let full_output = self.egui_state.egui_ctx().end_pass();

        self.egui_state.handle_platform_output(&self.window, full_output.platform_output);

        let tris = self
            .egui_state
            .egui_ctx()
            .tessellate(full_output.shapes, self.egui_state.egui_ctx().pixels_per_point());

        for (id, image_deltas) in &full_output.textures_delta.set {
            self.egui_renderer.update_texture(&self.device, &self.queue, *id, image_deltas);
        }
        self.egui_renderer.update_buffers(
            &self.device,
            &self.queue,
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
                    load: egui_wgpu::wgpu::LoadOp::Load,
                    store: egui_wgpu::wgpu::StoreOp::Store,
                },
            })],
            depth_stencil_attachment: None,
            timestamp_writes: None,
            occlusion_query_set: None,
        });

        self.egui_renderer.render(&mut render_pass.forget_lifetime(), &tris, &screen_descriptor);
        for tid in &full_output.textures_delta.free {
            self.egui_renderer.free_texture(tid);
        }
    }
    
    pub fn resize(&mut self, new_size: PhysicalSize<u32>) {
        self.surface_config.width = new_size.width;
        self.surface_config.height = new_size.height;
        self.surface.configure(&self.device, &self.surface_config);
    }
}
