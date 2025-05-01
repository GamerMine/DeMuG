use std::cell::RefCell;
use std::sync::Arc;
use crate::Demug;

enum WordsRegister {
    BC,
    DE,
    HL
}

struct CpuRegisters {
    a: u8,
    f: u8,
    b: u8,
    c: u8,
    d: u8,
    e: u8,
    h: u8,
    l: u8,
    pc: u16,
    sp: u16,
}

pub struct Cpu {
    bus: Arc<RefCell<Demug>>,
    registers: CpuRegisters,
}

impl Cpu {
    pub fn new(bus: Arc<RefCell<Demug>>) -> Self {
        Self {
            bus,
            registers: CpuRegisters {
                a: 0x01,
                f: 0x00,
                b: 0x00,
                c: 0x13,
                d: 0x00,
                e: 0xd8,
                h: 0x01,
                l: 0x4d,
                pc: 0x0100,
                sp: 0xfffe,
            }
        }
    }

    fn get_16bit_register(&self, word_register: WordsRegister) -> u16 {
        match word_register {
            WordsRegister::BC => {
                (self.registers.b as u16) << 8 | self.registers.c as u16
            }
            WordsRegister::DE => {
                (self.registers.d as u16) << 8 | self.registers.e as u16
            }
            WordsRegister::HL => {
                (self.registers.h as u16) << 8 | self.registers.l as u16
            }
        }
    }
    
    fn set_carry(&mut self) {
        self.registers.f |= 0x10;
    }
    
    fn clear_carry(&mut self) {
        self.registers.f &= 0xE0;
    }
    
    fn set_half_carry(&mut self) {
        self.registers.f |= 0x20;
    }
    
    fn clear_half_carry(&mut self) {
        self.registers.f &= 0xD0;
    }
}