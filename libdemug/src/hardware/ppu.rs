use crate::{Demug, SCREEN_HEIGHT, SCREEN_WIDTH};
use crate::utils::Register;
use std::cell::RefCell;
use std::sync::Arc;
use bincode::{config, Encode};

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
    ColorId0Lo = 0
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
    b: u8
}

impl Pixel {
    pub fn new() -> Self {
        Self {
            r: 0x00,
            g: 0x00,
            b: 0x00
        }
    }
}

pub struct Ppu {
    bus: Arc<RefCell<Demug>>,
    registers: PpuRegisters,
    screen_pixel_array: [Pixel; SCREEN_WIDTH as usize * SCREEN_HEIGHT as usize],
    dots: u32,
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
            frame_ready: false,
        }
    }

    pub fn read(&self, addr: u16) -> u8 {
        match addr {
            0xFF40 => {self.registers.lcdc.value()}
            0xFF41 => {self.registers.stat.value()}
            0xFF42 => {self.registers.scy}
            0xFF43 => {self.registers.scx}
            0xFF44 => {self.registers.ly}
            0xFF45 => {self.registers.lyc}
            0xFF46 => {self.registers.dma}
            0xFF47 => {self.registers.bgp.value()}
            0xFF48 => {self.registers.obp0.value()}
            0xFF49 => {self.registers.obp1.value()}
            0xFF4A => {self.registers.wy}
            0xFF4B => {self.registers.wx}
            _ => {unreachable!()}
        }
    }

    pub fn write(&mut self, addr: u16, value: u8) {
        match addr {
            0xFF40 => {self.registers.lcdc.set_value(value)}
            0xFF41 => {self.registers.stat.set_value(self.registers.stat.value() | value & 0xFC)}
            0xFF42 => {self.registers.scy = value}
            0xFF43 => {self.registers.scx = value}
            0xFF45 => {self.registers.lyc = value}
            0xFF46 => {self.registers.dma = value}
            0xFF47 => {self.registers.bgp.set_value(value)}
            0xFF48 => {self.registers.obp0.set_value(value)}
            0xFF49 => {self.registers.obp1.set_value(value)}
            0xFF4A => {self.registers.wy = value}
            0xFF4B => {self.registers.wx = value}
            _ => {unreachable!()}
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
        // TODO: Change the value of PpuModeHi | PpuModeLo according to current ppu mode
        if self.registers.lcdc.bit(LcdcReg::LcdPpuEnable as u8) == 0b1 {
            for _ in 0..m_cycles * 4 {
                let y_pos = self.dots / 456u32;
                let dot_x = self.dots - y_pos * 456u32;
                
                if dot_x < 80 {
                    // TODO: Manage OAM Scan (Mode 2)
                } else {
                    // TODO: Manage Mode 3 Length "penalties"
                    let x_pos: u16 = dot_x as u16 - 80u16;
                    
                    self.registers.ly = y_pos as u8;
                    self.registers.stat.set_conditional(StatReg::LycEqualLy as u8, self.registers.ly == self.registers.lyc);
                    
                    if self.registers.lcdc.bit(LcdcReg::BgWindowEnable as u8) == 0b1 && x_pos < SCREEN_WIDTH as u16 && y_pos < SCREEN_HEIGHT as u32 {
                        let mut tile_data_loc: u16 = if self.registers.lcdc.bit(LcdcReg::BgWindowTileMapDataArea as u8) == 0b0 { 0x9000 } else { 0x8000 };
                        let mut tile_map_loc: u16 = if self.registers.lcdc.bit(LcdcReg::BgTileMapArea as u8) == 0b0 { 0x9800 } else { 0x9C00 };
                        
                        tile_map_loc += x_pos / 8u16; // Address to Tile Data ID in Background/Window Tile Map
                        let tile_data_id = self.bus.borrow().read(tile_map_loc); // Tile Data ID from Background/Window map
                        tile_data_loc += tile_data_id as u16 * 8 * 2 + (y_pos as u16 % 8) * 2;
                        
                        let pixels_hi = self.bus.borrow().read(tile_data_loc) >> x_pos % 8 & 0x1;
                        let pixels_lo = self.bus.borrow().read(tile_data_loc + 1) >> x_pos % 8 & 0x1;
                        let color_index = pixels_hi << 1 | pixels_lo;
                        
                        self.screen_pixel_array[x_pos as usize + y_pos as usize * SCREEN_WIDTH as usize] = self.get_pixel_from_index(color_index);
                    }
                }
                if self.dots == 70224 {
                    self.frame_ready = true;
                }
                self.dots = if self.dots == 70224 { 0 } else { self.dots + 1 };
            }
        } else {
            self.registers.stat.sets(StatReg::PpuModeLo as u8 | StatReg::PpuModeHi as u8, 0b00);
        }
    }
    
    pub fn is_frame_ready(&self) -> bool {
        self.frame_ready
    }
    
    fn get_pixel_from_index(&self, index: u8) -> Pixel {
        let color: u8;
        
        match index {
            0b00 => {
                color = self.registers.bgp.bit(PaletteRegisters::ColorId0Hi as u8) << 1 | self.registers.bgp.bit(PaletteRegisters::ColorId0Lo as u8);
            }
            0b01 => {
                color = self.registers.bgp.bit(PaletteRegisters::ColorId1Hi as u8) << 1 | self.registers.bgp.bit(PaletteRegisters::ColorId1Lo as u8);
            }
            0b10 => {
                color = self.registers.bgp.bit(PaletteRegisters::ColorId2Hi as u8) << 1 | self.registers.bgp.bit(PaletteRegisters::ColorId2Lo as u8);
            }
            0b11 => {
                color = self.registers.bgp.bit(PaletteRegisters::ColorId3Hi as u8) << 1 | self.registers.bgp.bit(PaletteRegisters::ColorId3Lo as u8);
            }
            _ => {unreachable!()} 
        }
        
        match color {
            0b00 => {
                Pixel{r: 0x9A, g: 0x9E, b: 0x3F}
            }
            0b01 => {
                Pixel{r: 0x49, g: 0x6B, b: 0x22}
            }
            0b10 => {
                Pixel{r: 0x0E, g: 0x45, b: 0x0B}
            }
            0b11 => {
                Pixel{r: 0x1B, g: 0x2A, b: 0x09}
            }
            _ => {unreachable!()} 
        }
    }
}
