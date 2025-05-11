use crate::app::{App, AppStatus, DemugEvent};
use libdemug::Demug;
use std::path::PathBuf;
use std::sync::mpsc;
use std::thread;
use winit::event_loop::{ControlFlow, EventLoop};

mod app;
mod debug;

fn main() {
    let demug = Demug::init();
    let event_loop = EventLoop::<DemugEvent>::with_user_event().build().unwrap();

    demug.write().unwrap().insert_cartridge(PathBuf::from("./demug-gui/resources/Tetris.gb"));

    let (tx, rx) = mpsc::channel::<AppStatus>();
    let event_loop_proxy = event_loop.create_proxy();
    let demug_clone = demug.clone();
    let h = thread::spawn(move || {
        loop {
            let d = demug_clone.read();
            let mut frame_ready: bool = false;

            if let Ok(demug) = &d {
                frame_ready = demug.step();
            }

            if frame_ready {
                drop(d);
                if event_loop_proxy.send_event(DemugEvent::FrameReady).is_err() {
                    break;
                }
                if rx.recv().is_err() {
                    break;
                }
            }
        }
    });

    event_loop.set_control_flow(ControlFlow::Wait);

    let mut app = App::new(demug, tx);
    let _ = event_loop.run_app(&mut app);
    let _ = h.join();
}
