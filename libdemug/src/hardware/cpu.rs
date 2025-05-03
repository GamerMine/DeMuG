use crate::Demug;
use std::cell::RefCell;
use std::sync::Arc;

enum Registers16Bit {
    BC,
    DE,
    HL,
    AF,
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
    ime: bool,
}

static mut OPCODES: [fn(&mut Cpu); 0x100] = [
    |_| {
        /* 0x00 */
        /* NOP */
    },
    |cpu| {
        /* 0x01 */
        /* LD BC, n16 */
        let data = cpu.fetch_word();
        cpu.set_16bit_register(Registers16Bit::BC, data);
    },
    |cpu| {
        /* 0x02 */
        /* LD [BC], A */
        cpu.write_byte(cpu.get_16bit_register(Registers16Bit::BC), cpu.registers.a);
    },
    |cpu| {
        /* 0x03 */
        /* INC BC */
        cpu.set_16bit_register(
            Registers16Bit::BC,
            cpu.get_16bit_register(Registers16Bit::BC).wrapping_add(1),
        );
        cpu.mcycles += 1;
    },
    |cpu| {
        /* 0x04 */
        /* INC B */
        cpu.registers.b = cpu.registers.b.wrapping_add(1);
        cpu.increment8_flag(cpu.registers.b);
    },
    |cpu| {
        /* 0x05 */
        /* DEC B */
        cpu.registers.b = cpu.registers.b.wrapping_sub(1);
        cpu.decrement8_flag(cpu.registers.b);
    },
    |cpu| {
        /* 0x06 */
        /* LD B, n8 */
        let data = cpu.fetch_byte();
        cpu.registers.b = data;
    },
    |cpu| {
        /* 0x07 */
        /* RLCA */
        cpu.set_carry(cpu.registers.a >> 7 == 0x1);
        cpu.set_zero(false);
        cpu.set_negative(false);
        cpu.set_half_carry(false);

        cpu.registers.a = cpu.registers.a.rotate_left(1);
    },
    |cpu| {
        /* 0x08 */
        /* LD [a16], SP */
        let addr = cpu.fetch_word();
        cpu.write_word(addr, addr + 1, cpu.registers.sp);
    },
    |cpu| {
        /* 0x09 */
        /* ADD HL, BC */
        let original_value = cpu.get_16bit_register(Registers16Bit::HL);

        cpu.set_16bit_register(
            Registers16Bit::HL,
            original_value.wrapping_add(cpu.get_16bit_register(Registers16Bit::BC)),
        );
        cpu.add16_flag(original_value, cpu.get_16bit_register(Registers16Bit::HL));

        cpu.mcycles += 1;
    },
    |cpu| {
        /* 0x0A */
        /* LD A, [BC] */
        let value = cpu.read_byte(cpu.get_16bit_register(Registers16Bit::BC));
        cpu.registers.a = value;
    },
    |cpu| {
        /* 0x0B */
        /* DEC BC */
        cpu.set_16bit_register(
            Registers16Bit::BC,
            cpu.get_16bit_register(Registers16Bit::BC).wrapping_sub(1),
        )
    },
    |cpu| {
        /* 0x0C */
        /* INC C */
        cpu.registers.c = cpu.registers.c.wrapping_add(1);
        cpu.increment8_flag(cpu.registers.c);
    },
    |cpu| {
        /* 0x0D */
        /* DEC C */
        cpu.registers.c = cpu.registers.c.wrapping_sub(1);
        cpu.decrement8_flag(cpu.registers.c);
    },
    |cpu| {
        /* 0x0E */
        /* LD C, n8 */
        let data = cpu.fetch_byte();
        cpu.registers.c = data;
    },
    |cpu| {
        /* 0x0F */
        /* RRCA */
        cpu.set_carry(cpu.registers.a & 0x01 == 0x1);
        cpu.set_zero(false);
        cpu.set_negative(false);
        cpu.set_half_carry(false);

        cpu.registers.a = cpu.registers.a.rotate_right(1);
    },
    |_| {
        /* 0x10 */
        /* STOP n8 */
        unimplemented!()
    },
    |cpu| {
        /* 0x11 */
        /* LD DE, n16 */
        let data = cpu.fetch_word();
        cpu.set_16bit_register(Registers16Bit::DE, data);
    },
    |cpu| {
        /* 0x12 */
        /* LD [DE], A */
        cpu.write_byte(cpu.get_16bit_register(Registers16Bit::DE), cpu.registers.a);
    },
    |cpu| {
        /* 0x13 */
        /* INC DE */
        cpu.set_16bit_register(
            Registers16Bit::DE,
            cpu.get_16bit_register(Registers16Bit::DE).wrapping_add(1),
        );
        cpu.mcycles += 1;
    },
    |cpu| {
        /* 0x14 */
        /* INC D */
        cpu.registers.d = cpu.registers.d.wrapping_add(1);
        cpu.increment8_flag(cpu.registers.d);
    },
    |cpu| {
        /* 0x15 */
        /* DEC D */
        cpu.registers.d = cpu.registers.d.wrapping_sub(1);
        cpu.decrement8_flag(cpu.registers.d);
    },
    |cpu| {
        /* 0x16 */
        /* LD D, n8 */
        let data = cpu.fetch_byte();
        cpu.registers.d = data;
    },
    |cpu| {
        /* 0x17 */
        /* RLA */
        let carry = cpu.registers.a >> 7 == 0x1;
        cpu.set_zero(false);
        cpu.set_half_carry(false);
        cpu.set_negative(false);

        cpu.registers.a = cpu.registers.a << 1 | cpu.carry();

        cpu.set_carry(carry);
    },
    |cpu| {
        /* 0x18 */
        /* JR e8 */
        let value = cpu.fetch_byte() as i8 as i16;
        cpu.registers.pc = cpu.registers.pc.wrapping_add_signed(value);
        cpu.mcycles += 1;
    },
    |cpu| {
        /* 0x19 */
        /* ADD HL, DE */
        let original_value = cpu.get_16bit_register(Registers16Bit::HL);

        cpu.set_16bit_register(
            Registers16Bit::HL,
            original_value.wrapping_add(cpu.get_16bit_register(Registers16Bit::DE)),
        );
        cpu.add16_flag(original_value, cpu.get_16bit_register(Registers16Bit::HL));

        cpu.mcycles += 1;
    },
    |cpu| {
        /* 0x1A */
        /* LD A, [DE] */
        let value = cpu.read_byte(cpu.get_16bit_register(Registers16Bit::DE));
        cpu.registers.a = value;
    },
    |cpu| {
        /* 0x1B */
        /* DEC DE */
        cpu.set_16bit_register(
            Registers16Bit::DE,
            cpu.get_16bit_register(Registers16Bit::DE).wrapping_sub(1),
        )
    },
    |cpu| {
        /* 0x1C */
        /* INC E */
        cpu.registers.e = cpu.registers.e.wrapping_add(1);
        cpu.increment8_flag(cpu.registers.e);
    },
    |cpu| {
        /* 0x1D */
        /* DEC E */
        cpu.registers.e = cpu.registers.e.wrapping_sub(1);
        cpu.decrement8_flag(cpu.registers.e);
    },
    |cpu| {
        /* 0x1E */
        /* LD E, n8 */
        let data = cpu.fetch_byte();
        cpu.registers.e = data;
    },
    |cpu| {
        /* 0x1F */
        /* RRA */
        let carry = cpu.registers.a & 0x1 == 0x1;
        cpu.set_zero(false);
        cpu.set_half_carry(false);
        cpu.set_negative(false);

        cpu.registers.a = cpu.registers.a >> 1 | cpu.carry() << 7;

        cpu.set_carry(carry);
    },
    |cpu| {
        /* 0x20 */
        /* JR NZ, e8 */
        let value = cpu.fetch_byte() as i8 as i16;
        if cpu.zero() != 0x01 {
            cpu.registers.pc = cpu.registers.pc.wrapping_add_signed(value);
            cpu.mcycles += 1;
        }
    },
    |cpu| {
        /* 0x21 */
        /* LD HL, n16 */
        let data = cpu.fetch_word();
        cpu.set_16bit_register(Registers16Bit::HL, data);
    },
    |cpu| {
        /* 0x22 */
        /* LD [HL+], A */
        cpu.write_byte(cpu.get_16bit_register(Registers16Bit::HL), cpu.registers.a);
        cpu.set_16bit_register(
            Registers16Bit::HL,
            cpu.get_16bit_register(Registers16Bit::HL).wrapping_add(1),
        );
    },
    |cpu| {
        /* 0x23 */
        /* INC HL */
        cpu.set_16bit_register(
            Registers16Bit::HL,
            cpu.get_16bit_register(Registers16Bit::HL).wrapping_add(1),
        );
        cpu.mcycles += 1;
    },
    |cpu| {
        /* 0x24 */
        /* INC H */
        cpu.registers.h = cpu.registers.h.wrapping_add(1);
        cpu.increment8_flag(cpu.registers.h);
    },
    |cpu| {
        /* 0x25 */
        /* DEC H */
        cpu.registers.h = cpu.registers.h.wrapping_sub(1);
        cpu.decrement8_flag(cpu.registers.h);
    },
    |cpu| {
        /* 0x26 */
        /* LD H, n8 */
        let data = cpu.fetch_byte();
        cpu.registers.h = data;
    },
    |cpu| {
        /* 0x27 */
        /* DAA */
        if cpu.negative() == 0x00 {
            if cpu.carry() == 0x01 || cpu.registers.a > 0x99 {
                cpu.registers.a = cpu.registers.a.wrapping_add(0x60);
                cpu.set_carry(true);
            }
            if cpu.half_carry() == 0x01 || (cpu.registers.a & 0x0F) > 0x09 {
                cpu.registers.a = cpu.registers.a.wrapping_add(0x06);
            }
        } else {
            if cpu.carry() == 0x01 {
                cpu.registers.a = cpu.registers.a.wrapping_sub(0x60);
            }
            if cpu.half_carry() == 0x01 {
                cpu.registers.a = cpu.registers.a.wrapping_sub(0x06);
            }
        }

        cpu.set_zero(cpu.registers.a == 0x00);
        cpu.set_half_carry(false);
    },
    |cpu| {
        /* 0x28 */
        /* JR Z, e8 */
        let value = cpu.fetch_byte() as i8 as i16;
        if cpu.zero() == 0x01 {
            cpu.registers.pc = cpu.registers.pc.wrapping_add_signed(value);
            cpu.mcycles += 1;
        }
    },
    |cpu| {
        /* 0x29 */
        /* ADD HL, HL */
        let original_value = cpu.get_16bit_register(Registers16Bit::HL);

        cpu.set_16bit_register(
            Registers16Bit::HL,
            original_value.wrapping_add(cpu.get_16bit_register(Registers16Bit::HL)),
        );
        cpu.add16_flag(original_value, cpu.get_16bit_register(Registers16Bit::HL));

        cpu.mcycles += 1;
    },
    |cpu| {
        /* 0x2A */
        /* LD A, [HL+] */
        let data = cpu.read_byte(cpu.get_16bit_register(Registers16Bit::HL));

        cpu.registers.a = data;
        cpu.set_16bit_register(
            Registers16Bit::HL,
            cpu.get_16bit_register(Registers16Bit::HL).wrapping_add(1),
        );
    },
    |cpu| {
        /* 0x2B */
        /* DEC HL */
        cpu.set_16bit_register(
            Registers16Bit::HL,
            cpu.get_16bit_register(Registers16Bit::HL).wrapping_sub(1),
        )
    },
    |cpu| {
        /* 0x2C */
        /* INC L */
        cpu.registers.l = cpu.registers.l.wrapping_add(1);
        cpu.increment8_flag(cpu.registers.l);
    },
    |cpu| {
        /* 0x2D */
        /* DEC L */
        cpu.registers.l = cpu.registers.l.wrapping_sub(1);
        cpu.decrement8_flag(cpu.registers.l);
    },
    |cpu| {
        /* 0x2E */
        /* LD L, n8 */
        let data = cpu.fetch_byte();
        cpu.registers.l = data;
    },
    |cpu| {
        /* 0x2F */
        /* CPL */
        cpu.registers.a = !cpu.registers.a;
        cpu.set_half_carry(true);
        cpu.set_negative(true);
    },
    |cpu| {
        /* 0x30 */
        /* JR NC, e8 */
        let value = cpu.fetch_byte() as i8 as i16;
        if cpu.carry() != 0x01 {
            cpu.registers.pc = cpu.registers.pc.wrapping_add_signed(value);
            cpu.mcycles += 1;
        }
    },
    |cpu| {
        /* 0x31 */
        /* LD SP, n16 */
        let data = cpu.fetch_word();
        cpu.registers.sp = data;
    },
    |cpu| {
        /* 0x32 */
        /* LD [HL-], A */
        cpu.write_byte(cpu.get_16bit_register(Registers16Bit::HL), cpu.registers.a);
        cpu.set_16bit_register(
            Registers16Bit::HL,
            cpu.get_16bit_register(Registers16Bit::HL).wrapping_sub(1),
        );
    },
    |cpu| {
        /* 0x33 */
        /* INC SP */
        cpu.registers.sp = cpu.registers.sp.wrapping_add(1);
        cpu.mcycles += 1;
    },
    |cpu| {
        /* 0x34 */
        /* INC [HL] */
        let value = cpu
            .read_byte(cpu.get_16bit_register(Registers16Bit::HL))
            .wrapping_add(1);

        cpu.write_byte(cpu.get_16bit_register(Registers16Bit::HL), value);
        cpu.increment8_flag(value);
    },
    |cpu| {
        /* 0x35 */
        /* DEC [HL] */
        let value = cpu
            .read_byte(cpu.get_16bit_register(Registers16Bit::HL))
            .wrapping_sub(1);

        cpu.write_byte(cpu.get_16bit_register(Registers16Bit::HL), value);
        cpu.decrement8_flag(value);
    },
    |cpu| {
        /* 0x36 */
        /* LD [HL], n8 */
        let data = cpu.fetch_byte();

        cpu.write_byte(cpu.get_16bit_register(Registers16Bit::HL), data);
    },
    |cpu| {
        /* 0x37 */
        /* SCF */
        cpu.set_carry(true);
        cpu.set_half_carry(false);
        cpu.set_negative(false);
    },
    |cpu| {
        /* 0x38 */
        /* JR C, e8 */
        let value = cpu.fetch_byte() as i8 as i16;
        if cpu.carry() == 0x01 {
            cpu.registers.pc = cpu.registers.pc.wrapping_add_signed(value);
            cpu.mcycles += 1;
        }
    },
    |cpu| {
        /* 0x39 */
        /* ADD HL, SP */
        let original_value = cpu.get_16bit_register(Registers16Bit::HL);

        cpu.set_16bit_register(
            Registers16Bit::HL,
            original_value.wrapping_add(cpu.registers.sp),
        );
        cpu.add16_flag(original_value, cpu.get_16bit_register(Registers16Bit::HL));

        cpu.mcycles += 1;
    },
    |cpu| {
        /* 0x3A */
        /* LD A, [HL-] */
        let data = cpu.read_byte(cpu.get_16bit_register(Registers16Bit::HL));

        cpu.registers.a = data;
        cpu.set_16bit_register(
            Registers16Bit::HL,
            cpu.get_16bit_register(Registers16Bit::HL).wrapping_sub(1),
        );
    },
    |cpu| {
        /* 0x3B */
        /* DEC SP */
        cpu.registers.sp = cpu.registers.sp.wrapping_sub(1);
    },
    |cpu| {
        /* 0x3C */
        /* INC A */
        cpu.registers.a = cpu.registers.a.wrapping_add(1);
        cpu.increment8_flag(cpu.registers.a);
    },
    |cpu| {
        /* 0x3D */
        /* DEC A */
        cpu.registers.a = cpu.registers.a.wrapping_sub(1);
        cpu.decrement8_flag(cpu.registers.a);
    },
    |cpu| {
        /* 0x3E */
        /* LD A, n8 */
        let data = cpu.fetch_byte();
        cpu.registers.a = data;
    },
    |cpu| {
        /* 0x3F */
        /* CCF */
        cpu.set_carry(cpu.carry() != 0x01);
    },
    |cpu| {
        /* 0x40 */
        /* LD B, B */
        cpu.registers.b = cpu.registers.b;
    },
    |cpu| {
        /* 0x41 */
        /* LD B, C */
        cpu.registers.b = cpu.registers.c;
    },
    |cpu| {
        /* 0x42 */
        /* LD B, D */
        cpu.registers.b = cpu.registers.d;
    },
    |cpu| {
        /* 0x43 */
        /* LD B, E */
        cpu.registers.b = cpu.registers.e;
    },
    |cpu| {
        /* 0x44 */
        /* LD B, H */
        cpu.registers.b = cpu.registers.h;
    },
    |cpu| {
        /* 0x45 */
        /* LD B, L */
        cpu.registers.b = cpu.registers.l;
    },
    |cpu| {
        /* 0x46 */
        /* LD B, [HL] */
        let data = cpu.read_byte(cpu.get_16bit_register(Registers16Bit::HL));
        cpu.registers.b = data;
    },
    |cpu| {
        /* 0x47 */
        /* LD B, A */
        cpu.registers.b = cpu.registers.a;
    },
    |cpu| {
        /* 0x48 */
        /* LD C, B */
        cpu.registers.c = cpu.registers.b;
    },
    |cpu| {
        /* 0x49 */
        /* LD C, C */
        cpu.registers.c = cpu.registers.c;
    },
    |cpu| {
        /* 0x4A */
        /* LD C, D */
        cpu.registers.c = cpu.registers.d;
    },
    |cpu| {
        /* 0x4B */
        /* LD C, E */
        cpu.registers.c = cpu.registers.e;
    },
    |cpu| {
        /* 0x4C */
        /* LD C, H */
        cpu.registers.c = cpu.registers.h;
    },
    |cpu| {
        /* 0x4D */
        /* LD C, L */
        cpu.registers.c = cpu.registers.l;
    },
    |cpu| {
        /* 0x4E */
        /* LD C, [HL] */
        let data = cpu.read_byte(cpu.get_16bit_register(Registers16Bit::HL));
        cpu.registers.c = data;
    },
    |cpu| {
        /* 0x4F */
        /* LD C, A */
        cpu.registers.c = cpu.registers.a;
    },
    |cpu| {
        /* 0x50 */
        /* LD D, B */
        cpu.registers.d = cpu.registers.b;
    },
    |cpu| {
        /* 0x51 */
        /* LD D, C */
        cpu.registers.d = cpu.registers.c;
    },
    |cpu| {
        /* 0x52 */
        /* LD D, D */
        cpu.registers.d = cpu.registers.d;
    },
    |cpu| {
        /* 0x53 */
        /* LD D, E */
        cpu.registers.d = cpu.registers.e;
    },
    |cpu| {
        /* 0x54 */
        /* LD D, H */
        cpu.registers.d = cpu.registers.h;
    },
    |cpu| {
        /* 0x55 */
        /* LD D, L */
        cpu.registers.d = cpu.registers.l;
    },
    |cpu| {
        /* 0x56 */
        /* LD D, [HL] */
        let data = cpu.read_byte(cpu.get_16bit_register(Registers16Bit::HL));
        cpu.registers.d = data;
    },
    |cpu| {
        /* 0x57 */
        /* LD D, A */
        cpu.registers.d = cpu.registers.a;
    },
    |cpu| {
        /* 0x58 */
        /* LD E, B */
        cpu.registers.e = cpu.registers.b;
    },
    |cpu| {
        /* 0x59 */
        /* LD E, C */
        cpu.registers.e = cpu.registers.c;
    },
    |cpu| {
        /* 0x5A */
        /* LD E, D */
        cpu.registers.e = cpu.registers.d;
    },
    |cpu| {
        /* 0x5B */
        /* LD E, E */
        cpu.registers.e = cpu.registers.e;
    },
    |cpu| {
        /* 0x5C */
        /* LD E, H */
        cpu.registers.e = cpu.registers.h;
    },
    |cpu| {
        /* 0x5D */
        /* LD E, L */
        cpu.registers.e = cpu.registers.l;
    },
    |cpu| {
        /* 0x5E */
        /* LD E, [HL] */
        let data = cpu.read_byte(cpu.get_16bit_register(Registers16Bit::HL));
        cpu.registers.e = data;
    },
    |cpu| {
        /* 0x5F */
        /* LD E, A */
        cpu.registers.e = cpu.registers.a;
    },
    |cpu| {
        /* 0x60 */
        /* LD H, B */
        cpu.registers.h = cpu.registers.b;
    },
    |cpu| {
        /* 0x61 */
        /* LD H, C */
        cpu.registers.h = cpu.registers.c;
    },
    |cpu| {
        /* 0x62 */
        /* LD H, D */
        cpu.registers.h = cpu.registers.d;
    },
    |cpu| {
        /* 0x63 */
        /* LD H, E */
        cpu.registers.h = cpu.registers.e;
    },
    |cpu| {
        /* 0x64 */
        /* LD H, H */
        cpu.registers.h = cpu.registers.h;
    },
    |cpu| {
        /* 0x65 */
        /* LD H, L */
        cpu.registers.h = cpu.registers.l;
    },
    |cpu| {
        /* 0x66 */
        /* LD H, [HL] */
        let data = cpu.read_byte(cpu.get_16bit_register(Registers16Bit::HL));
        cpu.registers.h = data;
    },
    |cpu| {
        /* 0x67 */
        /* LD H, A */
        cpu.registers.h = cpu.registers.a;
    },
    |cpu| {
        /* 0x68 */
        /* LD L, B */
        cpu.registers.l = cpu.registers.b;
    },
    |cpu| {
        /* 0x69 */
        /* LD L, C */
        cpu.registers.l = cpu.registers.c;
    },
    |cpu| {
        /* 0x6A */
        /* LD L, D */
        cpu.registers.l = cpu.registers.d;
    },
    |cpu| {
        /* 0x6B */
        /* LD L, E */
        cpu.registers.l = cpu.registers.e;
    },
    |cpu| {
        /* 0x6C */
        /* LD L, H */
        cpu.registers.l = cpu.registers.h;
    },
    |cpu| {
        /* 0x6D */
        /* LD L, L */
        cpu.registers.l = cpu.registers.l;
    },
    |cpu| {
        /* 0x6E */
        /* LD L, [HL] */
        let data = cpu.read_byte(cpu.get_16bit_register(Registers16Bit::HL));
        cpu.registers.l = data;
    },
    |cpu| {
        /* 0x6F */
        /* LD L, A */
        cpu.registers.l = cpu.registers.a;
    },
    |cpu| {
        /* 0x70 */
        /* LD [HL], B */
        cpu.write_byte(cpu.get_16bit_register(Registers16Bit::HL), cpu.registers.b);
    },
    |cpu| {
        /* 0x71 */
        /* LD [HL], C */
        cpu.write_byte(cpu.get_16bit_register(Registers16Bit::HL), cpu.registers.c);
    },
    |cpu| {
        /* 0x72 */
        /* LD [HL], D */
        cpu.write_byte(cpu.get_16bit_register(Registers16Bit::HL), cpu.registers.d);
    },
    |cpu| {
        /* 0x73 */
        /* LD [HL], E */
        cpu.write_byte(cpu.get_16bit_register(Registers16Bit::HL), cpu.registers.e);
    },
    |cpu| {
        /* 0x74 */
        /* LD [HL], H */
        cpu.write_byte(cpu.get_16bit_register(Registers16Bit::HL), cpu.registers.h);
    },
    |cpu| {
        /* 0x75 */
        /* LD [HL], L */
        cpu.write_byte(cpu.get_16bit_register(Registers16Bit::HL), cpu.registers.l);
    },
    |_| {
        /* 0x76 */
        /* HALT */
        unimplemented!();
    },
    |cpu| {
        /* 0x77 */
        /* LD [HL], A */
        cpu.write_byte(cpu.get_16bit_register(Registers16Bit::HL), cpu.registers.a);
    },
    |cpu| {
        /* 0x78 */
        /* LD A, B */
        cpu.registers.a = cpu.registers.b;
    },
    |cpu| {
        /* 0x79 */
        /* LD A, C */
        cpu.registers.a = cpu.registers.c;
    },
    |cpu| {
        /* 0x7A */
        /* LD A, D */
        cpu.registers.a = cpu.registers.d;
    },
    |cpu| {
        /* 0x7B */
        /* LD A, E */
        cpu.registers.a = cpu.registers.e;
    },
    |cpu| {
        /* 0x7C */
        /* LD A, H */
        cpu.registers.a = cpu.registers.h;
    },
    |cpu| {
        /* 0x7D */
        /* LD A, L */
        cpu.registers.a = cpu.registers.l;
    },
    |cpu| {
        /* 0x7E */
        /* LD A, [HL] */
        let data = cpu.read_byte(cpu.get_16bit_register(Registers16Bit::HL));
        cpu.registers.a = data;
    },
    |cpu| {
        /* 0x7F */
        /* LD A, A */
        cpu.registers.a = cpu.registers.a;
    },
    |cpu| {
        /* 0x80 */
        /* ADD A, B */
        let original_value = cpu.registers.a;

        cpu.registers.a = cpu.registers.a.wrapping_add(cpu.registers.b);
        cpu.add8_flag(original_value, cpu.registers.a);
    },
    |cpu| {
        /* 0x81 */
        /* ADD A, C */
        let original_value = cpu.registers.a;

        cpu.registers.a = cpu.registers.a.wrapping_add(cpu.registers.c);
        cpu.add8_flag(original_value, cpu.registers.a);
    },
    |cpu| {
        /* 0x82 */
        /* ADD A, D */
        let original_value = cpu.registers.a;

        cpu.registers.a = cpu.registers.a.wrapping_add(cpu.registers.d);
        cpu.add8_flag(original_value, cpu.registers.a);
    },
    |cpu| {
        /* 0x83 */
        /* ADD A, E */
        let original_value = cpu.registers.a;

        cpu.registers.a = cpu.registers.a.wrapping_add(cpu.registers.e);
        cpu.add8_flag(original_value, cpu.registers.a);
    },
    |cpu| {
        /* 0x84 */
        /* ADD A, H */
        let original_value = cpu.registers.a;

        cpu.registers.a = cpu.registers.a.wrapping_add(cpu.registers.h);
        cpu.add8_flag(original_value, cpu.registers.a);
    },
    |cpu| {
        /* 0x85 */
        /* ADD A, L */
        let original_value = cpu.registers.a;

        cpu.registers.a = cpu.registers.a.wrapping_add(cpu.registers.l);
        cpu.add8_flag(original_value, cpu.registers.a);
    },
    |cpu| {
        /* 0x86 */
        /* ADD A, [HL] */
        let original_value = cpu.registers.a;
        let data = cpu.read_byte(cpu.get_16bit_register(Registers16Bit::HL));

        cpu.registers.a = cpu.registers.a.wrapping_add(data);
        cpu.add8_flag(original_value, cpu.registers.a);
    },
    |cpu| {
        /* 0x87 */
        /* ADD A, A */
        let original_value = cpu.registers.a;

        cpu.registers.a = cpu.registers.a.wrapping_add(cpu.registers.a);
        cpu.add8_flag(original_value, cpu.registers.a);
    },
    |cpu| {
        /* 0x88 */
        /* ADC A, B */
        let original_value = cpu.registers.a;

        cpu.registers.a = cpu
            .registers
            .a
            .wrapping_add(cpu.registers.b)
            .wrapping_add(cpu.carry());
        cpu.add8_flag(original_value, cpu.registers.a);
    },
    |cpu| {
        /* 0x89 */
        /* ADC A, C */
        let original_value = cpu.registers.a;

        cpu.registers.a = cpu
            .registers
            .a
            .wrapping_add(cpu.registers.c)
            .wrapping_add(cpu.carry());
        cpu.add8_flag(original_value, cpu.registers.a);
    },
    |cpu| {
        /* 0x8A */
        /* ADC A, D */
        let original_value = cpu.registers.a;

        cpu.registers.a = cpu
            .registers
            .a
            .wrapping_add(cpu.registers.d)
            .wrapping_add(cpu.carry());
        cpu.add8_flag(original_value, cpu.registers.a);
    },
    |cpu| {
        /* 0x8B */
        /* ADC A, E */
        let original_value = cpu.registers.a;

        cpu.registers.a = cpu
            .registers
            .a
            .wrapping_add(cpu.registers.e)
            .wrapping_add(cpu.carry());
        cpu.add8_flag(original_value, cpu.registers.a);
    },
    |cpu| {
        /* 0x8C */
        /* ADC A, H */
        let original_value = cpu.registers.a;

        cpu.registers.a = cpu
            .registers
            .a
            .wrapping_add(cpu.registers.h)
            .wrapping_add(cpu.carry());
        cpu.add8_flag(original_value, cpu.registers.a);
    },
    |cpu| {
        /* 0x8D */
        /* ADC A, L */
        let original_value = cpu.registers.a;

        cpu.registers.a = cpu
            .registers
            .a
            .wrapping_add(cpu.registers.l)
            .wrapping_add(cpu.carry());
        cpu.add8_flag(original_value, cpu.registers.a);
    },
    |cpu| {
        /* 0x8E */
        /* ADC A, [HL] */
        let original_value = cpu.registers.a;
        let data = cpu.read_byte(cpu.get_16bit_register(Registers16Bit::HL));

        cpu.registers.a = cpu.registers.a.wrapping_add(data).wrapping_add(cpu.carry());
        cpu.add8_flag(original_value, cpu.registers.a);
    },
    |cpu| {
        /* 0x8F */
        /* ADC A, A */
        let original_value = cpu.registers.a;

        cpu.registers.a = cpu
            .registers
            .a
            .wrapping_add(cpu.registers.a)
            .wrapping_add(cpu.carry());
        cpu.add8_flag(original_value, cpu.registers.a);
    },
    |cpu| {
        /* 0x90 */
        /* SUB A, B */
        cpu.subtract8_flag(cpu.registers.a, cpu.registers.b);
        cpu.registers.a = cpu.registers.a.wrapping_sub(cpu.registers.b);
    },
    |cpu| {
        /* 0x91 */
        /* SUB A, C */
        cpu.subtract8_flag(cpu.registers.a, cpu.registers.c);
        cpu.registers.a = cpu.registers.a.wrapping_sub(cpu.registers.c);
    },
    |cpu| {
        /* 0x92 */
        /* SUB A, D */
        cpu.subtract8_flag(cpu.registers.a, cpu.registers.d);
        cpu.registers.a = cpu.registers.a.wrapping_sub(cpu.registers.d);
    },
    |cpu| {
        /* 0x93 */
        /* SUB A, E */
        cpu.subtract8_flag(cpu.registers.a, cpu.registers.e);
        cpu.registers.a = cpu.registers.a.wrapping_sub(cpu.registers.e);
    },
    |cpu| {
        /* 0x94 */
        /* SUB A, H */
        cpu.subtract8_flag(cpu.registers.a, cpu.registers.h);
        cpu.registers.a = cpu.registers.a.wrapping_sub(cpu.registers.h);
    },
    |cpu| {
        /* 0x95 */
        /* SUB A, L */
        cpu.subtract8_flag(cpu.registers.a, cpu.registers.l);
        cpu.registers.a = cpu.registers.a.wrapping_sub(cpu.registers.l);
    },
    |cpu| {
        /* 0x96 */
        /* SUB A, [HL] */
        let value = cpu.read_byte(cpu.get_16bit_register(Registers16Bit::HL));

        cpu.subtract8_flag(cpu.registers.a, value);
        cpu.registers.a = cpu.registers.a.wrapping_sub(value);
    },
    |cpu| {
        /* 0x97 */
        /* SUB A, A */
        cpu.subtract8_flag(cpu.registers.a, cpu.registers.a);
        cpu.registers.a = cpu.registers.a.wrapping_sub(cpu.registers.a);
    },
    |cpu| {
        /* 0x98 */
        /* SBC A, B */
        cpu.subtract8_flag(cpu.registers.a, cpu.registers.b - cpu.carry());
        cpu.registers.a = cpu.registers.a.wrapping_sub(cpu.registers.b);
    },
    |cpu| {
        /* 0x99 */
        /* SBC A, C */
        cpu.subtract8_flag(cpu.registers.a, cpu.registers.c - cpu.carry());
        cpu.registers.a = cpu.registers.a.wrapping_sub(cpu.registers.c);
    },
    |cpu| {
        /* 0x9A */
        /* SBC A, D */
        cpu.subtract8_flag(cpu.registers.a, cpu.registers.d - cpu.carry());
        cpu.registers.a = cpu.registers.a.wrapping_sub(cpu.registers.d);
    },
    |cpu| {
        /* 0x9B */
        /* SBC A, E */
        cpu.subtract8_flag(cpu.registers.a, cpu.registers.e - cpu.carry());
        cpu.registers.a = cpu.registers.a.wrapping_sub(cpu.registers.e);
    },
    |cpu| {
        /* 0x9C */
        /* SBC A, H */
        cpu.subtract8_flag(cpu.registers.a, cpu.registers.h - cpu.carry());
        cpu.registers.a = cpu.registers.a.wrapping_sub(cpu.registers.h);
    },
    |cpu| {
        /* 0x9D */
        /* SBC A, L */
        cpu.subtract8_flag(cpu.registers.a, cpu.registers.l - cpu.carry());
        cpu.registers.a = cpu.registers.a.wrapping_sub(cpu.registers.l);
    },
    |cpu| {
        /* 0x9E */
        /* SBC A, [HL] */
        let data = cpu.read_byte(cpu.get_16bit_register(Registers16Bit::HL));

        cpu.subtract8_flag(cpu.registers.a, data - cpu.carry());
        cpu.registers.a = cpu.registers.a.wrapping_sub(data);
    },
    |cpu| {
        /* 0x9F */
        /* SBC A, A */
        cpu.subtract8_flag(cpu.registers.a, cpu.registers.a - cpu.carry());
        cpu.registers.a = cpu.registers.a.wrapping_sub(cpu.registers.a);
    },
    |cpu| {
        /* 0xA0 */
        /* AND A, B */
        cpu.registers.a &= cpu.registers.b;
        cpu.and8_flag(cpu.registers.a);
    },
    |cpu| {
        /* 0xA1 */
        /* AND A, C */
        cpu.registers.a &= cpu.registers.c;
        cpu.and8_flag(cpu.registers.a);
    },
    |cpu| {
        /* 0xA2 */
        /* AND A, D */
        cpu.registers.a &= cpu.registers.d;
        cpu.and8_flag(cpu.registers.a);
    },
    |cpu| {
        /* 0xA3 */
        /* AND A, E */
        cpu.registers.a &= cpu.registers.e;
        cpu.and8_flag(cpu.registers.a);
    },
    |cpu| {
        /* 0xA4 */
        /* AND A, h */
        cpu.registers.a &= cpu.registers.h;
        cpu.and8_flag(cpu.registers.a);
    },
    |cpu| {
        /* 0xA5 */
        /* AND A, L */
        cpu.registers.a &= cpu.registers.l;
        cpu.and8_flag(cpu.registers.a);
    },
    |cpu| {
        /* 0xA6 */
        /* AND A, [HL] */
        let value = cpu.read_byte(cpu.get_16bit_register(Registers16Bit::HL));

        cpu.registers.a &= value;
        cpu.and8_flag(cpu.registers.a);
    },
    |cpu| {
        /* 0xA7 */
        /* AND A, A */
        cpu.registers.a &= cpu.registers.a;
        cpu.and8_flag(cpu.registers.a);
    },
    |cpu| {
        /* 0xA8 */
        /* XOR A, B */
        cpu.registers.a ^= cpu.registers.b;
        cpu.xor8_flag(cpu.registers.a);
    },
    |cpu| {
        /* 0xA9 */
        /* XOR A, C */
        cpu.registers.a ^= cpu.registers.c;
        cpu.xor8_flag(cpu.registers.a);
    },
    |cpu| {
        /* 0xAA */
        /* XOR A, D */
        cpu.registers.a ^= cpu.registers.d;
        cpu.xor8_flag(cpu.registers.a);
    },
    |cpu| {
        /* 0xAB */
        /* XOR A, E */
        cpu.registers.a ^= cpu.registers.e;
        cpu.xor8_flag(cpu.registers.a);
    },
    |cpu| {
        /* 0xAC */
        /* XOR A, H */
        cpu.registers.a ^= cpu.registers.h;
        cpu.xor8_flag(cpu.registers.a);
    },
    |cpu| {
        /* 0xAD */
        /* XOR A, L */
        cpu.registers.a ^= cpu.registers.l;
        cpu.xor8_flag(cpu.registers.a);
    },
    |cpu| {
        /* 0xAE */
        /* XOR A, [HL] */
        let value = cpu.read_byte(cpu.get_16bit_register(Registers16Bit::HL));

        cpu.registers.a ^= value;
        cpu.xor8_flag(cpu.registers.a);
    },
    |cpu| {
        /* 0xAF */
        /* XOR A, A */
        cpu.registers.a ^= cpu.registers.a;
        cpu.xor8_flag(cpu.registers.a);
    },
    |cpu| {
        /* 0xB0 */
        /* OR A, B */
        cpu.registers.a |= cpu.registers.b;
        cpu.xor8_flag(cpu.registers.a);
    },
    |cpu| {
        /* 0xB1 */
        /* OR A, C */
        cpu.registers.a |= cpu.registers.c;
        cpu.xor8_flag(cpu.registers.a);
    },
    |cpu| {
        /* 0xB2 */
        /* OR A, D */
        cpu.registers.a |= cpu.registers.d;
        cpu.xor8_flag(cpu.registers.a);
    },
    |cpu| {
        /* 0xB3 */
        /* OR A, E */
        cpu.registers.a |= cpu.registers.e;
        cpu.xor8_flag(cpu.registers.a);
    },
    |cpu| {
        /* 0xB4 */
        /* OR A, H */
        cpu.registers.a |= cpu.registers.h;
        cpu.xor8_flag(cpu.registers.a);
    },
    |cpu| {
        /* 0xB5 */
        /* OR A, L */
        cpu.registers.a |= cpu.registers.l;
        cpu.xor8_flag(cpu.registers.a);
    },
    |cpu| {
        /* 0xB6 */
        /* OR A, [HL] */
        let value = cpu.read_byte(cpu.get_16bit_register(Registers16Bit::HL));

        cpu.registers.a |= value;
        cpu.xor8_flag(cpu.registers.a);
    },
    |cpu| {
        /* 0xB7 */
        /* OR A, A */
        cpu.registers.a |= cpu.registers.a;
        cpu.xor8_flag(cpu.registers.a);
    },
    |cpu| {
        /* 0xB8 */
        /* CP A, B */
        cpu.cp8_flag(cpu.registers.b);
    },
    |cpu| {
        /* 0xB9 */
        /* CP A, C */
        cpu.cp8_flag(cpu.registers.c);
    },
    |cpu| {
        /* 0xBA */
        /* CP A, D */
        cpu.cp8_flag(cpu.registers.d);
    },
    |cpu| {
        /* 0xBB */
        /* CP A, E */
        cpu.cp8_flag(cpu.registers.e);
    },
    |cpu| {
        /* 0xBC */
        /* CP A, H */
        cpu.cp8_flag(cpu.registers.h);
    },
    |cpu| {
        /* 0xBD */
        /* CP A, L */
        cpu.cp8_flag(cpu.registers.l);
    },
    |cpu| {
        /* 0xBE */
        /* CP A, [HL] */
        let value = cpu.read_byte(cpu.get_16bit_register(Registers16Bit::HL));

        cpu.cp8_flag(value);
    },
    |cpu| {
        /* 0xBF */
        /* CP A, A */
        cpu.cp8_flag(cpu.registers.a);
    },
    |cpu| {
        /* 0xC0 */
        /* RET NZ */
        if cpu.zero() != 0x01 {
            let pc_lo = cpu.read_byte(cpu.registers.sp);
            let pc_hi = cpu.read_byte(cpu.registers.sp + 1);

            cpu.registers.sp += 2;
            cpu.registers.pc = (pc_hi as u16) << 8 | pc_lo as u16;
            cpu.mcycles += 1;
        }
        cpu.mcycles += 1;
    },
    |cpu| {
        /* 0xC1 */
        /* POP BC */
        let value_lo = cpu.read_byte(cpu.registers.sp);
        let value_hi = cpu.read_byte(cpu.registers.sp + 1);

        cpu.registers.sp += 2;
        cpu.set_16bit_register(Registers16Bit::BC, (value_hi as u16) << 8 | value_lo as u16);
    },
    |cpu| {
        /* 0xC2 */
        /* JP NZ, a16 */
        let value = cpu.fetch_word();
        if cpu.zero() != 0x01 {
            cpu.registers.pc = value;
            cpu.mcycles += 1;
        }
    },
    |cpu| {
        /* 0xC3 */
        /* JP a16 */
        let value = cpu.fetch_word();

        cpu.registers.pc = value;
        cpu.mcycles += 1;
    },
    |cpu| {
        /* 0xC4 */
        /* CALL NZ, a16 */
        let value = cpu.fetch_word();
        if cpu.zero() != 0x01 {
            cpu.write_word(cpu.registers.sp - 2, cpu.registers.sp - 1, cpu.registers.pc);
            cpu.registers.pc = value;
            cpu.registers.sp -= 2;
            cpu.mcycles += 1;
        }
    },
    |cpu| {
        /* 0xC5 */
        /* PUSH BC */
        cpu.write_word(
            cpu.registers.sp - 2,
            cpu.registers.sp - 1,
            cpu.get_16bit_register(Registers16Bit::BC),
        );
        cpu.registers.sp -= 2;
        cpu.mcycles += 1;
    },
    |cpu| {
        /* 0xC6 */
        /* ADD A, n8 */
        let original_value = cpu.registers.a;
        let data = cpu.fetch_byte();

        cpu.registers.a = cpu.registers.a.wrapping_add(data);
        cpu.add8_flag(original_value, cpu.registers.a);
    },
    |cpu| {
        /* 0xC7 */
        /* RST $00 */
        cpu.write_word(cpu.registers.sp - 2, cpu.registers.sp - 1, cpu.registers.pc);
        cpu.registers.sp -= 2;
        cpu.registers.pc = 0x0000;
        cpu.mcycles += 1;
    },
    |cpu| {
        /* 0xC8 */
        /* RET Z */
        if cpu.zero() == 0x01 {
            let pc_lo = cpu.read_byte(cpu.registers.sp);
            let pc_hi = cpu.read_byte(cpu.registers.sp + 1);

            cpu.registers.sp += 2;
            cpu.registers.pc = (pc_hi as u16) << 8 | pc_lo as u16;
            cpu.mcycles += 1;
        }
        cpu.mcycles += 1;
    },
    |cpu| {
        /* 0xC9 */
        /* RET */
        let pc_lo = cpu.read_byte(cpu.registers.sp);
        let pc_hi = cpu.read_byte(cpu.registers.sp + 1);

        cpu.registers.sp += 2;
        cpu.registers.pc = (pc_hi as u16) << 8 | pc_lo as u16;
        cpu.mcycles += 1;
    },
    |cpu| {
        /* 0xCA */
        /* JP Z, a16 */
        let value = cpu.fetch_word();
        if cpu.zero() == 0x01 {
            cpu.registers.pc = value;
            cpu.mcycles += 1;
        }
    },
    |_| {
        /* 0xCB */
        /* PREFIX */
        unimplemented!();
    },
    |cpu| {
        /* 0xCC */
        /* CALL Z, a16 */
        let value = cpu.fetch_word();
        if cpu.zero() == 0x01 {
            cpu.write_word(cpu.registers.sp - 2, cpu.registers.sp - 1, cpu.registers.pc);
            cpu.registers.pc = value;
            cpu.registers.sp -= 2;
            cpu.mcycles += 1;
        }
    },
    |cpu| {
        /* 0xCD */
        /* CALL a16 */
        let value = cpu.fetch_word();
        cpu.write_word(cpu.registers.sp - 2, cpu.registers.sp - 1, cpu.registers.pc);
        cpu.registers.pc = value;
        cpu.registers.sp -= 2;
        cpu.mcycles += 1;
    },
    |cpu| {
        /* 0xCE */
        /* ADC A, n8 */
        let original_value = cpu.registers.a;
        let data = cpu.fetch_byte();

        cpu.registers.a = cpu.registers.a.wrapping_add(data).wrapping_add(cpu.carry());
        cpu.add8_flag(original_value, cpu.registers.a);
    },
    |cpu| {
        /* 0xCF */
        /* RST $08 */
        cpu.write_word(cpu.registers.sp - 2, cpu.registers.sp - 1, cpu.registers.pc);
        cpu.registers.sp -= 2;
        cpu.registers.pc = 0x0008;
        cpu.mcycles += 1;
    },
    |cpu| {
        /* 0xD0 */
        /* RET NC */
        if cpu.carry() != 0x01 {
            let pc_lo = cpu.read_byte(cpu.registers.sp);
            let pc_hi = cpu.read_byte(cpu.registers.sp + 1);

            cpu.registers.sp += 2;
            cpu.registers.pc = (pc_hi as u16) << 8 | pc_lo as u16;
            cpu.mcycles += 1;
        }
        cpu.mcycles += 1;
    },
    |cpu| {
        /* 0xD1 */
        /* POP DE */
        let value_lo = cpu.read_byte(cpu.registers.sp);
        let value_hi = cpu.read_byte(cpu.registers.sp + 1);

        cpu.registers.sp += 2;
        cpu.set_16bit_register(Registers16Bit::DE, (value_hi as u16) << 8 | value_lo as u16);
    },
    |cpu| {
        /* 0xD2 */
        /* JP NC, a16 */
        let value = cpu.fetch_word();
        if cpu.carry() != 0x01 {
            cpu.registers.pc = value;
            cpu.mcycles += 1;
        }
    },
    |_| {
        /* 0xD3 */
        /* - */
    },
    |cpu| {
        /* 0xD4 */
        /* CALL NC, a16 */
        let value = cpu.fetch_word();
        if cpu.carry() != 0x01 {
            cpu.write_word(cpu.registers.sp - 2, cpu.registers.sp - 1, cpu.registers.pc);
            cpu.registers.pc = value;
            cpu.registers.sp -= 2;
            cpu.mcycles += 1;
        }
    },
    |cpu| {
        /* 0xD5 */
        /* PUSH DE */
        cpu.write_word(
            cpu.registers.sp - 2,
            cpu.registers.sp - 1,
            cpu.get_16bit_register(Registers16Bit::DE),
        );
        cpu.registers.sp -= 2;
        cpu.mcycles += 1;
    },
    |cpu| {
        /* 0xD6 */
        /* SUB A, n8 */
        let value = cpu.fetch_byte();

        cpu.subtract8_flag(cpu.registers.a, value);
        cpu.registers.a = cpu.registers.a.wrapping_sub(cpu.registers.a);
    },
    |cpu| {
        /* 0xD7 */
        /* RST $10 */
        cpu.write_word(cpu.registers.sp - 2, cpu.registers.sp - 1, cpu.registers.pc);
        cpu.registers.sp -= 2;
        cpu.registers.pc = 0x0010;
        cpu.mcycles += 1;
    },
    |cpu| {
        /* 0xD8 */
        /* RET C */
        if cpu.carry() == 0x01 {
            let pc_lo = cpu.read_byte(cpu.registers.sp);
            let pc_hi = cpu.read_byte(cpu.registers.sp + 1);

            cpu.registers.sp += 2;
            cpu.registers.pc = (pc_hi as u16) << 8 | pc_lo as u16;
            cpu.mcycles += 1;
        }
    },
    |cpu| {
        /* 0xD9 */
        /* RETI */
        let pc_lo = cpu.read_byte(cpu.registers.sp);
        let pc_hi = cpu.read_byte(cpu.registers.sp + 1);

        cpu.registers.sp += 2;
        cpu.registers.pc = (pc_hi as u16) << 8 | pc_lo as u16;
        cpu.mcycles += 1;
    },
    |cpu| {
        /* 0xDA */
        /* JP C, a16 */
        let value = cpu.fetch_word();
        if cpu.carry() == 0x01 {
            cpu.registers.pc = value;
            cpu.mcycles += 1;
        }
    },
    |_| {
        /* 0xDB */
        /* - */
    },
    |cpu| {
        /* 0xDC */
        /* CALL C, a16 */
        let value = cpu.fetch_word();
        if cpu.carry() == 0x01 {
            cpu.write_word(cpu.registers.sp - 2, cpu.registers.sp - 1, cpu.registers.pc);
            cpu.registers.pc = value;
            cpu.registers.sp -= 2;
            cpu.mcycles += 1;
        }
    },
    |_| {
        /* 0xDD */
        /* - */
    },
    |cpu| {
        /* 0xDE */
        /* SBC A, n8 */
        let value = cpu.fetch_byte();

        cpu.subtract8_flag(cpu.registers.a, value - cpu.carry());
        cpu.registers.a = cpu.registers.a.wrapping_sub(value);
    },
    |cpu| {
        /* 0xDF */
        /* RST $18 */
        cpu.write_word(cpu.registers.sp - 2, cpu.registers.sp - 1, cpu.registers.pc);
        cpu.registers.sp -= 2;
        cpu.registers.pc = 0x0018;
        cpu.mcycles += 1;
    },
    |cpu| {
        /* 0xE0 */
        /* LDH [a8], A */
        let value = cpu.fetch_byte();

        cpu.write_byte(0xFF00 | value as u16, cpu.registers.a);
    },
    |cpu| {
        /* 0xE1 */
        /* POP HL */
        let value_lo = cpu.read_byte(cpu.registers.sp);
        let value_hi = cpu.read_byte(cpu.registers.sp + 1);

        cpu.registers.sp += 2;
        cpu.set_16bit_register(Registers16Bit::HL, (value_hi as u16) << 8 | value_lo as u16);
    },
    |cpu| {
        /* 0xE2 */
        /* LDH [C], A */
        cpu.write_byte(0xFF00 | cpu.registers.c as u16, cpu.registers.a);
    },
    |_| {
        /* 0xE3 */
        /* - */
    },
    |_| {
        /* 0xE4 */
        /* - */
    },
    |cpu| {
        /* 0xE5 */
        /* PUSH HL */
        cpu.write_word(
            cpu.registers.sp - 2,
            cpu.registers.sp - 1,
            cpu.get_16bit_register(Registers16Bit::HL),
        );
        cpu.registers.sp -= 2;
        cpu.mcycles += 1;
    },
    |cpu| {
        /* 0xE6 */
        /* AND A, n8 */
        let value = cpu.fetch_byte();

        cpu.registers.a &= value;
        cpu.and8_flag(cpu.registers.a);
    },
    |cpu| {
        /* 0xE7 */
        /* RST $20 */
        cpu.write_word(cpu.registers.sp - 2, cpu.registers.sp - 1, cpu.registers.pc);
        cpu.registers.sp -= 2;
        cpu.registers.pc = 0x0020;
        cpu.mcycles += 1;
    },
    |cpu| {
        /* 0xE8 */
        /* ADD SP, e8 */
        let original_value = cpu.registers.sp;
        let data = cpu.fetch_byte() as i8 as i16;

        cpu.registers.sp = cpu.registers.sp.wrapping_add_signed(data);
        cpu.add16_flag(original_value, cpu.registers.sp);
        cpu.set_zero(false);
    },
    |cpu| {
        /* 0xE9 */
        /* JP HL */
        cpu.registers.pc = cpu.get_16bit_register(Registers16Bit::HL);
    },
    |cpu| {
        /* 0xEA */
        /* LD [a16], A */
        let addr = cpu.fetch_word();
        cpu.write_byte(addr, cpu.registers.a);
    },
    |_| {
        /* 0xEB */
        /* - */
    },
    |_| {
        /* 0xEC */
        /* - */
    },
    |_| {
        /* 0xED */
        /* - */
    },
    |cpu| {
        /* 0xEE */
        /* XOR A, n8 */
        let value = cpu.fetch_byte();

        cpu.registers.a ^= value;
        cpu.xor8_flag(cpu.registers.a);
    },
    |cpu| {
        /* 0xEF */
        /* RST $28 */
        cpu.write_word(cpu.registers.sp - 2, cpu.registers.sp - 1, cpu.registers.pc);
        cpu.registers.sp -= 2;
        cpu.registers.pc = 0x0028;
        cpu.mcycles += 1;
    },
    |cpu| {
        /* 0xF0 */
        /* LDH A, [a8] */
        let value = cpu.fetch_byte();
        let value = cpu.read_byte(0xFF00 | value as u16);

        cpu.registers.a = value;
    },
    |cpu| {
        /* 0xF1 */
        /* POP AF */
        let value_lo = cpu.read_byte(cpu.registers.sp);
        let value_hi = cpu.read_byte(cpu.registers.sp + 1);

        cpu.registers.sp += 2;
        cpu.set_16bit_register(Registers16Bit::AF, (value_hi as u16) << 8 | value_lo as u16);
    },
    |cpu| {
        /* 0xF2 */
        /* LDH A, [C] */
        cpu.registers.a = cpu.read_byte(0xFF00 | cpu.registers.c as u16);
    },
    |cpu| {
        /* 0xF3 */
        /* DI */
        cpu.ime = false;
    },
    |_| {
        /* 0xF4 */
        /* - */
    },
    |cpu| {
        /* 0xF5 */
        /* PUSH AF */
        cpu.write_word(
            cpu.registers.sp - 2,
            cpu.registers.sp - 1,
            cpu.get_16bit_register(Registers16Bit::AF),
        );
        cpu.registers.sp -= 2;
        cpu.mcycles += 1;
    },
    |cpu| {
        /* 0xF6 */
        /* OR A, n8 */
        let value = cpu.fetch_byte();

        cpu.registers.a |= value;
        cpu.xor8_flag(cpu.registers.a);
    },
    |cpu| {
        /* 0xF7 */
        /* RST $30 */
        cpu.write_word(cpu.registers.sp - 2, cpu.registers.sp - 1, cpu.registers.pc);
        cpu.registers.sp -= 2;
        cpu.registers.pc = 0x0030;
        cpu.mcycles += 1;
    },
    |cpu| {
        /* 0xF8 */
        /* LD HL, SP + e8 */
        let original_value = cpu.registers.sp;
        let value = cpu.fetch_byte() as i8 as i16;

        cpu.set_16bit_register(
            Registers16Bit::HL,
            cpu.registers.sp.wrapping_add_signed(value),
        );
        cpu.add16_flag(original_value, cpu.get_16bit_register(Registers16Bit::HL));
        cpu.set_zero(false);
    },
    |cpu| {
        /* 0xF9 */
        /* LD SP, HL */
        cpu.registers.sp = cpu.get_16bit_register(Registers16Bit::HL);
    },
    |cpu| {
        /* 0xFA */
        /* LD A, [a16] */
        let addr = cpu.fetch_word();
        let value = cpu.read_byte(addr);

        cpu.registers.a = value;
    },
    |cpu| {
        /* 0xFB */
        /* EI */
        cpu.ime = true;
    },
    |_| {
        /* 0xFC */
        /* - */
    },
    |_| {
        /* 0xFD */
        /* - */
    },
    |cpu| {
        /* 0xFE */
        /* CP A, n8 */
        let value = cpu.fetch_byte();

        cpu.cp8_flag(value);
    },
    |cpu| {
        /* 0xFF */
        /* RST $38 */
        cpu.write_word(cpu.registers.sp - 2, cpu.registers.sp - 1, cpu.registers.pc);
        cpu.registers.sp -= 2;
        cpu.registers.pc = 0x0038;
        cpu.mcycles += 1;
    },
];

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
            ime: false,
        }
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

    fn write_word(&mut self, addr_lo: u16, addr_hi: u16, data: u16) {
        self.write_byte(addr_lo, data as u8);
        self.write_byte(addr_hi, (data >> 8) as u8);
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
}
