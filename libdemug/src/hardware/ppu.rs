use crate::hardware::cpu::Interrupts;
use crate::utils::Register;
use crate::{Demug, SCREEN_HEIGHT, SCREEN_WIDTH};
use bincode::{Encode, config};
use std::cell::RefCell;
use std::sync::Arc;

#[repr(u8)]
enum LcdcReg {
    LcdPpuEnable = 7,
    WindowTileMapArea = 6,
    WindowEnable = 5,
    BgWindowTileMapDataArea = 4,
    BgTileMapArea = 3,
    ObjSize = 2,
    ObjEnable = 1,
    BgWindowEnable = 0,
}

#[repr(u8)]
enum StatReg {
    LycSelect = 6,
    Mode2Select = 5,
    Mode1Select = 4,
    Mode0Select = 3,
    LycEqualLy = 2,
    PpuModeHi = 1,
    PpuModeLo = 0,
}

#[repr(u8)]
enum PaletteRegisters {
    ColorId3Hi = 7,
    ColorId3Lo = 6,
    ColorId2Hi = 5,
    ColorId2Lo = 4,
    ColorId1Hi = 3,
    ColorId1Lo = 2,
    ColorId0Hi = 1,
    ColorId0Lo = 0,
}

enum Palettes {
    BGP,
    OBP0,
    OBP1,
}

struct PpuRegisters {
    lcdc: Register, // LCD Control register             at 0xFF40
    stat: Register, // LCD status                       at 0xFF41
    scy: u8,        // Background Viewport Y Position   at 0xFF42
    scx: u8,        // Background Viewport X Position   at 0xFF43
    ly: u8,         // LCD Y coordinate                 at 0xFF44
    lyc: u8,        // LY compare                       at 0xFF45
    dma: u8,        // OAM DMA Source Address & Start   at 0xFF46
    bgp: Register,  // BG Palette Data                  at 0xFF47
    obp0: Register, // OBJ Palette 0 Data               at 0xFF48
    obp1: Register, // OBJ Palette 1 Data               at 0xFF49
    wy: u8,         // Window Y Position                at 0xFF4A
    wx: u8,         // Window X Position                at 0xFF4B
}

#[derive(Encode, Clone, Copy)]
pub struct Pixel {
    r: u8,
    g: u8,
    b: u8,
}

impl Pixel {
    pub fn new() -> Self {
        Self {
            r: 0x00,
            g: 0x00,
            b: 0x00,
        }
    }
}

#[repr(u8)]
enum ObjectAttributes {
    Priority = 7,
    YFlip = 6,
    XFlip = 5,
    DMGPalette = 4,
}

#[derive(Copy, Clone)]
struct Object {
    y_position: u8,
    x_position: u8,
    tile_index: u8,
    attributes: Register,
}

pub struct Ppu {
    bus: Arc<RefCell<Demug>>,
    registers: PpuRegisters,
    screen_pixel_array: [Pixel; SCREEN_WIDTH as usize * SCREEN_HEIGHT as usize],
    dots: u32,
    objects_in_line: ([Option<Object>; 10], u8),
    frame_ready: bool,
}

impl Ppu {
    pub fn init(bus: Arc<RefCell<Demug>>) -> Self {
        Self {
            bus,
            registers: PpuRegisters {
                lcdc: Register::new(0x91),
                stat: Register::new(0x81),
                scy: 0x00,
                scx: 0x00,
                ly: 0x91,
                lyc: 0x00,
                dma: 0xFF,
                bgp: Register::new(0xFC),
                obp0: Register::new(0x00),
                obp1: Register::new(0x00),
                wy: 0x00,
                wx: 0x00,
            },
            screen_pixel_array: [Pixel::new(); SCREEN_WIDTH as usize * SCREEN_HEIGHT as usize],
            dots: 0,
            objects_in_line: ([None; 10], 0),
            frame_ready: false,
        }
    }

    pub fn read(&self, addr: u16) -> u8 {
        match addr {
            0xFF40 => self.registers.lcdc.value(),
            0xFF41 => self.registers.stat.value(),
            0xFF42 => self.registers.scy,
            0xFF43 => self.registers.scx,
            0xFF44 => self.registers.ly,
            0xFF45 => self.registers.lyc,
            0xFF46 => self.registers.dma,
            0xFF47 => self.registers.bgp.value(),
            0xFF48 => self.registers.obp0.value(),
            0xFF49 => self.registers.obp1.value(),
            0xFF4A => self.registers.wy,
            0xFF4B => self.registers.wx,
            _ => {
                unreachable!()
            }
        }
    }

    pub fn write(&mut self, addr: u16, value: u8) {
        match addr {
            0xFF40 => self.registers.lcdc.set_value(value),
            0xFF41 => self.registers.stat.set_value(self.registers.stat.value() | value & 0xFC),
            0xFF42 => self.registers.scy = value,
            0xFF43 => self.registers.scx = value,
            0xFF44 => {}
            0xFF45 => self.registers.lyc = value,
            0xFF46 => {
                self.registers.dma = value;
                self.start_dma();
            }
            0xFF47 => self.registers.bgp.set_value(value),
            0xFF48 => self.registers.obp0.set_value(value),
            0xFF49 => self.registers.obp1.set_value(value),
            0xFF4A => self.registers.wy = value,
            0xFF4B => self.registers.wx = value,
            _ => {
                unreachable!()
            }
        }
    }

    pub fn get_frame(&mut self) -> Vec<u8> {
        let config = config::standard();

        self.frame_ready = false;

        // TODO: Don't rely on bincode
        if let Ok(vec) = bincode::encode_to_vec(self.screen_pixel_array, config) {
            vec
        } else {
            unreachable!()
        }
    }

    pub fn tick(&mut self, m_cycles: u64) {
        if self.registers.lcdc.bit(LcdcReg::LcdPpuEnable as u8) == 0b1 {
            for _ in 0..m_cycles * 4 {
                let y_pos = (self.dots / 456u32) as u8;
                let dot_x = (self.dots - y_pos as u32 * 456u32) as u8;

                if dot_x < 80 {
                    self.registers.stat.clear(StatReg::PpuModeLo as u8);
                    self.registers.stat.set(StatReg::PpuModeHi as u8);

                    // TODO: Handle 8x16 pixels objects
                    if dot_x % 2 == 0 {
                        let offset = (dot_x as u16 / 2) * 4;
                        let y_position = self.bus.borrow().read(0xFE00 | offset);

                        let big_obj = self.registers.lcdc.bit(LcdcReg::ObjSize as u8) == 0b1;
                        if self.objects_in_line.1 < 10
                            && y_position > if big_obj { 0 } else { 8 }
                            && y_position <= 160u8
                            && y_pos + 8 * 2 < y_position + if big_obj { 16 } else { 8 }
                            && y_pos + 8 * 2 >= y_position
                        {
                            let x_position = self.bus.borrow().read(0xFE00 | offset + 1);
                            let tile_index = self.bus.borrow().read(0xFE00 | offset + 2);
                            let attributes = self.bus.borrow().read(0xFE00 | offset + 3);

                            self.objects_in_line.0[self.objects_in_line.1 as usize] =
                                Some(Object {
                                    y_position,
                                    x_position,
                                    tile_index,
                                    attributes: Register::new(attributes),
                                });

                            self.objects_in_line.1 += 1;
                        }
                    }
                } else {
                    // TODO: Manage Mode 3 Length "penalties"
                    let x_pos = dot_x - 80u8;

                    self.registers.ly = y_pos as u8;
                    self.registers.stat.set_conditional(
                        StatReg::LycEqualLy as u8,
                        self.registers.ly == self.registers.lyc,
                    );
                    self.registers.stat.set(StatReg::PpuModeLo as u8);
                    self.registers.stat.set(StatReg::PpuModeHi as u8);

                    if x_pos < SCREEN_WIDTH && y_pos < SCREEN_HEIGHT {
                        if self.registers.lcdc.bit(LcdcReg::BgWindowEnable as u8) == 0b1 {
                            let mut tile_data_loc: u16 = {
                                if self.registers.lcdc.bit(LcdcReg::BgWindowTileMapDataArea as u8)
                                    == 0b0
                                {
                                    0x9000
                                } else {
                                    0x8000
                                }
                            };
                            let mut tile_map_loc: u16 = {
                                if self.registers.lcdc.bit(LcdcReg::WindowEnable as u8) == 0b1
                                    && x_pos + 7 >= self.registers.wx
                                    && y_pos > self.registers.wy
                                {
                                    if self.registers.lcdc.bit(LcdcReg::WindowTileMapArea as u8)
                                        == 0b0
                                    {
                                        0x9800
                                    } else {
                                        0x9C00
                                    }
                                } else {
                                    if self.registers.lcdc.bit(LcdcReg::BgTileMapArea as u8) == 0b0
                                    {
                                        0x9800
                                    } else {
                                        0x9C00
                                    }
                                }
                            };
                            let window_on_screen =
                                self.registers.lcdc.bit(LcdcReg::WindowEnable as u8) == 0b1
                                    && self.registers.wx >= 7
                                    && self.registers.wx < 166
                                    && x_pos + 7 >= self.registers.wx
                                    && self.registers.wy <= 143
                                    && y_pos >= self.registers.wy;

                            if window_on_screen {
                                tile_map_loc += (x_pos as u16 - (self.registers.wx as u16 - 7))
                                    / 8u16
                                    + ((y_pos as u16 - self.registers.wy as u16) / 8u16) * 32u16; // Address to Tile Data ID in Window Tile Map
                            } else {
                                tile_map_loc += (x_pos as u16 + self.registers.scx as u16) / 8u16
                                    % 32u16
                                    + ((y_pos as u16 + self.registers.scy as u16) / 8u16 % 32u16)
                                        * 32u16; // Address to Tile Data ID in Background Tile Map
                            }

                            let tile_data_id = self.bus.borrow().read(tile_map_loc); // Tile Data ID from Background/Window map
                            if tile_data_id > 127 {
                                tile_data_loc = 0x8000;
                            }

                            if window_on_screen {
                                tile_data_loc += tile_data_id as u16 * 8 * 2
                                    + ((y_pos as u16 - self.registers.wy as u16) % 8) * 2;
                            } else {
                                tile_data_loc += tile_data_id as u16 * 8 * 2
                                    + ((y_pos as u16 + self.registers.scy as u16 % 256) % 8) * 2;
                            }

                            let pixels_hi =
                                self.bus.borrow().read(tile_data_loc) >> 7 - x_pos % 8 & 0x1;
                            let pixels_lo =
                                self.bus.borrow().read(tile_data_loc + 1) >> 7 - x_pos % 8 & 0x1;
                            let color_index = pixels_hi << 1 | pixels_lo;

                            self.screen_pixel_array
                                [x_pos as usize + y_pos as usize * SCREEN_WIDTH as usize] =
                                self.get_pixel_from_index(color_index, Palettes::BGP);
                        }

                        if self.registers.lcdc.bit(LcdcReg::ObjEnable as u8) == 0b1 {
                            let mut last_found_xpos: u8 = 0xFF;
                            let mut found_obj: Option<Object> = None;
                            for o in self.objects_in_line.0 {
                                if let Some(obj) = o {
                                    if x_pos < obj.x_position
                                        && x_pos >= obj.x_position - 8
                                        && obj.x_position < last_found_xpos
                                    {
                                        last_found_xpos = obj.x_position;
                                        found_obj = Some(obj);
                                    }
                                }
                            }

                            if let Some(obj) = found_obj {
                                if obj.attributes.bit(ObjectAttributes::Priority as u8) == 0b0 {
                                    // TODO: Handle X flip & Y flip
                                    // TODO: Handle Transparent pixels
                                    let offset_obj: u16 =
                                        if self.registers.lcdc.bit(LcdcReg::ObjSize as u8) == 0b1 {
                                            if (obj.attributes.bit(ObjectAttributes::YFlip as u8) == 0b0 && y_pos + 8 >= obj.y_position - 8) 
                                                || (obj.attributes.bit(ObjectAttributes::YFlip as u8) == 0b1 && y_pos + 8 < obj.y_position - 8) {
                                                (obj.tile_index as u16 + 1) | 0x01
                                            } else {
                                                obj.tile_index as u16 & 0xFE
                                            }
                                        } else {
                                            obj.tile_index as u16
                                        };
                                    let tile_data_loc =
                                        0x8000 + offset_obj * 8 * 2 + y_pos as u16 % 8 * 2;

                                    let pixels_hi = self.bus.borrow().read(tile_data_loc)
                                        >> 7 - x_pos % 8
                                        & 0x1;
                                    let pixels_lo = self.bus.borrow().read(tile_data_loc + 1)
                                        >> 7 - x_pos % 8
                                        & 0x1;
                                    let color_index = pixels_hi << 1 | pixels_lo;

                                    self.screen_pixel_array
                                        [x_pos as usize + y_pos as usize * SCREEN_WIDTH as usize] =
                                        self.get_pixel_from_index(
                                            color_index,
                                            if obj
                                                .attributes
                                                .bit(ObjectAttributes::DMGPalette as u8)
                                                == 0b0
                                            {
                                                Palettes::OBP0
                                            } else {
                                                Palettes::OBP1
                                            },
                                        );
                                }
                            }
                        }
                    }

                    if x_pos == SCREEN_WIDTH {
                        self.objects_in_line = ([None; 10], 0);
                        self.registers.stat.clear(StatReg::PpuModeLo as u8);
                        self.registers.stat.clear(StatReg::PpuModeHi as u8);
                    }

                    if y_pos == SCREEN_HEIGHT {
                        self.bus.borrow().trigger_interrupt(Interrupts::Vblank);
                        self.registers.stat.set(StatReg::PpuModeLo as u8);
                        self.registers.stat.clear(StatReg::PpuModeHi as u8);
                    }
                }
                if self.dots == 70224 {
                    self.frame_ready = true;
                }
                self.dots = if self.dots == 70224 { 0 } else { self.dots + 1 };

                if self.registers.stat.bit(StatReg::LycSelect as u8) == 0b1
                    && self.registers.stat.bit(StatReg::LycEqualLy as u8) == 0b1
                {
                    self.bus.borrow().trigger_interrupt(Interrupts::Lcd);
                } else if self.registers.stat.bit(StatReg::Mode2Select as u8) == 0b1
                    && self.registers.stat.bit(StatReg::PpuModeHi as u8) << 1
                        | self.registers.stat.bit(StatReg::PpuModeLo as u8)
                        == 0b10
                {
                    self.bus.borrow().trigger_interrupt(Interrupts::Lcd);
                } else if self.registers.stat.bit(StatReg::Mode1Select as u8) == 0b1
                    && self.registers.stat.bit(StatReg::PpuModeHi as u8) << 1
                        | self.registers.stat.bit(StatReg::PpuModeLo as u8)
                        == 0b01
                {
                    self.bus.borrow().trigger_interrupt(Interrupts::Lcd);
                } else if self.registers.stat.bit(StatReg::Mode0Select as u8) == 0b1
                    && self.registers.stat.bit(StatReg::PpuModeHi as u8) << 1
                        | self.registers.stat.bit(StatReg::PpuModeLo as u8)
                        == 0b00
                {
                    self.bus.borrow().trigger_interrupt(Interrupts::Lcd);
                }
            }
        } else {
            // When disabled, the ppu is in Mode 0
            self.registers.stat.clear(StatReg::PpuModeLo as u8);
            self.registers.stat.clear(StatReg::PpuModeHi as u8);
        }
    }

    pub fn is_frame_ready(&self) -> bool {
        self.frame_ready
    }

    fn get_pixel_from_index(&self, index: u8, palette: Palettes) -> Pixel {
        let color: u8;

        match palette {
            Palettes::BGP => match index {
                0b00 => {
                    color = self.registers.bgp.bit(PaletteRegisters::ColorId0Hi as u8) << 1
                        | self.registers.bgp.bit(PaletteRegisters::ColorId0Lo as u8);
                }
                0b01 => {
                    color = self.registers.bgp.bit(PaletteRegisters::ColorId1Hi as u8) << 1
                        | self.registers.bgp.bit(PaletteRegisters::ColorId1Lo as u8);
                }
                0b10 => {
                    color = self.registers.bgp.bit(PaletteRegisters::ColorId2Hi as u8) << 1
                        | self.registers.bgp.bit(PaletteRegisters::ColorId2Lo as u8);
                }
                0b11 => {
                    color = self.registers.bgp.bit(PaletteRegisters::ColorId3Hi as u8) << 1
                        | self.registers.bgp.bit(PaletteRegisters::ColorId3Lo as u8);
                }
                _ => {
                    unreachable!()
                }
            },
            Palettes::OBP0 => match index {
                0b00 => {
                    color = self.registers.obp0.bit(PaletteRegisters::ColorId0Hi as u8) << 1
                        | self.registers.obp0.bit(PaletteRegisters::ColorId0Lo as u8);
                }
                0b01 => {
                    color = self.registers.obp0.bit(PaletteRegisters::ColorId1Hi as u8) << 1
                        | self.registers.obp0.bit(PaletteRegisters::ColorId1Lo as u8);
                }
                0b10 => {
                    color = self.registers.obp0.bit(PaletteRegisters::ColorId2Hi as u8) << 1
                        | self.registers.obp0.bit(PaletteRegisters::ColorId2Lo as u8);
                }
                0b11 => {
                    color = self.registers.obp0.bit(PaletteRegisters::ColorId3Hi as u8) << 1
                        | self.registers.obp0.bit(PaletteRegisters::ColorId3Lo as u8);
                }
                _ => {
                    unreachable!()
                }
            },
            Palettes::OBP1 => match index {
                0b00 => {
                    color = self.registers.obp1.bit(PaletteRegisters::ColorId0Hi as u8) << 1
                        | self.registers.obp1.bit(PaletteRegisters::ColorId0Lo as u8);
                }
                0b01 => {
                    color = self.registers.obp1.bit(PaletteRegisters::ColorId1Hi as u8) << 1
                        | self.registers.obp1.bit(PaletteRegisters::ColorId1Lo as u8);
                }
                0b10 => {
                    color = self.registers.obp1.bit(PaletteRegisters::ColorId2Hi as u8) << 1
                        | self.registers.obp1.bit(PaletteRegisters::ColorId2Lo as u8);
                }
                0b11 => {
                    color = self.registers.obp1.bit(PaletteRegisters::ColorId3Hi as u8) << 1
                        | self.registers.obp1.bit(PaletteRegisters::ColorId3Lo as u8);
                }
                _ => {
                    unreachable!()
                }
            },
        }

        match color {
            0b00 => Pixel {
                r: 0x9A,
                g: 0x9E,
                b: 0x3F,
            },
            0b01 => Pixel {
                r: 0x49,
                g: 0x6B,
                b: 0x22,
            },
            0b10 => Pixel {
                r: 0x0E,
                g: 0x45,
                b: 0x0B,
            },
            0b11 => Pixel {
                r: 0x1B,
                g: 0x2A,
                b: 0x09,
            },
            _ => {
                unreachable!()
            }
        }
    }

    fn start_dma(&self) {
        //FIXME: To make a cycle accurate emulator, a DMA OAM transfer should not be handled like that.
        //  While transferring, the cpu should continue to execute instructions and tick other devices.
        for obj_attr in 0..0x00A0 {
            let addr: u16 = (self.registers.dma as u16) << 8 | obj_attr;
            self.bus.borrow().write(0xFE00 | obj_attr, self.bus.borrow().read(addr));
        }
    }
}
