/*
 *           ____
 *          /\  _`\                                       /'\_/`\  __
 *          \ \ \L\_\     __      ___ ___      __   _ __ /\      \/\_\    ___      __
 *           \ \ \L_L   /'__`\  /' __` __`\  /'__`\/\`'__\ \ \__\ \/\ \ /' _ `\  /'__`\
 *            \ \ \/, \/\ \L\.\_/\ \/\ \/\ \/\  __/\ \ \/ \ \ \_/\ \ \ \/\ \/\ \/\  __/
 *             \ \____/\ \__/.\_\ \_\ \_\ \_\ \____\\ \_\  \ \_\\ \_\ \_\ \_\ \_\ \____\
 *              \/___/  \/__/\/_/\/_/\/_/\/_/\/____/ \/_/   \/_/ \/_/\/_/\/_/\/_/\/____/
 *  
 *      Copyright (C) 2025 GamerMine
 *  
 *      This program is free software: you can redistribute it and/or modify
 *      it under the terms of the GNU General Public License as published by
 *      the Free Software Foundation, either version 3 of the License, or
 *      (at your option) any later version.
 *  
 *      This program is distributed in the hope that it will be useful,
 *      but WITHOUT ANY WARRANTY; without even the implied warranty of
 *      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *      GNU General Public License for more details.
 *  
 *      You should have received a copy of the GNU General Public License
 *      along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

use crate::app::{App, AppStatus, DemugEvent};
use crate::debug::DebuggerControls;
use libdemug::Demug;
use std::sync::{mpsc, Arc, RwLock};
use std::thread;
use winit::event_loop::{ControlFlow, EventLoop};

mod app;
mod debug;

fn main() {
    pollster::block_on(run());
}

async fn run() {
    let demug = Demug::init();
    let dbg_controls = Arc::new(RwLock::new(DebuggerControls {
        pause: true,
        goto_next: false,
        breakpoints: Vec::new(),
    }));
    let event_loop = EventLoop::<DemugEvent>::with_user_event().build().unwrap();

    let (tx, rx) = mpsc::channel::<AppStatus>();
    let event_loop_proxy = event_loop.create_proxy();
    let demug_clone = demug.clone();
    let dbg_controls_clone = dbg_controls.clone();

    let h = thread::spawn(move || {
        let mut pause_moment = false;
        loop {
            let mut frame_ready = false;

            let mut controls = dbg_controls_clone.write().unwrap();
            if let Ok(demug) = demug_clone.read() {
                let pc = demug.gather_cpu_debug().registers.pc;
                if controls.breakpoints.contains(&pc) {
                    controls.pause = true;
                }

                if !controls.pause || controls.goto_next {
                    if controls.goto_next {
                        if event_loop_proxy.send_event(DemugEvent::DebugDataReady).is_err() {
                            break;
                        }
                        controls.goto_next = false;
                    } else {
                        pause_moment = false;
                    }

                    frame_ready = demug.step();
                } else {
                    if !pause_moment
                        && event_loop_proxy.send_event(DemugEvent::DebugDataReady).is_err()
                    {
                        break;
                    }
                    pause_moment = true;
                }
            }
            drop(controls);
            
            if frame_ready {
                if event_loop_proxy.send_event(DemugEvent::DebugDataReady).is_err() {
                    break;
                }
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

    let mut app = App::new(demug, tx, dbg_controls);
    let _ = event_loop.run_app(&mut app);
    let _ = h.join();
}
