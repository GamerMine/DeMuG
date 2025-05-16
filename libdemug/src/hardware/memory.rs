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

use std::fs;
use std::path::PathBuf;

pub(crate) struct Memory {
    pub(crate) boot_rom: [u8; 0x0100], // Boot Rom Data: Mapped at 0x0000 - 0x0100
    pub(crate) game_rom: Vec<u8>,      // Game Rom Data: Mapped at 0x0000 - 0x3FFF // FIXME: This is temporary (it should be placed under a cartridge management struct)
    pub(crate) vram: [u8; 0x2000],     // Video RAM    : Mapped at 0x8000 - 0x9FFF
    pub(crate) wram: [u8; 0x2000],     // Working RAM  : Mapped at 0xC000 - 0xDFFF
    pub(crate) hram: [u8; 0x007F],     // High RAM     : Mapped at 0xFF80 - 0xFFFE
    pub(crate) oam: [u8; 0xA0],        // Object Attribute Memory : Not mapped, directly accessed by cpu and ppu
}

impl Memory {
    pub(crate) fn init() -> Self {
        Self {
            boot_rom: *include_bytes!("../../resources/dmg_boot.bin"),
            game_rom: vec![0xFF; 0x8000],
            vram: [0x00; 0x2000],
            wram: [0x00; 0x2000],
            hram: [0x00; 0x007F],
            oam: [0x00; 0xA0],
        }
    }

    pub(crate) fn open_game(&mut self, path: PathBuf) {
        match fs::read(path.clone()) {
            Ok(data) => {self.game_rom = data;}
            Err(err) => {eprintln!("Cannot open ROM {}: {err}", path.display())}
        }
    }
    
    #[cfg(feature = "debug")]
    pub(crate) fn gather_debug_info(&self) -> MemoryDebugInfo {
        MemoryDebugInfo {
            boot_rom: self.boot_rom,
            game_rom: self.game_rom.clone(),
            vram: self.vram,
        }
    }
}

#[cfg(feature = "debug")]
pub struct MemoryDebugInfo {
    pub boot_rom: [u8; 0x0100], // Boot Rom Data: Mapped at 0x0000 - 0x0100
    pub game_rom: Vec<u8>,      // Game Rom Data: Mapped at 0x0000 - 0x3FFF
    pub vram: [u8; 0x2000],     // Video RAM    : Mapped at 0x8000 - 0x9FFF
}
