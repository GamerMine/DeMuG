pub(crate) struct Memory {
    pub(crate) boot_rom: [u8; 0x0100], // Boot Rom Data: Mapped at 0x0000 - 0x0100
    pub(crate) vram: [u8; 0x2000],     // Video RAM    : Mapped at 0x8000 - 0x9FFF
    pub(crate) wram: [u8; 0x2000],     // Working RAM  : Mapped at 0xC000 - 0xDFFF
    pub(crate) hram: [u8; 0x007E],     // High RAM     : Mapped at 0xFF80 - 0xFFFE
}

impl Memory {
    pub(crate) fn init() -> Self {
        Self {
            boot_rom: *include_bytes!("../../resources/dmg_boot.bin"),
            vram: [0x00; 0x2000],
            wram: [0x00; 0x2000],
            hram: [0x00; 0x007E],
        }
    }
}