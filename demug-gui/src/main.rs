use raylib::color::Color;
use raylib::consts::{KeyboardKey, PixelFormat};
use raylib::drawing::RaylibDraw;
use raylib::math::{Rectangle, Vector2};
use raylib::prelude::Image;
use raylib::texture::RaylibTexture2D;

const SCALE_FACTOR: f32 = 5.0;

fn main() {
    let demug = libdemug::Demug::init();
    let (mut rl, thread) = raylib::init()
        .size(
            1280,
            (libdemug::SCREEN_HEIGHT as u64 * SCALE_FACTOR as u64) as i32,
        )
        .title("DeMuG")
        .build();
    let mut game_render = Image::gen_image_color(
        libdemug::SCREEN_WIDTH as i32,
        libdemug::SCREEN_HEIGHT as i32,
        Color::BLACK,
    );

    //rl.set_target_fps(60);
    
    game_render.set_format(PixelFormat::PIXELFORMAT_UNCOMPRESSED_R8G8B8);
    //demug.borrow_mut().disable_boot_rom(false);
    /*demug
        .borrow_mut()
        .insert_cartridge(PathBuf::from("./demug-gui/resources/01-special.gb"));*/

    let mut is_paused = true;
    let mut goto_next = false;
    
    if let Ok(mut texture) = rl.load_texture_from_image(&thread, &game_render) {
        
        let mut cpu_debug = demug.borrow().gather_debug_info();
        while !rl.window_should_close() {
            let mut d = rl.begin_drawing(&thread);
            if let Some(key) = d.get_key_pressed() {
                match key {
                    KeyboardKey::KEY_SPACE => {is_paused ^= true}
                    KeyboardKey::KEY_N => {goto_next = true}
                    _ => {}
                }
            }

            {
                d.clear_background(Color::BLACK);

                // Draw next cpu instruction
                if cpu_debug.next_instr_opcode == 0xCB {
                    d.draw_text(format!("NEXT INSTR: {:#X}->{:#x} | {}", cpu_debug.next_instr_opcode, cpu_debug.next_instr_pfx_opcode, cpu_debug.next_instr).as_str(), (libdemug::SCREEN_WIDTH as u64 * SCALE_FACTOR as u64 + 10u64) as i32, 10i32, 20i32, Color::YELLOW);
                } else {
                    d.draw_text(format!("NEXT INSTR: {:#X} | {}", cpu_debug.next_instr_opcode, cpu_debug.next_instr).as_str(), (libdemug::SCREEN_WIDTH as u64 * SCALE_FACTOR as u64 + 10u64) as i32, 10i32, 20i32, Color::YELLOW);
                }

                // Draw flag status
                d.draw_text("Z", (libdemug::SCREEN_WIDTH as u64 * SCALE_FACTOR as u64 + 10u64) as i32, 30i32, 20i32, if cpu_debug.registers.f >> 7 == 0b1 { Color::GREEN } else { Color::RED });
                d.draw_text("N", (libdemug::SCREEN_WIDTH as u64 * SCALE_FACTOR as u64 + 30u64) as i32, 30i32, 20i32, if cpu_debug.registers.f >> 6 & 0x1 == 0b1 { Color::GREEN } else { Color::RED });
                d.draw_text("H", (libdemug::SCREEN_WIDTH as u64 * SCALE_FACTOR as u64 + 50u64) as i32, 30i32, 20i32, if cpu_debug.registers.f >> 5 & 0x1 == 0b1 { Color::GREEN } else { Color::RED });
                d.draw_text("C", (libdemug::SCREEN_WIDTH as u64 * SCALE_FACTOR as u64 + 70u64) as i32, 30i32, 20i32, if cpu_debug.registers.f >> 4 & 0x1 == 0b1 { Color::GREEN } else { Color::RED });

                // Draw cpu registers
                d.draw_text(format!("A: {:#X}", cpu_debug.registers.a).as_str(), (libdemug::SCREEN_WIDTH as u64 * SCALE_FACTOR as u64 + 10u64) as i32, 50i32, 20i32, Color::YELLOW);
                d.draw_text(format!("B: {:#X}", cpu_debug.registers.b).as_str(), (libdemug::SCREEN_WIDTH as u64 * SCALE_FACTOR as u64 + 10u64) as i32, 70i32, 20i32, Color::YELLOW);
                d.draw_text(format!("C: {:#X}", cpu_debug.registers.c).as_str(), (libdemug::SCREEN_WIDTH as u64 * SCALE_FACTOR as u64 + 10u64) as i32, 90i32, 20i32, Color::YELLOW);
                d.draw_text(format!("D: {:#X}", cpu_debug.registers.d).as_str(), (libdemug::SCREEN_WIDTH as u64 * SCALE_FACTOR as u64 + 10u64) as i32, 110i32, 20i32, Color::YELLOW);
                d.draw_text(format!("E: {:#X}", cpu_debug.registers.e).as_str(), (libdemug::SCREEN_WIDTH as u64 * SCALE_FACTOR as u64 + 10u64) as i32, 130i32, 20i32, Color::YELLOW);
                d.draw_text(format!("H: {:#X}", cpu_debug.registers.h).as_str(), (libdemug::SCREEN_WIDTH as u64 * SCALE_FACTOR as u64 + 10u64) as i32, 150i32, 20i32, Color::YELLOW);
                d.draw_text(format!("L: {:#X}", cpu_debug.registers.l).as_str(), (libdemug::SCREEN_WIDTH as u64 * SCALE_FACTOR as u64 + 10u64) as i32, 170i32, 20i32, Color::YELLOW);
                d.draw_text(format!("PC: {:#X}", cpu_debug.registers.pc).as_str(), (libdemug::SCREEN_WIDTH as u64 * SCALE_FACTOR as u64 + 10u64) as i32, 190i32, 20i32, Color::YELLOW);
                d.draw_text(format!("SP: {:#X}", cpu_debug.registers.sp).as_str(), (libdemug::SCREEN_WIDTH as u64 * SCALE_FACTOR as u64 + 10u64) as i32, 210i32, 20i32, Color::YELLOW);

                // Drawing game
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
            }
            
            if !is_paused || goto_next {
                goto_next = false;
                cpu_debug = demug.borrow().gather_debug_info();
                let frame_ready = demug.borrow().step();

                if frame_ready {
                    if let Err(err) = texture.update_texture(demug.borrow().get_frame().as_slice()) {
                        println!("Error while updating texture: {err}");
                    }
                }
            }
        }
    }
}