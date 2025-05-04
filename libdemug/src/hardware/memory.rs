use std::fs;
use std::path::PathBuf;

pub(crate) struct Memory {
    pub(crate) boot_rom: [u8; 0x0100], // Boot Rom Data: Mapped at 0x0000 - 0x0100
    pub(crate) game_rom: Vec<u8>,      // Game Rom Data: Mapped at 0x0000 - 0x3FFF
    pub(crate) vram: [u8; 0x2000],     // Video RAM    : Mapped at 0x8000 - 0x9FFF
    pub(crate) wram: [u8; 0x2000],     // Working RAM  : Mapped at 0xC000 - 0xDFFF
    pub(crate) hram: [u8; 0x007F],     // High RAM     : Mapped at 0xFF80 - 0xFFFE
}

impl Memory {
    pub(crate) fn init() -> Self {
        Self {
            boot_rom: *include_bytes!("../../resources/dmg_boot.bin"),
            game_rom: Vec::with_capacity(0x4000),
            vram: [0x00; 0x2000],
            wram: [0x00; 0x2000],
            hram: [0x00; 0x007F],
        }
    }

    pub(crate) fn open_game(&mut self, path: PathBuf) {
        match fs::read(path.clone()) {
            Ok(data) => {self.game_rom = data;}
            Err(err) => {println!("Cannot open ROM {}: {err}", path.display())}
        }
    }
}