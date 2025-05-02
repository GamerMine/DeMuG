use crate::Demug;
use std::cell::RefCell;
use std::sync::Arc;

enum Registers16Bit {
    BC,
    DE,
    HL,
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
    mcycles: u64,
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
                pc: 0x0000,
                sp: 0xfffe,
            },
            mcycles: 0,
        }
    }

    fn get_16bit_register(&self, register: Registers16Bit) -> u16 {
        match register {
            Registers16Bit::BC => (self.registers.b as u16) << 8 | self.registers.c as u16,
            Registers16Bit::DE => (self.registers.d as u16) << 8 | self.registers.e as u16,
            Registers16Bit::HL => (self.registers.h as u16) << 8 | self.registers.l as u16,
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

    fn fetch_byte(&mut self) -> u8 {
        let data: u8 = self.bus.borrow().read(self.registers.pc);
        self.registers.pc += 1;
        self.mcycles += 1;

        data
    }

    fn fetch_word(&mut self) -> u16 {
        let data_lo: u8 = self.fetch_byte();
        let data_hi: u8 = self.fetch_byte();

        (data_hi as u16) << 8 | data_lo as u16
    }

    fn read_byte(&mut self, addr: u16) -> u8 {
        let data = self.bus.borrow().read(addr);

        self.mcycles += 1;

        data
    }

    fn write_byte(&mut self, addr: u16, data: u8) {
        self.bus.borrow_mut().write(addr, data);
        self.mcycles += 1;
    }

    fn write_word(&mut self, addr: u16, data: u16) {
        self.write_byte(addr, data as u8);
        self.write_byte(addr + 1, (data >> 8) as u8);
    }

    fn execute(&mut self, opcode: u8) {
        match opcode {
            0x00 => { /* NOP */ }
            0x01 => {
                /* LD BC, n16 */
                let data = self.fetch_word();
                self.set_16bit_register(Registers16Bit::BC, data);
            }
            0x02 => {
                /* LD [BC], A */
                self.write_byte(
                    self.get_16bit_register(Registers16Bit::BC),
                    self.registers.a,
                );
            }
            0x03 => {
                /* INC BC */
                self.set_16bit_register(
                    Registers16Bit::BC,
                    self.get_16bit_register(Registers16Bit::BC) + 1,
                );
                self.mcycles += 1;
            }
            0x04 => {
                /* INC B */
                self.registers.b = self.registers.b.wrapping_add(1);
                self.increment8_flag(self.registers.b);
            }
            0x05 => {
                /* DEC B */
                self.registers.b = self.registers.b.wrapping_add_signed(-1);
                self.decrement8_flag(self.registers.b);
            }
            0x06 => {
                /* LD B, n8 */
                let data = self.fetch_byte();
                self.registers.b = data;
            }
            0x07 => {
                /* RLCA */
                self.set_carry(self.registers.a >> 7 == 0x1);
                self.set_zero(false);
                self.set_negative(false);
                self.set_half_carry(false);

                self.registers.a <<= 1;
            }
            0x08 => {
                /* LD [a16], SP */
                let addr = self.fetch_word();
                self.write_word(addr, self.registers.sp);
            }
            0x09 => {
                /* ADD HL, BC */
                let original_value = self.get_16bit_register(Registers16Bit::HL);

                self.set_16bit_register(
                    Registers16Bit::HL,
                    original_value.wrapping_add(self.registers.sp),
                );
                self.add16_flag(original_value, self.get_16bit_register(Registers16Bit::HL));

                self.mcycles += 1;
            }
            0x0A => {
                /* LD A, [BC] */
                let value = self.read_byte(self.get_16bit_register(Registers16Bit::BC));
                self.registers.a = value;
            }
            0x0B => {
                /* DEC BC */
                self.set_16bit_register(
                    Registers16Bit::BC,
                    self.get_16bit_register(Registers16Bit::BC)
                        .wrapping_add_signed(-1),
                )
            }
            0x0C => {
                /* INC C */
                self.registers.c = self.registers.c.wrapping_add(1);
                self.increment8_flag(self.registers.c);
            }
            0x0D => {
                /* DEC C */
                self.registers.c = self.registers.c.wrapping_add_signed(-1);
                self.decrement8_flag(self.registers.c);
            }
            0x0E => {
                /* LD C, n8 */
                let data = self.fetch_byte();
                self.registers.c = data;
            }
            0x0F => {
                /* RRCA */
                self.set_carry(self.registers.a & 0x01 == 0x1);
                self.set_zero(false);
                self.set_negative(false);
                self.set_half_carry(false);

                self.registers.a >>= 1;
            }
            _ => {}
        }
    }

    fn increment8_flag(&mut self, value: u8) {
        self.set_zero(value == 0x00);
        self.set_half_carry(value & 0x0F == 0x00);
        self.set_negative(false);
    }

    fn decrement8_flag(&mut self, value: u8) {
        self.set_zero(value == 0x00);
        self.set_half_carry(value & 0x0F == 0x0F);
        self.set_negative(true);
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
}
