#[cfg(feature = "debug")]use crate::hardware::cpu::CpuDebugInfo;
#[cfg(feature = "debug")]use crate::hardware::memory::MemoryDebugInfo;

use crate::hardware::cpu::{Cpu, Interrupts};
use crate::hardware::memory::Memory;
use crate::hardware::ppu::Ppu;
use crate::utils::Register;
use std::cell::{Cell, RefCell};
use std::path::PathBuf;
use std::rc::Rc;
use crate::hardware::timer::Timer;

mod hardware;
mod utils;

pub const SCREEN_WIDTH: u8 = 160;
pub const SCREEN_HEIGHT: u8 = 144;

pub struct Demug {
    memory: RefCell<Option<Memory>>,
    cpu: RefCell<Option<Cpu>>,
    ppu: RefCell<Option<Ppu>>,
    timer: RefCell<Option<Timer>>,
    disable_boot_rom: Cell<bool>,
    interrupt_flags: Cell<Register>,
    interrupt_enable: Cell<Register>,
    #[cfg(feature = "debug")]bus_debug_info: RefCell<Option<BusDebugInfo>>,
}

impl Demug {
    pub fn init() -> Rc<RefCell<Demug>> {
        let demug = Rc::new(RefCell::new(Demug {
            memory: RefCell::new(None),
            cpu: RefCell::new(None),
            ppu: RefCell::new(None),
            timer: RefCell::new(None),
            disable_boot_rom: Cell::new(false),
            interrupt_flags: Cell::new(Register::new(0xE1)),
            interrupt_enable: Cell::new(Register::new(0x00)),
            #[cfg(feature = "debug")]bus_debug_info: RefCell::new(None),
        }));

        // Create all necessary 'devices'
        let memory = Memory::init();
        let cpu = Cpu::init(demug.clone());
        let ppu = Ppu::init(demug.clone());
        let timer = Timer::init(demug.clone());

        demug.borrow_mut().memory.replace(Some(memory));
        demug.borrow_mut().cpu.replace(Some(cpu));
        demug.borrow_mut().ppu.replace(Some(ppu));
        demug.borrow_mut().timer.replace(Some(timer));

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

    pub fn step_frame(&self) {
        if let Some(cpu) = &mut *self.cpu.borrow_mut() {
            cpu.execute_frame();
        }
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
        } else if (0x0100..=0x7FFF).contains(&addr) {
            // FIXME: This should not be handled like that, the game rom should be accessed through a read method of a cartridge management struct
            if let Some(mem) = &*self.memory.borrow() {
                data = mem.game_rom[addr];
            }
        } else if (0x8000..=0x9FFF).contains(&addr) {
            if let Some(mem) = &*self.memory.borrow() {
                data = mem.vram[addr - 0x8000];
            }
        } else if (0xC000..=0xDFFF).contains(&addr) {
            if let Some(mem) = &*self.memory.borrow() {
                data = mem.wram[addr - 0xC000];
            }
        } else if (0xE000..=0xFDFF).contains(&addr) {
            if let Some(mem) = &*self.memory.borrow() {
                data = mem.wram[addr - 0xE000];
            }
        } else if (0xFE00..=0xFE9F).contains(&addr) {
            if let Some(mem) = &*self.memory.borrow() {
                data = mem.oam[addr - 0xFE00];
            }
        } else if (0xFF04..=0xFF07).contains(&addr) { 
            if let Some(timer) = &*self.timer.borrow() {
                data = timer.read(addr as u16);
            }
        } else if addr == 0xFF0F {
            data = self.interrupt_flags.get().value()
        } else if (0xFF40..=0xFF4B).contains(&addr) {
            if let Some(ppu) = &*self.ppu.borrow() {
                data = ppu.read(addr as u16)
            }
        } else if (0xFF80..=0xFFFE).contains(&addr) {
            if let Some(mem) = &*self.memory.borrow() {
                data = mem.hram[addr - 0xFF80];
            }
        } else if addr == 0xFFFF {
            data = self.interrupt_enable.get().value();
        }
        
        #[cfg(feature = "debug")] {
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

        if (0x8000..=0x9FFF).contains(&addr) {
            if let Some(mem) = &mut *self.memory.borrow_mut() {
                mem.vram[addr - 0x8000] = data;
            }
        } else if (0xC000..=0xDFFF).contains(&addr) {
            if let Some(mem) = &mut *self.memory.borrow_mut() {
                mem.wram[addr - 0xC000] = data;
            }
        } else if (0xFE00..=0xFE9F).contains(&addr) { 
            if let Some(mem) = &mut *self.memory.borrow_mut() {
                mem.oam[addr - 0xFE00] = data;
            }
        } else if (0xFF04..=0xFF07).contains(&addr) { 
            if let Some(timer) = &mut *self.timer.borrow_mut() {
                timer.write(addr as u16, data);
            }
        } else if addr == 0xFF0F {
            let mut new_register = self.interrupt_flags.get();
            new_register.set_value(data);
            self.interrupt_flags.set(new_register);
        } else if (0xFF40..=0xFF4B).contains(&addr) {
            if let Some(ppu) = &mut *self.ppu.borrow_mut() {
                ppu.write(addr as u16, data);
            }
        } else if addr == 0xFF50 && data != 0x00 {
            self.disable_boot_rom.set(true);
            let mut new_register = self.interrupt_flags.get();
            new_register.set_value(data);
            self.interrupt_flags.set(new_register);
            println!("Disabling Boot Rom");
        } else if (0xFF80..=0xFFFE).contains(&addr) {
            if let Some(mem) = &mut *self.memory.borrow_mut() {
                mem.hram[addr - 0xFF80] = data;
            }
        } else if addr == 0xFFFF {
            let mut new_register = self.interrupt_enable.get();
            new_register.set_value(data);
            self.interrupt_enable.set(new_register);
        }

        #[cfg(feature = "debug")] {
            let debug_info = BusDebugInfo {
                last_accessed_addr: addr as u16,
                last_accessed_addr_mode: AccessMode::WRITE,
            };

            self.bus_debug_info.borrow_mut().replace(debug_info);
        }
    }

    fn tick(&self, m_cycles: u64) {
        if let (Some(ppu), Some(timer)) = (&mut *self.ppu.borrow_mut(), &mut *self.timer.borrow_mut()) {
            ppu.tick(m_cycles);
            timer.tick(m_cycles);
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