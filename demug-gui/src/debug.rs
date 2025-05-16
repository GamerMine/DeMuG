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

pub mod analyzer;

use libdemug::{AccessMode, BusDebugInfo};
use libdemug::hardware::cpu::{CpuDebugInfo, CpuRegisters};
use libdemug::hardware::memory::MemoryDebugInfo;
use libdemug::utils::Register;

pub struct DebuggerControls {
    pub pause: bool,
    pub goto_next: bool,
    pub breakpoints: Vec<u16>,
}

pub struct DemugDebugData {
    pub cpu: CpuDebugInfo,
    pub bus: BusDebugInfo,
    pub mem: MemoryDebugInfo,
}

impl DemugDebugData {
    pub fn new_empty() -> Self {
        Self {
            cpu: CpuDebugInfo {
                registers: CpuRegisters {
                    a: 0x00,
                    f: 0x00,
                    b: 0x00,
                    c: 0x00,
                    d: 0x00,
                    e: 0x00,
                    h: 0x00,
                    l: 0x00,
                    pc: 0x0000,
                    sp: 0x0000,
                },
                ime: false,
            },
            bus: BusDebugInfo {
                last_accessed_addr: 0x0000,
                last_accessed_addr_mode: AccessMode::READ,
                boot_rom_disabled: false,
                interrupts_enable: Register::new(0x00),
                interrupts_flags: Register::new(0x00),
            },
            mem: MemoryDebugInfo {
                boot_rom: [0x00; 256],
                game_rom: vec![0x00; 0x4000],
                vram: [0x00; 8192],
            }
        }
    }

    pub fn from(dbg_data: (CpuDebugInfo, BusDebugInfo, MemoryDebugInfo)) -> Self {
        Self {
            cpu: dbg_data.0,
            bus: dbg_data.1,
            mem: dbg_data.2,
        }
    }
}