pub struct Memory {
    pub boot_rom: [u8; 0x0100],
    pub vram: [u8; 0x2000],
    pub wram: [u8; 0x2000],
    pub hram: [u8; 0x007E],
}

impl Memory {
    pub fn init() -> Self {
        Self {
            boot_rom: *include_bytes!("../../resources/dmg_boot.bin"),
            vram: [0x00; 0x2000],
            wram: [0x00; 0x2000],
            hram: [0x00; 0x007E],
        }
    }
}