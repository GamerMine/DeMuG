mod debug_window;
mod main_window;

use crate::app::debug_window::DebuggerWindowState;
use crate::app::main_window::MainWindowState;
use libdemug::Demug;
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
    main_window_state: Option<MainWindowState>,
    debug_window_state: Option<DebuggerWindowState>,
    demug: Arc<RwLock<Demug>>,
    tx: Sender<AppStatus>,
}

impl App {
    pub fn new(demug: Arc<RwLock<Demug>>, tx: Sender<AppStatus>) -> Self {
        Self {
            main_window_state: None,
            debug_window_state: None,
            demug,
            tx,
        }
    }
}

impl ApplicationHandler<DemugEvent> for App {
    fn resumed(&mut self, event_loop: &ActiveEventLoop) {
        /*let main_window_attributes =
            Window::default_attributes().with_title("DeMuG").with_inner_size(LogicalSize::new(
                libdemug::SCREEN_WIDTH as f64 * SCALE_FACTOR as f64,
                libdemug::SCREEN_HEIGHT as f64 * SCALE_FACTOR as f64,
            ));
        let main_window = Arc::new(event_loop.create_window(main_window_attributes).unwrap());
        let main_window_state = pollster::block_on(MainWindowState::new(
            main_window.clone(),
            self.demug.clone(),
            self.tx.clone(),
        ));*/

        let debugger_window_attributes = Window::default_attributes()
            .with_title("DeMuGGER")
            .with_inner_size(LogicalSize::new(1280.0, 720.0));
        let debugger_window =
            Arc::new(event_loop.create_window(debugger_window_attributes).unwrap());
        let debugger_window_state = pollster::block_on(DebuggerWindowState::new(
            debugger_window.clone(),
            self.demug.clone(),
        ));

        //self.main_window_state = Some(main_window_state);
        self.debug_window_state = Some(debugger_window_state);
    }

    fn user_event(&mut self, _event_loop: &ActiveEventLoop, event: DemugEvent) {
        let main_window_state = match self.main_window_state.as_ref() {
            Some(window) => window,
            None => return,
        };

        match event {
            DemugEvent::FrameReady => {
                main_window_state.window.request_redraw();
            }
        }
    }

    fn window_event(
        &mut self, event_loop: &ActiveEventLoop, window_id: WindowId, event: WindowEvent,
    ) {
        /*let main_window_state = match self.main_window_state.as_mut() {
            Some(window) => window,
            None => return,
        };*/
        let debugger_window_state = match self.debug_window_state.as_mut() {
            Some(window) => window,
            None => return,
        };

        match event {
            WindowEvent::CloseRequested => event_loop.exit(),
            WindowEvent::RedrawRequested => {
                /*if window_id == main_window_state.window.id() {
                    main_window_state.render();
                } else */if window_id ==  debugger_window_state.window.id() {
                    debugger_window_state.render();
                    debugger_window_state.window.request_redraw();
                }
            }
            WindowEvent::Resized(new_size) => {
                if window_id == debugger_window_state.window.id() {
                    debugger_window_state.resize(new_size);
                }
            }
            _ => {}
        }
    }
}
