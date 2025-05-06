use crate::hardware::cpu::{Cpu, CpuDebugInfo, Interrupts};
use crate::hardware::memory::{Memory, MemoryDebugInfo};
use crate::hardware::ppu::Ppu;
use crate::utils::Register;
use std::cell::{Cell, RefCell};
use std::path::PathBuf;
use std::sync::Arc;

mod hardware;
mod utils;

pub const SCREEN_WIDTH: u8 = 160;
pub const SCREEN_HEIGHT: u8 = 144;

pub struct Demug {
    memory: RefCell<Option<Memory>>,
    cpu: RefCell<Option<Cpu>>,
    ppu: RefCell<Option<Ppu>>,
    disable_boot_rom: Cell<bool>,
    interrupt_flags: Cell<Register>,
    interrupt_enable: Cell<Register>,
    bus_debug_info: RefCell<Option<BusDebugInfo>>,
}

impl Demug {
    pub fn init() -> Arc<RefCell<Self>> {
        let demug = Arc::new(RefCell::new(Demug {
            memory: RefCell::new(None),
            cpu: RefCell::new(None),
            ppu: RefCell::new(None),
            disable_boot_rom: Cell::new(false),
            interrupt_flags: Cell::new(Register::new(0xE1)),
            interrupt_enable: Cell::new(Register::new(0x00)),
            bus_debug_info: RefCell::new(None),
        }));

        // Create all necessary 'devices'
        let memory = Memory::init();
        let cpu = Cpu::init(demug.clone());
        let ppu = Ppu::init(demug.clone());

        demug.borrow_mut().memory.replace(Some(memory));
        demug.borrow_mut().cpu.replace(Some(cpu));
        demug.borrow_mut().ppu.replace(Some(ppu));

        demug
    }

    pub fn insert_cartridge(&mut self, path: PathBuf) {
        if let Some(mem) = &mut *self.memory.borrow_mut() {
            mem.open_game(path)
        }
    }

    pub fn get_frame(&self) -> Vec<u8> {
        if let Some(ppu) = &mut *self.ppu.borrow_mut() { ppu.get_frame() } else { unreachable!() }
    }

    pub fn step(&self) -> bool {
        if let Some(cpu) = &mut *self.cpu.borrow_mut() {
            cpu.execute();
        }
        if let Some(ppu) = &*self.ppu.borrow() { ppu.is_frame_ready() } else { unreachable!() }
    }

    #[cfg(feature = "debug")]
    pub fn gather_debug_info(&self) -> (CpuDebugInfo, BusDebugInfo, MemoryDebugInfo) {
        let cpu_debug_info = 
            if let Some(cpu) = &*self.cpu.borrow() { cpu.gather_debug_info() } else { unreachable!() };
        let bus_debug_info = self.bus_debug_info.borrow_mut().take().unwrap();
        let memory_debug_info =
            if let Some(mem) = &*self.memory.borrow() { mem.gather_debug_info() } else { unreachable!() };
        
        (cpu_debug_info, bus_debug_info, memory_debug_info)
    }

    pub fn disable_boot_rom(&mut self, disabled: bool) {
        self.disable_boot_rom.set(disabled);
    }

    fn read(&self, addr: u16) -> u8 {
        let mut data: u8 = 0xFF;
        let addr = addr as usize;

        if addr <= 0x00FF && !self.disable_boot_rom.get() {
            if let Some(mem) = &*self.memory.borrow() {
                data = mem.boot_rom[addr];
            }
        } else if addr <= 0x00FF && self.disable_boot_rom.get() {
            if let Some(mem) = &*self.memory.borrow() {
                data = mem.game_rom[addr];
            }
        } else if addr >= 0x0100 && addr <= 0x7FFF {
            // FIXME: This should not be handled like that, the game rom should be accessed through a read method of a cartridge management struct
            if let Some(mem) = &*self.memory.borrow() {
                data = mem.game_rom[addr];
            }
        } else if addr >= 0x8000 && addr <= 0x9FFF {
            if let Some(mem) = &*self.memory.borrow() {
                data = mem.vram[addr - 0x8000];
            }
        } else if addr >= 0xC000 && addr <= 0xDFFF {
            if let Some(mem) = &*self.memory.borrow() {
                data = mem.wram[addr - 0xC000];
            }
        } else if addr >= 0xE000 && addr <= 0xFDFF {
            if let Some(mem) = &*self.memory.borrow() {
                data = mem.wram[addr - 0xE000];
            }
        } else if addr == 0xFF0F {
            data = self.interrupt_flags.get().value()
        } else if addr >= 0xFF40 && addr <= 0xFF4B {
            if let Some(ppu) = &*self.ppu.borrow() {
                data = ppu.read(addr as u16)
            }
        } else if addr >= 0xFF80 && addr <= 0xFFFE {
            if let Some(mem) = &*self.memory.borrow() {
                data = mem.hram[addr - 0xFF80];
            }
        } else if addr == 0xFFFF {
            data = self.interrupt_enable.get().value();
        } else {
            //println!("Read from {:#X} is not implemented!", addr);
            //exit(0)
        }
        
        if cfg!(feature = "debug") {
            let debug_info = BusDebugInfo {
                last_accessed_addr: addr as u16,
                last_accessed_addr_mode: AccessMode::READ,
            };
            
            self.bus_debug_info.borrow_mut().replace(debug_info);
        }

        data
    }

    fn write(&self, addr: u16, data: u8) {
        let addr = addr as usize;

        if addr >= 0x8000 && addr <= 0x9FFF {
            if let Some(mem) = &mut *self.memory.borrow_mut() {
                mem.vram[addr - 0x8000] = data;
            }
        } else if addr >= 0xC000 && addr <= 0xDFFF {
            if let Some(mem) = &mut *self.memory.borrow_mut() {
                mem.wram[addr - 0xC000] = data;
            }
        } else if addr == 0xFF0F {
            let mut new_register = self.interrupt_flags.get();
            new_register.set_value(data);
            self.interrupt_flags.set(new_register);
        } else if addr >= 0xFF40 && addr <= 0xFF4B {
            if let Some(ppu) = &mut *self.ppu.borrow_mut() {
                ppu.write(addr as u16, data);
            }
        } else if addr == 0xFF50 && data != 0x00 {
            self.disable_boot_rom.set(true);
            let mut new_register = self.interrupt_flags.get();
            new_register.set_value(data);
            self.interrupt_flags.set(new_register);
            println!("Disabling Boot Rom");
        } else if addr >= 0xFF80 && addr <= 0xFFFE {
            if let Some(mem) = &mut *self.memory.borrow_mut() {
                mem.hram[addr - 0xFF80] = data;
            }
        } else if addr == 0xFFFF {
            let mut new_register = self.interrupt_enable.get();
            new_register.set_value(data);
            self.interrupt_enable.set(new_register);
        } else {
            //println!("Write to {:#X} is not implemented!", addr);
        }

        if cfg!(feature = "debug") {
            let debug_info = BusDebugInfo {
                last_accessed_addr: addr as u16,
                last_accessed_addr_mode: AccessMode::WRITE,
            };

            self.bus_debug_info.borrow_mut().replace(debug_info);
        }
    }

    fn tick(&self, m_cycles: u64) {
        if let Some(ppu) = &mut *self.ppu.borrow_mut() {
            ppu.tick(m_cycles);
        }
    }

    fn trigger_interrupt(&self, interrupt: Interrupts) {
        let mut new_register = self.interrupt_flags.get();
        new_register.set(interrupt as u8);
        self.interrupt_flags.set(new_register);
    }
}

#[cfg(feature = "debug")]
pub enum AccessMode {
    READ,
    WRITE
}

#[cfg(feature = "debug")]
pub struct BusDebugInfo {
    last_accessed_addr: u16,
    last_accessed_addr_mode: AccessMode
}