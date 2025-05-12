use libdemug::{AccessMode, BusDebugInfo};
use libdemug::hardware::cpu::{CpuDebugInfo, CpuRegisters};
use libdemug::hardware::memory::MemoryDebugInfo;

pub struct DebuggerControls {
    pub pause: bool,
    pub goto_next: bool,
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
                next_instr: "",
                next_instr_opcode: 0x00,
                next_instr_pfx_opcode: 0x00,
            },
            bus: BusDebugInfo {
                last_accessed_addr: 0x0000,
                last_accessed_addr_mode: AccessMode::READ
            },
            mem: MemoryDebugInfo {
                vram: [0x00; 8192]
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