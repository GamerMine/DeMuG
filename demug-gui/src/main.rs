use raylib::color::Color;
use raylib::consts::PixelFormat;
use raylib::drawing::RaylibDraw;
use raylib::math::{Rectangle, Vector2};
use raylib::prelude::Image;
use raylib::texture::RaylibTexture2D;
use std::path::PathBuf;

const SCALE_FACTOR: f32 = 5.0;

fn main() {
    let demug = libdemug::Demug::init();
    let (mut rl, thread) = raylib::init()
        .size(
            (libdemug::SCREEN_WIDTH as u64 * SCALE_FACTOR as u64) as i32,
            (libdemug::SCREEN_HEIGHT as u64 * SCALE_FACTOR as u64) as i32,
        )
        .title("DeMuG")
        .build();
    let mut game_render = Image::gen_image_color(
        libdemug::SCREEN_WIDTH as i32,
        libdemug::SCREEN_HEIGHT as i32,
        Color::BLACK,
    );

    rl.set_target_fps(60);

    game_render.set_format(PixelFormat::PIXELFORMAT_UNCOMPRESSED_R8G8B8);
    demug.borrow_mut().disable_boot_rom(false);
    demug.borrow_mut().insert_cartridge(PathBuf::from("./demug-gui/resources/instr_timing.gb"));

    let mut should_exit = false;

    if let Ok(mut texture) = rl.load_texture_from_image(&thread, &game_render) {
        while !should_exit {
            should_exit = rl.window_should_close();

            demug.borrow().step_frame();

            if let Err(err) = texture.update_texture(demug.borrow().get_frame().as_slice()) {
                println!("Error while updating texture: {err}");
            }

            let mut d = rl.begin_drawing(&thread);
            d.clear_background(Color::BLACK);

            d.draw_texture_pro(
                &texture,
                Rectangle::new(0.0, 0.0, texture.width as f32, texture.height as f32),
                Rectangle::new(
                    0.0,
                    0.0,
                    texture.width as f32 * SCALE_FACTOR,
                    texture.height as f32 * SCALE_FACTOR,
                ),
                Vector2::new(0.0, 0.0),
                0.0,
                Color::WHITE,
            );
            d.draw_fps(5, 5);
        }
    }
}
