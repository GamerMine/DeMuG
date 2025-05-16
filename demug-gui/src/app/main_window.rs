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

use crate::app::{AppStatus, SCALE_FACTOR};
use libdemug::Demug;
use pixels::{wgpu, Pixels, PixelsBuilder, SurfaceTexture};
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
