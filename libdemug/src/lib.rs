use crate::hardware::cpu::Cpu;
use crate::hardware::memory::Memory;
use crate::hardware::ppu::Ppu;
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
}

impl Demug {
    pub fn init() -> Arc<RefCell<Self>> {
        let demug = Arc::new(RefCell::new(Demug {
            memory: RefCell::new(None),
            cpu: RefCell::new(None),
            ppu: RefCell::new(None),
            disable_boot_rom: Cell::new(false),
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
        if let Some(ppu) = &mut *self.ppu.borrow_mut() {
            ppu.get_frame()
        } else {
            unreachable!()
        }
    }

    pub fn step(&self) -> bool {
        if let Some(cpu) = &mut *self.cpu.borrow_mut() {
            cpu.execute();
        }
        if let Some(ppu) = &*self.ppu.borrow() {
            ppu.is_frame_ready()
        } else {
            unreachable!()
        }
    }

    pub fn disable_boot_rom(&mut self, disabled: bool) {
        self.disable_boot_rom.set(disabled);
    }

    fn read(&self, addr: u16) -> u8 {
        let mut data: u8 = 0x00;
        let addr = addr as usize;

        if addr <= 0x00FF && !self.disable_boot_rom.get() {
            if let Some(mem) = &*self.memory.borrow() {
                data = mem.boot_rom[addr];
            }
        } else if addr <= 0x00FF && self.disable_boot_rom.get() {
            if let Some(mem) = &*self.memory.borrow() {
                data = mem.game_rom[addr];
            }
        } else if addr >= 0x0100 && addr <= 0x3FFF {
            if let Some(mem) = &*self.memory.borrow() {
                data = mem.game_rom[addr];
            }
        } else if addr >= 0xC000 && addr <= 0xDFFF {
            if let Some(mem) = &*self.memory.borrow() {
                data = mem.wram[addr - 0xC000];
            }
        } else if addr >= 0x8000 && addr <= 0x9FFF {
            if let Some(mem) = &*self.memory.borrow() {
                data = mem.vram[addr - 0x8000];
            }
        } else if addr >= 0xFF40 && addr <= 0xFF4B {
            if let Some(ppu) = &*self.ppu.borrow() {
                data = ppu.read(addr as u16)
            }
        } else if addr >= 0xFF80 && addr <= 0xFFFE {
            if let Some(mem) = &*self.memory.borrow() {
                data = mem.hram[addr - 0xFF80];
            }
        } else {
            println!("Read from {:#X} is not implemented!", addr);
        }

        data
    }

    fn write(&self, addr: u16, data: u8) {
        if let (Some(mem), Some(ppu)) = (&mut *self.memory.borrow_mut(), &mut *self.ppu.borrow_mut()) {
            let addr = addr as usize;

            if addr >= 0xC000 && addr <= 0xDFFF {
                mem.wram[addr - 0xC000] = data;
            } else if addr >= 0x8000 && addr <= 0x9FFF {
                mem.vram[addr - 0x8000] = data;
            } else if addr >= 0xFF40 && addr <= 0xFF4B {
                ppu.write(addr as u16, data);
            } else if addr == 0xFF50 && data != 0x00 {
                self.disable_boot_rom.set(true);
                println!("Disabling Boot Rom");
            } else if addr >= 0xFF80 && addr <= 0xFFFE {
                mem.hram[addr - 0xFF80] = data;
            } else {
                println!("Write to {:#X} is not implemented!", addr);
            }
        }
    }

    fn tick(&self, m_cycles: u64) {
        if let (Some(ppu)) = (&mut *self.ppu.borrow_mut()) {
            ppu.tick(m_cycles);
        }
    }
}
