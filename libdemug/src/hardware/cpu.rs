mod opcodes;

use crate::hardware::cpu::opcodes::{OPCODES, OPCODES_STRING};
use crate::Demug;
use std::cell::RefCell;
use std::sync::Arc;

enum Registers16Bit {
    BC,
    DE,
    HL,
    AF,
}

#[repr(u8)]
pub(crate) enum Interrupts {
    Joypad = 4,
    Serial = 3,
    Timer = 2,
    Lcd = 1,
    Vblank = 0,
}

#[derive(Clone)]
pub struct CpuRegisters {
    pub a: u8,
    pub f: u8,
    pub b: u8,
    pub c: u8,
    pub d: u8,
    pub e: u8,
    pub h: u8,
    pub l: u8,
    pub pc: u16,
    pub sp: u16,
}

pub struct Cpu {
    bus: Arc<RefCell<Demug>>,
    registers: CpuRegisters,
    m_cycles: u64,
    ime: bool,
}

impl Cpu {
    pub fn init(bus: Arc<RefCell<Demug>>) -> Self {
        Self {
            bus,
            registers: CpuRegisters {
                a: 0x01,
                f: 0x80,
                b: 0x00,
                c: 0x13,
                d: 0x00,
                e: 0xd8,
                h: 0x01,
                l: 0x4d,
                pc: 0x0000,
                sp: 0xfffe,
            },
            m_cycles: 0,
            ime: false,
        }
    }

    pub fn execute(&mut self) {
        let opcode = self.fetch_byte();

        if self.m_cycles <= 17556 {
            let old_m_cycles = self.m_cycles;

            OPCODES[opcode as usize](self);

            self.bus.borrow().tick(self.m_cycles - old_m_cycles);

            if self.ime {
                self.check_interrupts()
            }
        } else {
            self.m_cycles -= 17556;
        }
    }

    #[cfg(feature = "debug")]
    pub fn gather_debug_info(&self) -> CpuDebugInfo {
        let opcode = self.bus.borrow().read(self.registers.pc);
        let prefixed_opcode = self.bus.borrow().read(self.registers.pc + 1);
        
        CpuDebugInfo {
            registers: self.registers.clone(),
            next_instr: OPCODES_STRING[opcode as usize](prefixed_opcode),
            next_instr_opcode: opcode,
            next_instr_pfx_opcode: prefixed_opcode,
        }
    }

    fn check_interrupts(&mut self) {
        let mut interrupt_triggered = (false, 0x0000);
        if self.bus.borrow().interrupt_enable.get().bit(Interrupts::Vblank as u8) == 0b1
            && self.bus.borrow().interrupt_flags.get().bit(Interrupts::Vblank as u8) == 0b1
        {
            interrupt_triggered.0 = true;
            interrupt_triggered.1 = 0x0040;
            let mut register_new = self.bus.borrow().interrupt_flags.get();
            register_new.clear(Interrupts::Vblank as u8);
            self.bus.borrow().interrupt_flags.set(register_new);
        }
        else if self.bus.borrow().interrupt_enable.get().bit(Interrupts::Lcd as u8) == 0b1
            && self.bus.borrow().interrupt_flags.get().bit(Interrupts::Lcd as u8) == 0b1
        {
            interrupt_triggered.0 = true;
            interrupt_triggered.1 = 0x0048;
            let mut register_new = self.bus.borrow().interrupt_flags.get();
            register_new.clear(Interrupts::Lcd as u8);
            self.bus.borrow().interrupt_flags.set(register_new);
        }

        if interrupt_triggered.0 {
            self.ime = false;
            self.m_cycles += 2;
            self.write_word(self.registers.sp - 2, self.registers.sp - 1, self.registers.pc);
            self.registers.sp -= 2;
            self.registers.pc = interrupt_triggered.1;
        }
        // TODO: Implements Timer, Serial, Joypad interrupts
    }

    fn get_16bit_register(&self, register: Registers16Bit) -> u16 {
        match register {
            Registers16Bit::BC => (self.registers.b as u16) << 8 | self.registers.c as u16,
            Registers16Bit::DE => (self.registers.d as u16) << 8 | self.registers.e as u16,
            Registers16Bit::HL => (self.registers.h as u16) << 8 | self.registers.l as u16,
            Registers16Bit::AF => (self.registers.a as u16) << 8 | self.registers.f as u16,
        }
    }

    fn set_16bit_register(&mut self, register: Registers16Bit, value: u16) {
        match register {
            Registers16Bit::BC => {
                self.registers.b = (value >> 8) as u8;
                self.registers.c = value as u8;
            }
            Registers16Bit::DE => {
                self.registers.d = (value >> 8) as u8;
                self.registers.e = value as u8;
            }
            Registers16Bit::HL => {
                self.registers.h = (value >> 8) as u8;
                self.registers.l = value as u8;
            }
            Registers16Bit::AF => {
                self.registers.a = (value >> 8) as u8;
                self.registers.f = value as u8;
            }
        }
    }

    fn set_carry(&mut self, set: bool) {
        if set {
            self.registers.f |= 0x10;
        } else {
            self.registers.f &= 0xE0;
        }
    }
    fn set_half_carry(&mut self, set: bool) {
        if set {
            self.registers.f |= 0x20;
        } else {
            self.registers.f &= 0xD0;
        }
    }
    fn set_negative(&mut self, set: bool) {
        if set {
            self.registers.f |= 0x40;
        } else {
            self.registers.f &= 0xB0;
        }
    }
    fn set_zero(&mut self, set: bool) {
        if set {
            self.registers.f |= 0x80;
        } else {
            self.registers.f &= 0x70;
        }
    }

    fn carry(&self) -> u8 {
        (self.registers.f >> 4) & 0x01
    }
    fn half_carry(&self) -> u8 {
        (self.registers.f >> 5) & 0x01
    }
    fn negative(&self) -> u8 {
        (self.registers.f >> 6) & 0x01
    }
    fn zero(&self) -> u8 {
        self.registers.f >> 7
    }

    fn fetch_byte(&mut self) -> u8 {
        let data: u8 = self.bus.borrow().read(self.registers.pc);
        self.registers.pc += 1;
        self.m_cycles += 1;

        data
    }

    fn fetch_word(&mut self) -> u16 {
        let data_lo: u8 = self.fetch_byte();
        let data_hi: u8 = self.fetch_byte();

        (data_hi as u16) << 8 | data_lo as u16
    }

    fn read_byte(&mut self, addr: u16) -> u8 {
        let data = self.bus.borrow().read(addr);

        self.m_cycles += 1;

        data
    }

    fn write_byte(&mut self, addr: u16, data: u8) {
        self.bus.borrow().write(addr, data);
        self.m_cycles += 1;
    }

    fn write_word(&mut self, addr_lo: u16, addr_hi: u16, data: u16) {
        self.write_byte(addr_lo, data as u8);
        self.write_byte(addr_hi, (data >> 8) as u8);
    }

    fn increment8_flag(&mut self, res_value: u8) {
        self.set_zero(res_value == 0x00);
        self.set_half_carry(res_value - 0b1 & 0x0F == 0x0F);
        self.set_negative(false);
    }

    fn decrement8_flag(&mut self, value: u8) {
        self.set_zero(value == 0x00);
        self.set_half_carry(value & 0x0F == 0x0F);
        self.set_negative(true);
    }

    fn add8_flag(&mut self, base_value: u8, result_value: u8) {
        self.set_zero(result_value == 0x00);
        if result_value < base_value {
            self.set_half_carry(true);
            self.set_carry(true);
        } else if result_value >> 4 > base_value >> 4 {
            self.set_half_carry(true);
        }
        self.set_negative(false);
    }

    fn add16_flag(&mut self, base_value: u16, result_value: u16) {
        if result_value < base_value {
            self.set_half_carry(true);
            self.set_carry(true);
        } else if result_value >> 12 > base_value >> 12 {
            self.set_half_carry(true);
        }
        self.set_negative(false);
    }

    fn subtract8_flag(&mut self, base_value: u8, value: u8) {
        self.set_zero(base_value.wrapping_sub(value) == 0x00);
        self.set_half_carry(base_value & 0x0F < value & 0x0F);
        self.set_carry(base_value < value);
        self.set_negative(true);
    }

    fn and8_flag(&mut self, value: u8) {
        self.set_zero(value == 0x00);
        self.set_half_carry(true);
        self.set_carry(false);
        self.set_negative(false);
    }

    fn xor8_flag(&mut self, value: u8) {
        self.set_zero(value == 0x00);
        self.set_half_carry(false);
        self.set_carry(false);
        self.set_negative(false);
    }

    fn cp8_flag(&mut self, value: u8) {
        self.set_zero(self.registers.a == value);
        self.set_half_carry(self.registers.a & 0x0F < value & 0x0F);
        self.set_carry(self.registers.a < value);
        self.set_negative(true);
    }

    fn rotate8_flag(&mut self, value: u8, is_left: bool, set_carry: bool) -> bool {
        let mut carry = false;
        if is_left {
            carry = value >> 7 == 0x1;
        } else {
            carry = value & 0x1 == 0x1;
        }

        if set_carry {
            self.set_carry(carry);
        }
        self.set_zero(value == 0x00);
        self.set_negative(false);
        self.set_half_carry(false);

        carry
    }

    fn bit8_flag(&mut self, bit: u8, value: u8) {
        self.set_zero(value >> bit & 0b1 == 0b0);
        self.set_half_carry(true);
        self.set_negative(false);
    }
}

#[cfg(feature = "debug")]
pub struct CpuDebugInfo {
    pub registers: CpuRegisters,
    pub next_instr: &'static str,
    pub next_instr_opcode: u8, 
    pub next_instr_pfx_opcode: u8,
}