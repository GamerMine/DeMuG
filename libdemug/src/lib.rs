use std::cell::RefCell;
use crate::hardware::cpu::Cpu;
use std::sync::Arc;
use crate::hardware::memory::Memory;

mod hardware;

pub struct Demug {
    cpu: Option<Cpu>,
    memory: Option<Memory>
}

impl Demug {
    pub fn init() -> Arc<RefCell<Self>> {
        let demug = Arc::new(RefCell::new(Demug { cpu: None, memory: None }));

        // Create all necessary 'devices'
        let cpu = Cpu::new(demug.clone());
        let memory = Memory::init();

        demug.borrow_mut().cpu = Some(cpu);
        demug.borrow_mut().memory = Some(memory);

        demug
    }
    
    fn read(&self, addr: u16) -> u8 {
        if let Some(mem) = &self.memory {
            let mut data: u8 = 0x00;
            let addr = addr as usize;

            if addr <= 0x0100 {
                data = mem.boot_rom[addr];
            } else if addr >= 0xC000 && addr <= 0xDFFF {
                data = mem.wram[addr - 0xC000];
            } else if addr >= 0xFF80 && addr <= 0xFFFE {
                data = mem.hram[addr - 0xFF80];
            }

            data
        } else {
            unreachable!()
        }
    }

    fn write(&mut self, addr: u16, data: u8) {
        if let Some(mem) = &mut self.memory {
            let addr = addr as usize;

            if addr >= 0xC000 && addr <= 0xDFFF {
                mem.wram[addr - 0xC000] = data;
            } else if addr >= 0xFF80 && addr <= 0xFFFE {
                mem.hram[addr - 0xFF80] = data;
            }
        }
    }
}
