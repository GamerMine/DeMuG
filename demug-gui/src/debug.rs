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