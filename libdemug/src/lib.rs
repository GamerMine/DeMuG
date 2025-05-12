#[cfg(feature = "debug")]
use crate::hardware::cpu::CpuDebugInfo;
#[cfg(feature = "debug")]
use crate::hardware::memory::MemoryDebugInfo;

use crate::hardware::cpu::{Cpu, Interrupts};
use crate::hardware::memory::Memory;
use crate::hardware::ppu::{Pixel, Ppu};
use crate::hardware::timer::Timer;
use crate::utils::Register;
use std::path::PathBuf;
use std::sync::atomic::{AtomicBool, Ordering};
use std::sync::{Arc, RwLock};

pub mod hardware;
mod utils;

pub const SCREEN_WIDTH: u8 = 160;
pub const SCREEN_HEIGHT: u8 = 144;

pub struct Demug {
    memory: RwLock<Option<Memory>>,
    cpu: RwLock<Option<Cpu>>,
    ppu: RwLock<Option<Ppu>>,
    timer: RwLock<Option<Timer>>,
    disable_boot_rom: AtomicBool,
    interrupt_flags: RwLock<Register>,
    interrupt_enable: RwLock<Register>,
    #[cfg(feature = "debug")]
    bus_debug_info: RwLock<Option<BusDebugInfo>>,
}

impl Demug {
    pub fn init() -> Arc<RwLock<Demug>> {
        let demug = Arc::new(RwLock::new(Demug {
            memory: RwLock::new(None),
            cpu: RwLock::new(None),
            ppu: RwLock::new(None),
            timer: RwLock::new(None),
            disable_boot_rom: AtomicBool::new(false),
            interrupt_flags: RwLock::new(Register::new(0xE1)),
            interrupt_enable: RwLock::new(Register::new(0x00)),
            #[cfg(feature = "debug")]
            bus_debug_info: RwLock::new(None),
        }));

        // Create all necessary 'devices'
        let memory = Memory::init();
        let cpu = Cpu::init(demug.clone());
        let ppu = Ppu::init(demug.clone());
        let timer = Timer::init(demug.clone());

        demug.write().unwrap().memory = RwLock::new(Some(memory));
        demug.write().unwrap().cpu = RwLock::new(Some(cpu));
        demug.write().unwrap().ppu = RwLock::new(Some(ppu));
        demug.write().unwrap().timer = RwLock::new(Some(timer));

        demug
    }

    pub fn insert_cartridge(&mut self, path: PathBuf) {
        if let Some(mem) = &mut *self.memory.write().unwrap() {
            mem.open_game(path)
        }
    }

    pub fn get_frame(&self) -> Vec<Pixel> {
        if let Some(ppu) = &*self.ppu.read().unwrap() { ppu.get_frame() } else { unreachable!() }
    }

    pub fn step(&self) -> bool {
        if let Some(cpu) = &mut *self.cpu.write().unwrap() { cpu.execute() } else { unreachable!() }
    }

    #[cfg(feature = "debug")]
    pub fn gather_debug_info(&self) -> (CpuDebugInfo, BusDebugInfo, MemoryDebugInfo) {
        let cpu_debug_info = if let Some(cpu) = &*self.cpu.read().unwrap() {
            cpu.gather_debug_info()
        } else {
            unreachable!()
        };
        let bus_debug_info = self.bus_debug_info.write().unwrap().take().unwrap();
        let memory_debug_info = if let Some(mem) = &*self.memory.read().unwrap() {
            mem.gather_debug_info()
        } else {
            unreachable!()
        };

        (cpu_debug_info, bus_debug_info, memory_debug_info)
    }

    pub fn disable_boot_rom(&mut self, disabled: bool) {
        self.disable_boot_rom.store(disabled, Ordering::SeqCst);
    }

    fn read(&self, addr: u16) -> u8 {
        let mut data: u8 = 0xFF;
        let addr = addr as usize;

        if addr <= 0x00FF && !self.disable_boot_rom.load(Ordering::SeqCst) {
            if let Some(mem) = &*self.memory.read().unwrap() {
                data = mem.boot_rom[addr];
            }
        } else if addr <= 0x00FF && self.disable_boot_rom.load(Ordering::SeqCst) {
            if let Some(mem) = &*self.memory.read().unwrap() {
                data = mem.game_rom[addr];
            }
        } else if (0x0100..=0x7FFF).contains(&addr) {
            // FIXME: This should not be handled like that, the game rom should be accessed through a read method of a cartridge management struct
            if let Some(mem) = &*self.memory.read().unwrap() {
                data = mem.game_rom[addr];
            }
        } else if (0x8000..=0x9FFF).contains(&addr) {
            if let Some(mem) = &*self.memory.read().unwrap() {
                data = mem.vram[addr - 0x8000];
            }
        } else if (0xC000..=0xDFFF).contains(&addr) {
            if let Some(mem) = &*self.memory.read().unwrap() {
                data = mem.wram[addr - 0xC000];
            }
        } else if (0xE000..=0xFDFF).contains(&addr) {
            if let Some(mem) = &*self.memory.read().unwrap() {
                data = mem.wram[addr - 0xE000];
            }
        } else if (0xFE00..=0xFE9F).contains(&addr) {
            if let Some(mem) = &*self.memory.read().unwrap() {
                data = mem.oam[addr - 0xFE00];
            }
        } else if (0xFF04..=0xFF07).contains(&addr) {
            if let Some(timer) = &*self.timer.read().unwrap() {
                data = timer.read(addr as u16);
            }
        } else if addr == 0xFF0F {
            data = self.interrupt_flags.read().unwrap().value()
        } else if (0xFF40..=0xFF4B).contains(&addr) {
            if let Some(ppu) = &*self.ppu.read().unwrap() {
                data = ppu.read(addr as u16)
            }
        } else if (0xFF80..=0xFFFE).contains(&addr) {
            if let Some(mem) = &*self.memory.read().unwrap() {
                data = mem.hram[addr - 0xFF80];
            }
        } else if addr == 0xFFFF {
            data = self.interrupt_enable.read().unwrap().value();
        }

        #[cfg(feature = "debug")]
        {
            let debug_info = BusDebugInfo {
                last_accessed_addr: addr as u16,
                last_accessed_addr_mode: AccessMode::READ,
            };

            self.bus_debug_info.write().unwrap().replace(debug_info);
        }
        data
    }

    fn write(&self, addr: u16, data: u8) {
        let addr = addr as usize;

        if (0x8000..=0x9FFF).contains(&addr) {
            if let Some(mem) = &mut *self.memory.write().unwrap() {
                mem.vram[addr - 0x8000] = data;
            }
        } else if (0xC000..=0xDFFF).contains(&addr) {
            if let Some(mem) = &mut *self.memory.write().unwrap() {
                mem.wram[addr - 0xC000] = data;
            }
        } else if (0xFE00..=0xFE9F).contains(&addr) {
            if let Some(mem) = &mut *self.memory.write().unwrap() {
                mem.oam[addr - 0xFE00] = data;
            }
        } else if (0xFF04..=0xFF07).contains(&addr) {
            if let Some(timer) = &mut *self.timer.write().unwrap() {
                timer.write(addr as u16, data);
            }
        } else if addr == 0xFF0F {
            self.interrupt_flags.write().unwrap().set_value(data);
        } else if (0xFF40..=0xFF4B).contains(&addr) {
            if let Some(ppu) = &mut *self.ppu.write().unwrap() {
                ppu.write(addr as u16, data);
            }
        } else if addr == 0xFF50 && data != 0x00 {
            self.disable_boot_rom.store(true, Ordering::SeqCst);
            self.interrupt_flags.write().unwrap().set_value(data);
            println!("Disabling Boot Rom");
        } else if (0xFF80..=0xFFFE).contains(&addr) {
            if let Some(mem) = &mut *self.memory.write().unwrap() {
                mem.hram[addr - 0xFF80] = data;
            }
        } else if addr == 0xFFFF {
            self.interrupt_enable.write().unwrap().set_value(data);
        }

        #[cfg(feature = "debug")]
        {
            let debug_info = BusDebugInfo {
                last_accessed_addr: addr as u16,
                last_accessed_addr_mode: AccessMode::WRITE,
            };

            self.bus_debug_info.write().unwrap().replace(debug_info);
        }
    }

    fn tick(&self, m_cycles: u64) {
        if let (Some(ppu), Some(timer)) =
            (&mut *self.ppu.write().unwrap(), &mut *self.timer.write().unwrap())
        {
            ppu.tick(m_cycles);
            timer.tick(m_cycles);
        }
    }

    fn trigger_interrupt(&self, interrupt: Interrupts) {
        self.interrupt_flags.write().unwrap().set_value(interrupt as u8);
    }
}

#[cfg(feature = "debug")]
pub enum AccessMode {
    READ,
    WRITE,
}

#[cfg(feature = "debug")]
pub struct BusDebugInfo {
    pub last_accessed_addr: u16,
    pub last_accessed_addr_mode: AccessMode,
}
