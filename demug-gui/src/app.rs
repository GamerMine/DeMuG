mod debug_window;
mod main_window;

use crate::app::debug_window::DebuggerWindowState;
use crate::app::main_window::MainWindowState;
use crate::debug::DebuggerControls;
use libdemug::Demug;
use std::process::exit;
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
    DebugDataReady,
}

pub enum AppStatus {
    FrameDrawn,
    DebugDataGathered,
}

pub struct App {
    app_resources: Arc<AppResources>,
    debug_window_state: Option<DebuggerWindowState>,
    main_window_state: Option<MainWindowState>,
    demug: Arc<RwLock<Demug>>,
    tx: Sender<AppStatus>,
    dbg_controls: Arc<RwLock<DebuggerControls>>,
}

impl App {
    pub fn new(demug: Arc<RwLock<Demug>>, tx: Sender<AppStatus>, dbg_controls: Arc<RwLock<DebuggerControls>>) -> Self {
        Self {
            app_resources: Arc::new(pollster::block_on(AppResources::new())),
            debug_window_state: None,
            main_window_state: None,
            demug,
            tx,
            dbg_controls
        }
    }
}

impl ApplicationHandler<DemugEvent> for App {
    fn resumed(&mut self, event_loop: &ActiveEventLoop) {

        let display = event_loop.available_monitors().next().unwrap();
        let debugger_window_attributes = Window::default_attributes()
            .with_title("DeMuGGER")
            .with_inner_size(LogicalSize::new(display.size().width, display.size().height));
        let debugger_window =
            Arc::new(event_loop.create_window(debugger_window_attributes).unwrap());
        let debugger_window_state = pollster::block_on(DebuggerWindowState::new(
            debugger_window.clone(),
            self.app_resources.clone(),
            self.dbg_controls.clone(),
        ));

        let main_window_attributes =
            Window::default_attributes().with_title("DeMuG").with_inner_size(LogicalSize::new(
                libdemug::SCREEN_WIDTH as f64 * SCALE_FACTOR as f64,
                libdemug::SCREEN_HEIGHT as f64 * SCALE_FACTOR as f64,
            ));
        let main_window = Arc::new(event_loop.create_window(main_window_attributes).unwrap());
        let main_window_state = pollster::block_on(MainWindowState::new(
            main_window.clone(),
            self.demug.clone(),
            self.tx.clone(),
        ));

        self.debug_window_state = Some(debugger_window_state);
        self.main_window_state = Some(main_window_state);
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
            DemugEvent::DebugDataReady => {
                if let Some(dbg_window_main) = self.debug_window_state.as_mut() {
                    dbg_window_main.push_info(self.demug.read().unwrap().gather_debug_info());
                }
            }
        }
    }

    fn window_event(
        &mut self, event_loop: &ActiveEventLoop, window_id: WindowId, event: WindowEvent,
    ) {
        let main_window_state = match self.main_window_state.as_mut() {
            Some(window) => window,
            None => return,
        };

        if let Some(dbg_window_state) = self.debug_window_state.as_mut() {
            if window_id == dbg_window_state.window.id() {
                dbg_window_state.handle_events(&event);
            }
        }

        match event {
            WindowEvent::CloseRequested => {
                if window_id == main_window_state.window.id() {
                    self.debug_window_state = None;
                    self.main_window_state  = None;
                    event_loop.exit();
                    exit(0);
                } else if let Some(dbg_window_state) = self.debug_window_state.as_mut() {
                    if window_id == dbg_window_state.window.id() {
                        self.debug_window_state = None;
                        return;
                    }
                }
            }
            WindowEvent::RedrawRequested => {
                if window_id == main_window_state.window.id() {
                    main_window_state.render();
                } else if let Some(dbg_window_state) = self.debug_window_state.as_mut() {
                    if window_id == dbg_window_state.window.id() {
                        dbg_window_state.render();
                        dbg_window_state.window.request_redraw();
                    }
                }
            }
            WindowEvent::Resized(new_size) => {
                if let Some(dbg_window_state) = self.debug_window_state.as_mut() {
                    if window_id == dbg_window_state.window.id() {
                        dbg_window_state.resize(new_size);
                    }
                }
            }
            _ => {}
        }
    }
}

pub struct AppResources {
    instance: egui_wgpu::wgpu::Instance,
    adapter: egui_wgpu::wgpu::Adapter,
    queue: egui_wgpu::wgpu::Queue,
    device: egui_wgpu::wgpu::Device,
}

impl AppResources {
    pub async fn new() -> Self {
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

        Self {
            instance,
            adapter,
            queue,
            device,
        }
    }
}