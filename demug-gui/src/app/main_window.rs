use crate::app::{AppStatus, SCALE_FACTOR};
use libdemug::Demug;
use pixels::{Pixels, PixelsBuilder, SurfaceTexture, wgpu};
use std::sync::mpsc::Sender;
use std::sync::{Arc, RwLock};
use winit::window::Window;

pub struct MainWindowState {
    demug: Arc<RwLock<Demug>>,
    tx: Sender<AppStatus>,
    render_buffer: Pixels<'static>,

    pub window: Arc<Window>,
}

impl MainWindowState {
    pub async fn new(
        window: Arc<Window>, demug: Arc<RwLock<Demug>>, tx: Sender<AppStatus>,
    ) -> Self {
        let surface_texture = SurfaceTexture::new(
            libdemug::SCREEN_WIDTH as u32 * SCALE_FACTOR as u32,
            libdemug::SCREEN_HEIGHT as u32 * SCALE_FACTOR as u32,
            window.clone(),
        );
        let pixels = PixelsBuilder::new(
            libdemug::SCREEN_WIDTH as u32,
            libdemug::SCREEN_HEIGHT as u32,
            surface_texture,
        )
        .enable_vsync(false)
        .blend_state(wgpu::BlendState::REPLACE)
        .build()
        .unwrap();

        Self {
            demug,
            tx,
            window,
            render_buffer: pixels,
        }
    }

    pub fn render(&mut self) {
        let mut buff = Vec::<u8>::new();
        for pixel in self.demug.read().unwrap().get_frame() {
            buff.push(pixel.r);
            buff.push(pixel.g);
            buff.push(pixel.b);
            buff.push(0xFF);
        }
        let frame = self.render_buffer.frame_mut();
        frame[..buff.len()].copy_from_slice(buff.as_slice());

        self.render_buffer.render().unwrap();
        self.tx.send(AppStatus::FrameDrawn).unwrap();
    }
}
