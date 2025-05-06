use std::path::PathBuf;
use raylib::color::Color;
use raylib::consts::{GuiControl, GuiDefaultProperty, KeyboardKey, PixelFormat};
use raylib::drawing::RaylibDraw;
use raylib::math::{Rectangle, Vector2};
use raylib::prelude::Image;
use raylib::rgui::RaylibDrawGui;
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

    rl.set_target_fps(60);
    
    game_render.set_format(PixelFormat::PIXELFORMAT_UNCOMPRESSED_R8G8B8);
    demug.borrow_mut().disable_boot_rom(false);
    demug
        .borrow_mut()
        .insert_cartridge(PathBuf::from("./demug-gui/resources/01-special.gb"));

    let mut should_exit = false;
    let mut is_paused = true;
    let mut goto_next = false;
    let mut buffer: String = String::from("0000\0");

    let mut show_memory = false;
    let mut current_memory_page: u8 = 0x01;
    
    if let Ok(mut texture) = rl.load_texture_from_image(&thread, &game_render) {
        
        //let mut cpu_debug = demug.borrow().gather_debug_info();
        while !should_exit {
            should_exit = rl.window_should_close();

            if let Some(key) = rl.get_key_pressed() {
                match key {
                    KeyboardKey::KEY_SPACE => {is_paused ^= true}
                    KeyboardKey::KEY_N => {goto_next = true}
                    KeyboardKey::KEY_K => {show_memory ^= true}
                    KeyboardKey::KEY_LEFT => {
                        if show_memory {
                            if current_memory_page > 1 {
                                current_memory_page -= 1;
                            } else {
                                current_memory_page = 16;
                            }
                        }
                    }
                    KeyboardKey::KEY_RIGHT => {
                        if show_memory {
                            if current_memory_page < 16 {
                                current_memory_page += 1;
                            } else {
                                current_memory_page = 1;
                            }
                        }
                    }
                    _ => {}
                }
            }

            /*let bp_buffer = &buffer.trim_end_matches('\0').to_lowercase();
            if let Ok(bp) = u16::from_str_radix(bp_buffer, 16) {
                if bp == cpu_debug.0.registers.pc {
                    is_paused = true;
                }
            }*/

            {
                /*let mut d = rl.begin_drawing(&thread);
                d.clear_background(Color::BLACK);*/

                if !show_memory {
                    /*d.gui_set_style(GuiControl::DEFAULT, GuiDefaultProperty::TEXT_SIZE, 20);
                    d.gui_text_box(Rectangle::new(950.0, 70.0, 80.0, 60.0), &mut buffer, true);

                    // Draw next cpu instruction
                    if cpu_debug.0.next_instr_opcode == 0xCB {
                        d.draw_text(format!("NEXT INSTR: {:#X}->{:#x} | {}", cpu_debug.0.next_instr_opcode, cpu_debug.0.next_instr_pfx_opcode, cpu_debug.0.next_instr).as_str(), (libdemug::SCREEN_WIDTH as u64 * SCALE_FACTOR as u64 + 10u64) as i32, 10i32, 20i32, Color::YELLOW);
                    } else {
                        d.draw_text(format!("NEXT INSTR: {:#X} | {}", cpu_debug.0.next_instr_opcode, cpu_debug.0.next_instr).as_str(), (libdemug::SCREEN_WIDTH as u64 * SCALE_FACTOR as u64 + 10u64) as i32, 10i32, 20i32, Color::YELLOW);
                    }

                    // Draw flag status
                    d.draw_text("Z", (libdemug::SCREEN_WIDTH as u64 * SCALE_FACTOR as u64 + 10u64) as i32, 30i32, 20i32, if cpu_debug.0.registers.f >> 7 == 0b1 { Color::GREEN } else { Color::RED });
                    d.draw_text("N", (libdemug::SCREEN_WIDTH as u64 * SCALE_FACTOR as u64 + 30u64) as i32, 30i32, 20i32, if cpu_debug.0.registers.f >> 6 & 0x1 == 0b1 { Color::GREEN } else { Color::RED });
                    d.draw_text("H", (libdemug::SCREEN_WIDTH as u64 * SCALE_FACTOR as u64 + 50u64) as i32, 30i32, 20i32, if cpu_debug.0.registers.f >> 5 & 0x1 == 0b1 { Color::GREEN } else { Color::RED });
                    d.draw_text("C", (libdemug::SCREEN_WIDTH as u64 * SCALE_FACTOR as u64 + 70u64) as i32, 30i32, 20i32, if cpu_debug.0.registers.f >> 4 & 0x1 == 0b1 { Color::GREEN } else { Color::RED });

                    // Draw cpu registers
                    d.draw_text(format!("A: {:#X}", cpu_debug.0.registers.a).as_str(), (libdemug::SCREEN_WIDTH as u64 * SCALE_FACTOR as u64 + 10u64) as i32, 50i32, 20i32, Color::YELLOW);
                    d.draw_text(format!("B: {:#X}", cpu_debug.0.registers.b).as_str(), (libdemug::SCREEN_WIDTH as u64 * SCALE_FACTOR as u64 + 10u64) as i32, 70i32, 20i32, Color::YELLOW);
                    d.draw_text(format!("C: {:#X}", cpu_debug.0.registers.c).as_str(), (libdemug::SCREEN_WIDTH as u64 * SCALE_FACTOR as u64 + 10u64) as i32, 90i32, 20i32, Color::YELLOW);
                    d.draw_text(format!("D: {:#X}", cpu_debug.0.registers.d).as_str(), (libdemug::SCREEN_WIDTH as u64 * SCALE_FACTOR as u64 + 10u64) as i32, 110i32, 20i32, Color::YELLOW);
                    d.draw_text(format!("E: {:#X}", cpu_debug.0.registers.e).as_str(), (libdemug::SCREEN_WIDTH as u64 * SCALE_FACTOR as u64 + 10u64) as i32, 130i32, 20i32, Color::YELLOW);
                    d.draw_text(format!("H: {:#X}", cpu_debug.0.registers.h).as_str(), (libdemug::SCREEN_WIDTH as u64 * SCALE_FACTOR as u64 + 10u64) as i32, 150i32, 20i32, Color::YELLOW);
                    d.draw_text(format!("L: {:#X}", cpu_debug.0.registers.l).as_str(), (libdemug::SCREEN_WIDTH as u64 * SCALE_FACTOR as u64 + 10u64) as i32, 170i32, 20i32, Color::YELLOW);
                    d.draw_text(format!("PC: {:#X}", cpu_debug.0.registers.pc).as_str(), (libdemug::SCREEN_WIDTH as u64 * SCALE_FACTOR as u64 + 10u64) as i32, 190i32, 20i32, Color::YELLOW);
                    d.draw_text(format!("SP: {:#X}", cpu_debug.0.registers.sp).as_str(), (libdemug::SCREEN_WIDTH as u64 * SCALE_FACTOR as u64 + 10u64) as i32, 210i32, 20i32, Color::YELLOW);
*/
                    // Drawing game
                    /*d.draw_texture_pro(
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
                    );*/
                } /*else {
                    for line in 32 * (current_memory_page - 1) as usize..current_memory_page as usize * 32 {
                        let mut line_str: String = String::from("");
                        for value in 0..=0xF {
                            line_str.push_str(format!("{:#04X} ", cpu_debug.2.vram[value * line]).as_str())
                        }
                        d.draw_text(format!("{:#04X} {line_str}", line * 0x10 + 0x8000).as_str(), 0, ((line - 32 * (current_memory_page as usize - 1)) * 20) as i32, 20, Color::YELLOW);
                    }
                }*/
            }
            
           /* if !is_paused || goto_next {
                goto_next = false;*/
                //cpu_debug = demug.borrow().gather_debug_info();
                let frame_ready = demug.borrow().step();

                if frame_ready {
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
                }
            //}
        }
    }
}