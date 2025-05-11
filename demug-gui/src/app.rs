use libdemug::Demug;
use std::num::NonZeroU32;
use std::rc::Rc;
use std::sync::mpsc::Sender;
use std::sync::{Arc, RwLock};
use winit::application::ApplicationHandler;
use winit::dpi::LogicalSize;
use winit::event::WindowEvent;
use winit::event_loop::ActiveEventLoop;
use winit::window::{Window, WindowId};

const SCALE_FACTOR: f32 = 5.0;

pub enum DemugEvent {
    FrameReady,
}

pub enum AppStatus {
    FrameDrawn,
}

pub struct App {
    window: Option<Rc<Window>>,
    soft_buffer_ctx: Option<softbuffer::Context<Rc<Window>>>,
    render_surface: Option<softbuffer::Surface<Rc<Window>, Rc<Window>>>,
    demug: Arc<RwLock<Demug>>,
    tx: Sender<AppStatus>,
}

impl App {
    pub fn new(demug: Arc<RwLock<Demug>>, tx: Sender<AppStatus>) -> Self {
        Self {
            window: None,
            soft_buffer_ctx: None,
            render_surface: None,
            demug,
            tx,
        }
    }
}

impl ApplicationHandler<DemugEvent> for App {
    fn resumed(&mut self, event_loop: &ActiveEventLoop) {
        let window_attributes =
            Window::default_attributes().with_title("DeMuG").with_inner_size(LogicalSize::new(
                libdemug::SCREEN_WIDTH as f64 * SCALE_FACTOR as f64,
                libdemug::SCREEN_HEIGHT as f64 * SCALE_FACTOR as f64,
            ));

        let window = Rc::new(event_loop.create_window(window_attributes).unwrap());

        let context = softbuffer::Context::new(window.clone()).unwrap();
        let surface = softbuffer::Surface::new(&context, window.clone()).unwrap();

        self.soft_buffer_ctx = Some(context);
        self.render_surface = Some(surface);
        self.window = Some(window);
    }

    fn user_event(&mut self, event_loop: &ActiveEventLoop, event: DemugEvent) {
        let window = match self.window.as_ref() {
            Some(window) => window,
            None => return,
        };

        match event {
            DemugEvent::FrameReady => {
                window.request_redraw();
            }
        }
    }

    fn window_event(
        &mut self, event_loop: &ActiveEventLoop, window_id: WindowId, event: WindowEvent,
    ) {
        match event {
            WindowEvent::CloseRequested => event_loop.exit(),
            WindowEvent::RedrawRequested => {
                if let Some(surface) = &mut self.render_surface {
                    surface
                        .resize(
                            NonZeroU32::new(libdemug::SCREEN_WIDTH as u32).unwrap(),
                            NonZeroU32::new(libdemug::SCREEN_HEIGHT as u32).unwrap(),
                        )
                        .unwrap();

                    let frame = self.demug.read().unwrap().get_frame();
                    let mut buffer = surface.buffer_mut().unwrap();
                    for index in 0..(libdemug::SCREEN_WIDTH as u32 * libdemug::SCREEN_HEIGHT as u32)
                    {
                        let pixel = frame[index as usize];
                        buffer[index as usize] =
                            pixel.b as u32 | ((pixel.g as u32) << 8) | ((pixel.r as u32) << 16);
                    }

                    buffer.present().unwrap();
                    self.tx.send(AppStatus::FrameDrawn).unwrap();
                }
            }
            _ => {}
        }
    }
}
