use crate::hardware::cpu::{Cpu, Registers16Bit};

pub(crate) static OPCODES: [fn(&mut Cpu); 0x100] = [
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
        cpu.m_cycles += 1;
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
        cpu.rotate8_flag(cpu.registers.a, true, true);
        cpu.set_zero(false);

        cpu.registers.a = cpu.registers.a.rotate_left(1) | cpu.carry();
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
        cpu.add16_flag(original_value, cpu.get_16bit_register(Registers16Bit::BC));

        cpu.m_cycles += 1;
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
        );
        cpu.m_cycles += 1;
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
        cpu.rotate8_flag(cpu.registers.a, false, true);
        cpu.set_zero(false);

        cpu.registers.a = cpu.registers.a >> 1 | cpu.carry() << 7;
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
        cpu.m_cycles += 1;
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
        let carry = cpu.rotate8_flag(cpu.registers.a, true, false);

        cpu.set_zero(false);
        cpu.registers.a = cpu.registers.a << 1 | cpu.carry();
        cpu.set_carry(carry);
    },
    |cpu| {
        /* 0x18 */
        /* JR e8 */
        let value = cpu.fetch_byte() as i8 as i16;
        cpu.registers.pc = cpu.registers.pc.wrapping_add_signed(value);
        cpu.m_cycles += 1;
    },
    |cpu| {
        /* 0x19 */
        /* ADD HL, DE */
        let original_value = cpu.get_16bit_register(Registers16Bit::HL);

        cpu.set_16bit_register(
            Registers16Bit::HL,
            original_value.wrapping_add(cpu.get_16bit_register(Registers16Bit::DE)),
        );
        cpu.add16_flag(original_value, cpu.get_16bit_register(Registers16Bit::DE));

        cpu.m_cycles += 1;
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
        );
        cpu.m_cycles += 1;
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
        let carry = cpu.rotate8_flag(cpu.registers.a, false, false);

        cpu.set_zero(false);
        cpu.registers.a = cpu.registers.a >> 1 | cpu.carry() << 7;
        cpu.set_carry(carry);
    },
    |cpu| {
        /* 0x20 */
        /* JR NZ, e8 */
        let value = cpu.fetch_byte() as i8 as i16;
        if cpu.zero() != 0x01 {
            cpu.registers.pc = cpu.registers.pc.wrapping_add_signed(value);
            cpu.m_cycles += 1;
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
        cpu.m_cycles += 1;
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
            cpu.m_cycles += 1;
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
        cpu.add16_flag(original_value, original_value);

        cpu.m_cycles += 1;
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
        );
        cpu.m_cycles += 1;
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
            cpu.m_cycles += 1;
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
        cpu.m_cycles += 1;
    },
    |cpu| {
        /* 0x34 */
        /* INC [HL] */
        let value = cpu.read_byte(cpu.get_16bit_register(Registers16Bit::HL)).wrapping_add(1);

        cpu.write_byte(cpu.get_16bit_register(Registers16Bit::HL), value);
        cpu.increment8_flag(value);
    },
    |cpu| {
        /* 0x35 */
        /* DEC [HL] */
        let value = cpu.read_byte(cpu.get_16bit_register(Registers16Bit::HL)).wrapping_sub(1);

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
            cpu.m_cycles += 1;
        }
    },
    |cpu| {
        /* 0x39 */
        /* ADD HL, SP */
        let original_value = cpu.get_16bit_register(Registers16Bit::HL);

        cpu.set_16bit_register(Registers16Bit::HL, original_value.wrapping_add(cpu.registers.sp));
        cpu.add16_flag(original_value, cpu.registers.sp);

        cpu.m_cycles += 1;
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
        cpu.m_cycles += 1;
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
        cpu.set_carry(cpu.carry() == 0x00);
        cpu.set_half_carry(false);
        cpu.set_negative(false);
    },
    |_| {
        /* 0x40 */
        /* LD B, B */
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
    |_| {
        /* 0x49 */
        /* LD C, C */
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
    |_| {
        /* 0x52 */
        /* LD D, D */
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
    |_| {
        /* 0x5B */
        /* LD E, E */
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
    |_| {
        /* 0x64 */
        /* LD H, H */
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
    |_| {
        /* 0x6D */
        /* LD L, L */
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
    |cpu| {
        /* 0x76 */
        /* HALT */
        let bus = cpu.bus.read().unwrap();
        let mut ite = bus.interrupt_enable.read().unwrap().value();
        let mut itf = bus.interrupt_flags.read().unwrap().value();
        while (ite & itf) == 0x00 {
            bus.tick(1);
            cpu.m_cycles += 1;

            ite = bus.interrupt_enable.read().unwrap().value();
            itf = bus.interrupt_flags.read().unwrap().value();
        }
        drop(bus);

        if cpu.ime {
            cpu.check_interrupts();
        } else {
            unimplemented!()
        }
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
    |_| {
        /* 0x7F */
        /* LD A, A */
    },
    |cpu| {
        /* 0x80 */
        /* ADD A, B */
        let original_value = cpu.registers.a;

        cpu.registers.a = cpu.registers.a.wrapping_add(cpu.registers.b);
        cpu.add8_flag(original_value, cpu.registers.b);
    },
    |cpu| {
        /* 0x81 */
        /* ADD A, C */
        let original_value = cpu.registers.a;

        cpu.registers.a = cpu.registers.a.wrapping_add(cpu.registers.c);
        cpu.add8_flag(original_value, cpu.registers.c);
    },
    |cpu| {
        /* 0x82 */
        /* ADD A, D */
        let original_value = cpu.registers.a;

        cpu.registers.a = cpu.registers.a.wrapping_add(cpu.registers.d);
        cpu.add8_flag(original_value, cpu.registers.d);
    },
    |cpu| {
        /* 0x83 */
        /* ADD A, E */
        let original_value = cpu.registers.a;

        cpu.registers.a = cpu.registers.a.wrapping_add(cpu.registers.e);
        cpu.add8_flag(original_value, cpu.registers.e);
    },
    |cpu| {
        /* 0x84 */
        /* ADD A, H */
        let original_value = cpu.registers.a;

        cpu.registers.a = cpu.registers.a.wrapping_add(cpu.registers.h);
        cpu.add8_flag(original_value, cpu.registers.h);
    },
    |cpu| {
        /* 0x85 */
        /* ADD A, L */
        let original_value = cpu.registers.a;

        cpu.registers.a = cpu.registers.a.wrapping_add(cpu.registers.l);
        cpu.add8_flag(original_value, cpu.registers.l);
    },
    |cpu| {
        /* 0x86 */
        /* ADD A, [HL] */
        let original_value = cpu.registers.a;
        let data = cpu.read_byte(cpu.get_16bit_register(Registers16Bit::HL));

        cpu.registers.a = cpu.registers.a.wrapping_add(data);
        cpu.add8_flag(original_value, data);
    },
    |cpu| {
        /* 0x87 */
        /* ADD A, A */
        let original_value = cpu.registers.a;

        cpu.registers.a = cpu.registers.a.wrapping_add(cpu.registers.a);
        cpu.add8_flag(original_value, original_value);
    },
    |cpu| {
        /* 0x88 */
        /* ADC A, B */
        let original_value = cpu.registers.a;

        cpu.registers.a = cpu.registers.a.wrapping_add(cpu.registers.b).wrapping_add(cpu.carry());
        cpu.adc8_flag(original_value, cpu.registers.b);
    },
    |cpu| {
        /* 0x89 */
        /* ADC A, C */
        let original_value = cpu.registers.a;

        cpu.registers.a = cpu.registers.a.wrapping_add(cpu.registers.c).wrapping_add(cpu.carry());
        cpu.adc8_flag(original_value, cpu.registers.c);
    },
    |cpu| {
        /* 0x8A */
        /* ADC A, D */
        let original_value = cpu.registers.a;

        cpu.registers.a = cpu.registers.a.wrapping_add(cpu.registers.d).wrapping_add(cpu.carry());
        cpu.adc8_flag(original_value, cpu.registers.d);
    },
    |cpu| {
        /* 0x8B */
        /* ADC A, E */
        let original_value = cpu.registers.a;

        cpu.registers.a = cpu.registers.a.wrapping_add(cpu.registers.e).wrapping_add(cpu.carry());
        cpu.adc8_flag(original_value, cpu.registers.e);
    },
    |cpu| {
        /* 0x8C */
        /* ADC A, H */
        let original_value = cpu.registers.a;

        cpu.registers.a = cpu.registers.a.wrapping_add(cpu.registers.h).wrapping_add(cpu.carry());
        cpu.adc8_flag(original_value, cpu.registers.h);
    },
    |cpu| {
        /* 0x8D */
        /* ADC A, L */
        let original_value = cpu.registers.a;

        cpu.registers.a = cpu.registers.a.wrapping_add(cpu.registers.l).wrapping_add(cpu.carry());
        cpu.adc8_flag(original_value, cpu.registers.l);
    },
    |cpu| {
        /* 0x8E */
        /* ADC A, [HL] */
        let original_value = cpu.registers.a;
        let data = cpu.read_byte(cpu.get_16bit_register(Registers16Bit::HL));

        cpu.registers.a = cpu.registers.a.wrapping_add(data).wrapping_add(cpu.carry());
        cpu.adc8_flag(original_value, data);
    },
    |cpu| {
        /* 0x8F */
        /* ADC A, A */
        let original_value = cpu.registers.a;

        cpu.registers.a = cpu.registers.a.wrapping_add(cpu.registers.a).wrapping_add(cpu.carry());
        cpu.adc8_flag(original_value, original_value);
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
        let old_carry = cpu.carry();

        cpu.subtractc8_flag(cpu.registers.a, cpu.registers.b);
        cpu.registers.a = cpu.registers.a.wrapping_sub(cpu.registers.b).wrapping_sub(old_carry);
    },
    |cpu| {
        /* 0x99 */
        /* SBC A, C */
        let old_carry = cpu.carry();

        cpu.subtractc8_flag(cpu.registers.a, cpu.registers.c);
        cpu.registers.a = cpu.registers.a.wrapping_sub(cpu.registers.c).wrapping_sub(old_carry);
    },
    |cpu| {
        /* 0x9A */
        /* SBC A, D */
        let old_carry = cpu.carry();

        cpu.subtractc8_flag(cpu.registers.a, cpu.registers.d);
        cpu.registers.a = cpu.registers.a.wrapping_sub(cpu.registers.d).wrapping_sub(old_carry);
    },
    |cpu| {
        /* 0x9B */
        /* SBC A, E */
        let old_carry = cpu.carry();

        cpu.subtractc8_flag(cpu.registers.a, cpu.registers.e);
        cpu.registers.a = cpu.registers.a.wrapping_sub(cpu.registers.e).wrapping_sub(old_carry);
    },
    |cpu| {
        /* 0x9C */
        /* SBC A, H */
        let old_carry = cpu.carry();

        cpu.subtractc8_flag(cpu.registers.a, cpu.registers.h);
        cpu.registers.a = cpu.registers.a.wrapping_sub(cpu.registers.h).wrapping_sub(old_carry);
    },
    |cpu| {
        /* 0x9D */
        /* SBC A, L */
        let old_carry = cpu.carry();

        cpu.subtractc8_flag(cpu.registers.a, cpu.registers.l);
        cpu.registers.a = cpu.registers.a.wrapping_sub(cpu.registers.l).wrapping_sub(old_carry);
    },
    |cpu| {
        /* 0x9E */
        /* SBC A, [HL] */
        let old_carry = cpu.carry();
        let data = cpu.read_byte(cpu.get_16bit_register(Registers16Bit::HL));

        cpu.subtractc8_flag(cpu.registers.a, data);
        cpu.registers.a = cpu.registers.a.wrapping_sub(data).wrapping_sub(old_carry);
    },
    |cpu| {
        /* 0x9F */
        /* SBC A, A */
        let old_carry = cpu.carry();

        cpu.subtractc8_flag(cpu.registers.a, cpu.registers.a);
        cpu.registers.a = cpu.registers.a.wrapping_sub(cpu.registers.a).wrapping_sub(old_carry);
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
            cpu.m_cycles += 1;
        }
        cpu.m_cycles += 1;
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
            cpu.m_cycles += 1;
        }
    },
    |cpu| {
        /* 0xC3 */
        /* JP a16 */
        let value = cpu.fetch_word();

        cpu.registers.pc = value;
        cpu.m_cycles += 1;
    },
    |cpu| {
        /* 0xC4 */
        /* CALL NZ, a16 */
        let value = cpu.fetch_word();
        if cpu.zero() != 0x01 {
            cpu.write_word(cpu.registers.sp - 2, cpu.registers.sp - 1, cpu.registers.pc);
            cpu.registers.pc = value;
            cpu.registers.sp -= 2;
            cpu.m_cycles += 1;
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
        cpu.m_cycles += 1;
    },
    |cpu| {
        /* 0xC6 */
        /* ADD A, n8 */
        let original_value = cpu.registers.a;
        let data = cpu.fetch_byte();

        cpu.registers.a = cpu.registers.a.wrapping_add(data);
        cpu.add8_flag(original_value, data);
    },
    |cpu| {
        /* 0xC7 */
        /* RST $00 */
        cpu.write_word(cpu.registers.sp - 2, cpu.registers.sp - 1, cpu.registers.pc);
        cpu.registers.sp -= 2;
        cpu.registers.pc = 0x0000;
        cpu.m_cycles += 1;
    },
    |cpu| {
        /* 0xC8 */
        /* RET Z */
        if cpu.zero() == 0x01 {
            let pc_lo = cpu.read_byte(cpu.registers.sp);
            let pc_hi = cpu.read_byte(cpu.registers.sp + 1);

            cpu.registers.sp += 2;
            cpu.registers.pc = (pc_hi as u16) << 8 | pc_lo as u16;
            cpu.m_cycles += 1;
        }
        cpu.m_cycles += 1;
    },
    |cpu| {
        /* 0xC9 */
        /* RET */
        let pc_lo = cpu.read_byte(cpu.registers.sp);
        let pc_hi = cpu.read_byte(cpu.registers.sp + 1);

        cpu.registers.sp += 2;
        cpu.registers.pc = (pc_hi as u16) << 8 | pc_lo as u16;
        cpu.m_cycles += 1;
    },
    |cpu| {
        /* 0xCA */
        /* JP Z, a16 */
        let value = cpu.fetch_word();
        if cpu.zero() == 0x01 {
            cpu.registers.pc = value;
            cpu.m_cycles += 1;
        }
    },
    |cpu| {
        /* 0xCB */
        /* PREFIX */
        let opcode = cpu.fetch_byte();

        PREFIXED_OPCODES[opcode as usize](cpu);
    },
    |cpu| {
        /* 0xCC */
        /* CALL Z, a16 */
        let value = cpu.fetch_word();
        if cpu.zero() == 0x01 {
            cpu.write_word(cpu.registers.sp - 2, cpu.registers.sp - 1, cpu.registers.pc);
            cpu.registers.pc = value;
            cpu.registers.sp -= 2;
            cpu.m_cycles += 1;
        }
    },
    |cpu| {
        /* 0xCD */
        /* CALL a16 */
        let value = cpu.fetch_word();
        cpu.write_word(cpu.registers.sp - 2, cpu.registers.sp - 1, cpu.registers.pc);
        cpu.registers.pc = value;
        cpu.registers.sp -= 2;
        cpu.m_cycles += 1;
    },
    |cpu| {
        /* 0xCE */
        /* ADC A, n8 */
        let original_value = cpu.registers.a;
        let data = cpu.fetch_byte();

        cpu.registers.a = cpu.registers.a.wrapping_add(data).wrapping_add(cpu.carry());
        cpu.adc8_flag(original_value, data);
    },
    |cpu| {
        /* 0xCF */
        /* RST $08 */
        cpu.write_word(cpu.registers.sp - 2, cpu.registers.sp - 1, cpu.registers.pc);
        cpu.registers.sp -= 2;
        cpu.registers.pc = 0x0008;
        cpu.m_cycles += 1;
    },
    |cpu| {
        /* 0xD0 */
        /* RET NC */
        if cpu.carry() != 0x01 {
            let pc_lo = cpu.read_byte(cpu.registers.sp);
            let pc_hi = cpu.read_byte(cpu.registers.sp + 1);

            cpu.registers.sp += 2;
            cpu.registers.pc = (pc_hi as u16) << 8 | pc_lo as u16;
            cpu.m_cycles += 1;
        }
        cpu.m_cycles += 1;
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
            cpu.m_cycles += 1;
        }
    },
    |_| { /* 0xD3 */ },
    |cpu| {
        /* 0xD4 */
        /* CALL NC, a16 */
        let value = cpu.fetch_word();
        if cpu.carry() != 0x01 {
            cpu.write_word(cpu.registers.sp - 2, cpu.registers.sp - 1, cpu.registers.pc);
            cpu.registers.pc = value;
            cpu.registers.sp -= 2;
            cpu.m_cycles += 1;
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
        cpu.m_cycles += 1;
    },
    |cpu| {
        /* 0xD6 */
        /* SUB A, n8 */
        let value = cpu.fetch_byte();

        cpu.subtract8_flag(cpu.registers.a, value);
        cpu.registers.a = cpu.registers.a.wrapping_sub(value);
    },
    |cpu| {
        /* 0xD7 */
        /* RST $10 */
        cpu.write_word(cpu.registers.sp - 2, cpu.registers.sp - 1, cpu.registers.pc);
        cpu.registers.sp -= 2;
        cpu.registers.pc = 0x0010;
        cpu.m_cycles += 1;
    },
    |cpu| {
        /* 0xD8 */
        /* RET C */
        if cpu.carry() == 0x01 {
            let pc_lo = cpu.read_byte(cpu.registers.sp);
            let pc_hi = cpu.read_byte(cpu.registers.sp + 1);

            cpu.registers.sp += 2;
            cpu.registers.pc = (pc_hi as u16) << 8 | pc_lo as u16;
            cpu.m_cycles += 1;
        }
        cpu.m_cycles += 1;
    },
    |cpu| {
        /* 0xD9 */
        /* RETI */
        let pc_lo = cpu.read_byte(cpu.registers.sp);
        let pc_hi = cpu.read_byte(cpu.registers.sp + 1);

        cpu.ime = true;
        cpu.registers.sp += 2;
        cpu.registers.pc = (pc_hi as u16) << 8 | pc_lo as u16;
        cpu.m_cycles += 1;
    },
    |cpu| {
        /* 0xDA */
        /* JP C, a16 */
        let value = cpu.fetch_word();
        if cpu.carry() == 0x01 {
            cpu.registers.pc = value;
            cpu.m_cycles += 1;
        }
    },
    |_| { /* 0xDB */ },
    |cpu| {
        /* 0xDC */
        /* CALL C, a16 */
        let value = cpu.fetch_word();
        if cpu.carry() == 0x01 {
            cpu.write_word(cpu.registers.sp - 2, cpu.registers.sp - 1, cpu.registers.pc);
            cpu.registers.pc = value;
            cpu.registers.sp -= 2;
            cpu.m_cycles += 1;
        }
    },
    |_| { /* 0xDD */ },
    |cpu| {
        /* 0xDE */
        /* SBC A, n8 */
        let old_carry = cpu.carry();
        let value = cpu.fetch_byte();

        cpu.subtractc8_flag(cpu.registers.a, value);
        cpu.registers.a = cpu.registers.a.wrapping_sub(value).wrapping_sub(old_carry);
    },
    |cpu| {
        /* 0xDF */
        /* RST $18 */
        cpu.write_word(cpu.registers.sp - 2, cpu.registers.sp - 1, cpu.registers.pc);
        cpu.registers.sp -= 2;
        cpu.registers.pc = 0x0018;
        cpu.m_cycles += 1;
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
    |_| { /* 0xE3 */ },
    |_| { /* 0xE4 */ },
    |cpu| {
        /* 0xE5 */
        /* PUSH HL */
        cpu.write_word(
            cpu.registers.sp - 2,
            cpu.registers.sp - 1,
            cpu.get_16bit_register(Registers16Bit::HL),
        );
        cpu.registers.sp -= 2;
        cpu.m_cycles += 1;
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
        cpu.m_cycles += 1;
    },
    |cpu| {
        /* 0xE8 */
        /* ADD SP, e8 */
        let original_value = cpu.registers.sp;
        let data = cpu.fetch_byte() as i8 as i16;

        cpu.registers.sp = cpu.registers.sp.wrapping_add_signed(data);
        cpu.add_signed16_flag(original_value, data);
        cpu.m_cycles += 2;
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
    |_| { /* 0xEB */ },
    |_| { /* 0xEC */ },
    |_| { /* 0xED */ },
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
        cpu.m_cycles += 1;
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
    |_| { /* 0xF4 */ },
    |cpu| {
        /* 0xF5 */
        /* PUSH AF */
        cpu.write_word(
            cpu.registers.sp - 2,
            cpu.registers.sp - 1,
            cpu.get_16bit_register(Registers16Bit::AF),
        );
        cpu.registers.sp -= 2;
        cpu.m_cycles += 1;
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
        cpu.m_cycles += 1;
    },
    |cpu| {
        /* 0xF8 */
        /* LD HL, SP + e8 */
        let original_value = cpu.registers.sp;
        let value = cpu.fetch_byte() as i8 as i16;

        cpu.set_16bit_register(Registers16Bit::HL, cpu.registers.sp.wrapping_add_signed(value));
        cpu.add_signed16_flag(original_value, value);
        cpu.m_cycles += 1;
    },
    |cpu| {
        /* 0xF9 */
        /* LD SP, HL */
        cpu.registers.sp = cpu.get_16bit_register(Registers16Bit::HL);
        cpu.m_cycles += 1;
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
    |_| { /* 0xFC */ },
    |_| { /* 0xFD */ },
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
        cpu.m_cycles += 1;
    },
];

pub(crate) static PREFIXED_OPCODES: [fn(&mut Cpu); 0x100] = [
    |cpu| {
        /* 0x00 */
        /* RLC B */
        cpu.rotate8_flag(cpu.registers.b, true, true);
        cpu.registers.b = cpu.registers.b.rotate_left(1) | cpu.carry();
    },
    |cpu| {
        /* 0x01 */
        /* RLC C */
        cpu.rotate8_flag(cpu.registers.c, true, true);
        cpu.registers.c = cpu.registers.c.rotate_left(1) | cpu.carry();
    },
    |cpu| {
        /* 0x02 */
        /* RLC D */
        cpu.rotate8_flag(cpu.registers.d, true, true);
        cpu.registers.d = cpu.registers.d.rotate_left(1) | cpu.carry();
    },
    |cpu| {
        /* 0x03 */
        /* RLC E */
        cpu.rotate8_flag(cpu.registers.e, true, true);
        cpu.registers.e = cpu.registers.e.rotate_left(1) | cpu.carry();
    },
    |cpu| {
        /* 0x04 */
        /* RLC H */
        cpu.rotate8_flag(cpu.registers.h, true, true);
        cpu.registers.h = cpu.registers.h.rotate_left(1) | cpu.carry();
    },
    |cpu| {
        /* 0x05 */
        /* RLC L */
        cpu.rotate8_flag(cpu.registers.l, true, true);
        cpu.registers.l = cpu.registers.l.rotate_left(1) | cpu.carry();
    },
    |cpu| {
        /* 0x06 */
        /* RLC [HL] */
        let mut value = cpu.read_byte(cpu.get_16bit_register(Registers16Bit::HL));

        cpu.rotate8_flag(value, true, true);
        value = value.rotate_left(1) | cpu.carry();
        cpu.write_byte(cpu.get_16bit_register(Registers16Bit::HL), value);
    },
    |cpu| {
        /* 0x07 */
        /* RLC A */
        cpu.rotate8_flag(cpu.registers.a, true, true);
        cpu.registers.a = cpu.registers.a.rotate_left(1) | cpu.carry();
    },
    |cpu| {
        /* 0x08 */
        /* RRC B */
        cpu.rotate8_flag(cpu.registers.b, false, true);
        cpu.registers.b = cpu.registers.b >> 1 | cpu.carry() << 7;
    },
    |cpu| {
        /* 0x09 */
        /* RRC C */
        cpu.rotate8_flag(cpu.registers.c, false, true);
        cpu.registers.c = cpu.registers.c >> 1 | cpu.carry() << 7;
    },
    |cpu| {
        /* 0x0A */
        /* RRC D */
        cpu.rotate8_flag(cpu.registers.d, false, true);
        cpu.registers.d = cpu.registers.d >> 1 | cpu.carry() << 7;
    },
    |cpu| {
        /* 0x0B */
        /* RRC E */
        cpu.rotate8_flag(cpu.registers.e, false, true);
        cpu.registers.e = cpu.registers.e >> 1 | cpu.carry() << 7;
    },
    |cpu| {
        /* 0x0C */
        /* RRC H */
        cpu.rotate8_flag(cpu.registers.h, false, true);
        cpu.registers.h = cpu.registers.h >> 1 | cpu.carry() << 7;
    },
    |cpu| {
        /* 0x0D */
        /* RRC L */
        cpu.rotate8_flag(cpu.registers.l, false, true);
        cpu.registers.l = cpu.registers.l >> 1 | cpu.carry() << 7;
    },
    |cpu| {
        /* 0x0E */
        /* RRC [HL] */
        let mut value = cpu.read_byte(cpu.get_16bit_register(Registers16Bit::HL));

        cpu.rotate8_flag(value, false, true);
        value = value >> 1 | cpu.carry() << 7;
        cpu.write_byte(cpu.get_16bit_register(Registers16Bit::HL), value);
    },
    |cpu| {
        /* 0x0F */
        /* RRC A */
        cpu.rotate8_flag(cpu.registers.a, false, true);
        cpu.registers.a = cpu.registers.a >> 1 | cpu.carry() << 7;
    },
    |cpu| {
        /* 0x10 */
        /* RL B */
        let carry = cpu.rotate8_flag(cpu.registers.b, true, false);

        cpu.registers.b = cpu.registers.b << 1 | cpu.carry();
        cpu.set_zero(cpu.registers.b == 0x00);
        cpu.set_carry(carry);
    },
    |cpu| {
        /* 0x11 */
        /* RL C */
        let carry = cpu.rotate8_flag(cpu.registers.c, true, false);

        cpu.registers.c = cpu.registers.c << 1 | cpu.carry();
        cpu.set_zero(cpu.registers.c == 0x00);
        cpu.set_carry(carry);
    },
    |cpu| {
        /* 0x12 */
        /* RL D */
        let carry = cpu.rotate8_flag(cpu.registers.d, true, false);

        cpu.registers.d = cpu.registers.d << 1 | cpu.carry();
        cpu.set_zero(cpu.registers.d == 0x00);
        cpu.set_carry(carry);
    },
    |cpu| {
        /* 0x13 */
        /* RL E */
        let carry = cpu.rotate8_flag(cpu.registers.e, true, false);

        cpu.registers.e = cpu.registers.e << 1 | cpu.carry();
        cpu.set_zero(cpu.registers.e == 0x00);
        cpu.set_carry(carry);
    },
    |cpu| {
        /* 0x14 */
        /* RL H */
        let carry = cpu.rotate8_flag(cpu.registers.h, true, false);

        cpu.registers.h = cpu.registers.h << 1 | cpu.carry();
        cpu.set_zero(cpu.registers.h == 0x00);
        cpu.set_carry(carry);
    },
    |cpu| {
        /* 0x15 */
        /* RL L */
        let carry = cpu.rotate8_flag(cpu.registers.l, true, false);

        cpu.registers.l = cpu.registers.l << 1 | cpu.carry();
        cpu.set_zero(cpu.registers.l == 0x00);
        cpu.set_carry(carry);
    },
    |cpu| {
        /* 0x16 */
        /* RL [HL] */
        let mut value = cpu.read_byte(cpu.get_16bit_register(Registers16Bit::HL));
        let carry = cpu.rotate8_flag(value, true, false);

        value = value << 1 | cpu.carry();

        cpu.set_zero(value == 0x00);
        cpu.set_carry(carry);
        cpu.write_byte(cpu.get_16bit_register(Registers16Bit::HL), value);
    },
    |cpu| {
        /* 0x17 */
        /* RL A */
        let carry = cpu.rotate8_flag(cpu.registers.a, true, false);

        cpu.registers.a = cpu.registers.a << 1 | cpu.carry();
        cpu.set_zero(cpu.registers.a == 0x00);
        cpu.set_carry(carry);
    },
    |cpu| {
        /* 0x18 */
        /* RR B */
        let carry = cpu.rotate8_flag(cpu.registers.b, false, false);

        cpu.registers.b = cpu.registers.b >> 1 | cpu.carry() << 7;
        cpu.set_zero(cpu.registers.b == 0x00);
        cpu.set_carry(carry);
    },
    |cpu| {
        /* 0x19 */
        /* RR C */
        let carry = cpu.rotate8_flag(cpu.registers.c, false, false);

        cpu.registers.c = cpu.registers.c >> 1 | cpu.carry() << 7;
        cpu.set_zero(cpu.registers.c == 0x00);
        cpu.set_carry(carry);
    },
    |cpu| {
        /* 0x1A */
        /* RR D */
        let carry = cpu.rotate8_flag(cpu.registers.d, false, false);

        cpu.registers.d = cpu.registers.d >> 1 | cpu.carry() << 7;
        cpu.set_zero(cpu.registers.d == 0x00);
        cpu.set_carry(carry);
    },
    |cpu| {
        /* 0x1B */
        /* RR E */
        let carry = cpu.rotate8_flag(cpu.registers.e, false, false);

        cpu.registers.e = cpu.registers.e >> 1 | cpu.carry() << 7;
        cpu.set_zero(cpu.registers.e == 0x00);
        cpu.set_carry(carry);
    },
    |cpu| {
        /* 0x1C */
        /* RR H */
        let carry = cpu.rotate8_flag(cpu.registers.h, false, false);

        cpu.registers.h = cpu.registers.h >> 1 | cpu.carry() << 7;
        cpu.set_zero(cpu.registers.h == 0x00);
        cpu.set_carry(carry);
    },
    |cpu| {
        /* 0x1D */
        /* RR L */
        let carry = cpu.rotate8_flag(cpu.registers.l, false, false);

        cpu.registers.l = cpu.registers.l >> 1 | cpu.carry() << 7;
        cpu.set_zero(cpu.registers.l == 0x00);
        cpu.set_carry(carry);
    },
    |cpu| {
        /* 0x1E */
        /* RR [HL] */
        let mut value = cpu.read_byte(cpu.get_16bit_register(Registers16Bit::HL));
        let carry = cpu.rotate8_flag(value, false, false);

        value = value >> 1 | cpu.carry() << 7;

        cpu.set_zero(value == 0x00);
        cpu.set_carry(carry);
        cpu.write_byte(cpu.get_16bit_register(Registers16Bit::HL), value);
    },
    |cpu| {
        /* 0x1F */
        /* RR A */
        let carry = cpu.rotate8_flag(cpu.registers.a, false, false);

        cpu.registers.a = cpu.registers.a >> 1 | cpu.carry() << 7;
        cpu.set_zero(cpu.registers.a == 0x00);
        cpu.set_carry(carry);
    },
    |cpu| {
        /* 0x20 */
        /* SLA B */
        cpu.rotate8_flag(cpu.registers.b, true, true);

        cpu.registers.b <<= 1;
        cpu.set_zero(cpu.registers.b == 0x00);
    },
    |cpu| {
        /* 0x21 */
        /* SLA C */
        cpu.rotate8_flag(cpu.registers.c, true, true);

        cpu.registers.c <<= 1;
        cpu.set_zero(cpu.registers.c == 0x00);
    },
    |cpu| {
        /* 0x22 */
        /* SLA D */
        cpu.rotate8_flag(cpu.registers.d, true, true);

        cpu.registers.d <<= 1;
        cpu.set_zero(cpu.registers.d == 0x00);
    },
    |cpu| {
        /* 0x23 */
        /* SLA E */
        cpu.rotate8_flag(cpu.registers.e, true, true);

        cpu.registers.e <<= 1;
        cpu.set_zero(cpu.registers.e == 0x00);
    },
    |cpu| {
        /* 0x24 */
        /* SLA H */
        cpu.rotate8_flag(cpu.registers.h, true, true);

        cpu.registers.h <<= 1;
        cpu.set_zero(cpu.registers.h == 0x00);
    },
    |cpu| {
        /* 0x25 */
        /* SLA L */
        cpu.rotate8_flag(cpu.registers.l, true, true);

        cpu.registers.l <<= 1;
        cpu.set_zero(cpu.registers.l == 0x00);
    },
    |cpu| {
        /* 0x26 */
        /* SLA [HL] */
        let mut value = cpu.read_byte(cpu.get_16bit_register(Registers16Bit::HL));
        cpu.rotate8_flag(value, true, true);

        value <<= 1;

        cpu.set_zero(value == 0x00);
        cpu.write_byte(cpu.get_16bit_register(Registers16Bit::HL), value);
    },
    |cpu| {
        /* 0x27 */
        /* SLA A */
        cpu.rotate8_flag(cpu.registers.a, true, true);

        cpu.registers.a <<= 1;
        cpu.set_zero(cpu.registers.a == 0x00);
    },
    |cpu| {
        /* 0x28 */
        /* SRA B */
        let last_bit = cpu.registers.b & 0x80;
        cpu.rotate8_flag(cpu.registers.b, false, true);

        cpu.registers.b = cpu.registers.b >> 1 | last_bit;
        cpu.set_zero(cpu.registers.b == 0x00);
    },
    |cpu| {
        /* 0x29 */
        /* SRA C */
        let last_bit = cpu.registers.c & 0x80;
        cpu.rotate8_flag(cpu.registers.c, false, true);

        cpu.registers.c = cpu.registers.c >> 1 | last_bit;
        cpu.set_zero(cpu.registers.c == 0x00);
    },
    |cpu| {
        /* 0x2A */
        /* SRA D */
        let last_bit = cpu.registers.d & 0x80;
        cpu.rotate8_flag(cpu.registers.d, false, true);

        cpu.registers.d = cpu.registers.d >> 1 | last_bit;
        cpu.set_zero(cpu.registers.d == 0x00);
    },
    |cpu| {
        /* 0x2B */
        /* SRA E */
        let last_bit = cpu.registers.e & 0x80;
        cpu.rotate8_flag(cpu.registers.e, false, true);

        cpu.registers.e = cpu.registers.e >> 1 | last_bit;
        cpu.set_zero(cpu.registers.e == 0x00);
    },
    |cpu| {
        /* 0x2C */
        /* SRA H */
        let last_bit = cpu.registers.h & 0x80;
        cpu.rotate8_flag(cpu.registers.h, false, true);

        cpu.registers.h = cpu.registers.h >> 1 | last_bit;
        cpu.set_zero(cpu.registers.h == 0x00);
    },
    |cpu| {
        /* 0x2D */
        /* SRA L */
        let last_bit = cpu.registers.l & 0x80;
        cpu.rotate8_flag(cpu.registers.l, false, true);

        cpu.registers.l = cpu.registers.l >> 1 | last_bit;
        cpu.set_zero(cpu.registers.l == 0x00);
    },
    |cpu| {
        /* 0x2E */
        /* SRA [HL] */
        let mut value = cpu.read_byte(cpu.get_16bit_register(Registers16Bit::HL));
        let last_bit = value & 0x80;
        cpu.rotate8_flag(value, false, true);

        value = value >> 1 | last_bit;

        cpu.set_zero(value == 0x00);
        cpu.write_byte(cpu.get_16bit_register(Registers16Bit::HL), value);
    },
    |cpu| {
        /* 0x2F */
        /* SRA A */
        let last_bit = cpu.registers.a & 0x80;
        cpu.rotate8_flag(cpu.registers.a, false, true);

        cpu.registers.a = cpu.registers.a >> 1 | last_bit;
        cpu.set_zero(cpu.registers.a == 0x00);
    },
    |cpu| {
        /* 0x30 */
        /* SWAP B */
        cpu.registers.b = cpu.registers.b.rotate_left(4);
        cpu.set_zero(cpu.registers.b == 0x00);
        cpu.set_carry(false);
        cpu.set_half_carry(false);
        cpu.set_negative(false);
    },
    |cpu| {
        /* 0x31 */
        /* SWAP C */
        cpu.registers.c = cpu.registers.c.rotate_left(4);
        cpu.set_zero(cpu.registers.c == 0x00);
        cpu.set_carry(false);
        cpu.set_half_carry(false);
        cpu.set_negative(false);
    },
    |cpu| {
        /* 0x32 */
        /* SWAP D */
        cpu.registers.d = cpu.registers.d.rotate_left(4);
        cpu.set_zero(cpu.registers.d == 0x00);
        cpu.set_carry(false);
        cpu.set_half_carry(false);
        cpu.set_negative(false);
    },
    |cpu| {
        /* 0x33 */
        /* SWAP E */
        cpu.registers.e = cpu.registers.e.rotate_left(4);
        cpu.set_zero(cpu.registers.e == 0x00);
        cpu.set_carry(false);
        cpu.set_half_carry(false);
        cpu.set_negative(false);
    },
    |cpu| {
        /* 0x34 */
        /* SWAP H */
        cpu.registers.h = cpu.registers.h.rotate_left(4);
        cpu.set_zero(cpu.registers.h == 0x00);
        cpu.set_carry(false);
        cpu.set_half_carry(false);
        cpu.set_negative(false);
    },
    |cpu| {
        /* 0x35 */
        /* SWAP L */
        cpu.registers.l = cpu.registers.l.rotate_left(4);
        cpu.set_zero(cpu.registers.l == 0x00);
        cpu.set_carry(false);
        cpu.set_half_carry(false);
        cpu.set_negative(false);
    },
    |cpu| {
        /* 0x36 */
        /* SWAP [HL] */
        let mut value = cpu.read_byte(cpu.get_16bit_register(Registers16Bit::HL));

        value = value.rotate_left(4);
        cpu.set_zero(value == 0x00);
        cpu.set_carry(false);
        cpu.set_half_carry(false);
        cpu.set_negative(false);
        cpu.write_byte(cpu.get_16bit_register(Registers16Bit::HL), value);
    },
    |cpu| {
        /* 0x37 */
        /* SWAP A */
        cpu.registers.a = cpu.registers.a.rotate_left(4);
        cpu.set_zero(cpu.registers.a == 0x00);
        cpu.set_carry(false);
        cpu.set_half_carry(false);
        cpu.set_negative(false);
    },
    |cpu| {
        /* 0x38 */
        /* SRL B */
        cpu.rotate8_flag(cpu.registers.b, false, true);

        cpu.registers.b >>= 1;
        cpu.set_zero(cpu.registers.b == 0x00);
    },
    |cpu| {
        /* 0x39 */
        /* SRL C */
        cpu.rotate8_flag(cpu.registers.c, false, true);

        cpu.registers.c >>= 1;
        cpu.set_zero(cpu.registers.c == 0x00);
    },
    |cpu| {
        /* 0x3A */
        /* SRL D */
        cpu.rotate8_flag(cpu.registers.d, false, true);

        cpu.registers.d >>= 1;
        cpu.set_zero(cpu.registers.d == 0x00);
    },
    |cpu| {
        /* 0x3B */
        /* SRL E */
        cpu.rotate8_flag(cpu.registers.e, false, true);

        cpu.registers.e >>= 1;
        cpu.set_zero(cpu.registers.e == 0x00);
    },
    |cpu| {
        /* 0x3C */
        /* SRL H */
        cpu.rotate8_flag(cpu.registers.h, false, true);

        cpu.registers.h >>= 1;
        cpu.set_zero(cpu.registers.h == 0x00);
    },
    |cpu| {
        /* 0x3D */
        /* SRL L */
        cpu.rotate8_flag(cpu.registers.l, false, true);

        cpu.registers.l >>= 1;
        cpu.set_zero(cpu.registers.l == 0x00);
    },
    |cpu| {
        /* 0x3E */
        /* SRL [HL] */
        let mut value = cpu.read_byte(cpu.get_16bit_register(Registers16Bit::HL));
        cpu.rotate8_flag(value, false, true);

        value >>= 1;

        cpu.set_zero(value == 0x00);
        cpu.write_byte(cpu.get_16bit_register(Registers16Bit::HL), value);
    },
    |cpu| {
        /* 0x3F */
        /* SRL A */
        cpu.rotate8_flag(cpu.registers.a, false, true);

        cpu.registers.a >>= 1;
        cpu.set_zero(cpu.registers.a == 0x00);
    },
    |cpu| {
        /* 0x40 */
        /* BIT 0, B */
        cpu.bit8_flag(0, cpu.registers.b);
    },
    |cpu| {
        /* 0x41 */
        /* BIT 0, C */
        cpu.bit8_flag(0, cpu.registers.c);
    },
    |cpu| {
        /* 0x42 */
        /* BIT 0, D */
        cpu.bit8_flag(0, cpu.registers.d);
    },
    |cpu| {
        /* 0x43 */
        /* BIT 0, E */
        cpu.bit8_flag(0, cpu.registers.e);
    },
    |cpu| {
        /* 0x44 */
        /* BIT 0, H */
        cpu.bit8_flag(0, cpu.registers.h);
    },
    |cpu| {
        /* 0x45 */
        /* BIT 0, L */
        cpu.bit8_flag(0, cpu.registers.l);
    },
    |cpu| {
        /* 0x46 */
        /* BIT 0, [HL] */
        let value = cpu.read_byte(cpu.get_16bit_register(Registers16Bit::HL));
        cpu.bit8_flag(0, value);
    },
    |cpu| {
        /* 0x47 */
        /* BIT 0, A */
        cpu.bit8_flag(0, cpu.registers.a);
    },
    |cpu| {
        /* 0x48 */
        /* BIT 1, B */
        cpu.bit8_flag(1, cpu.registers.b);
    },
    |cpu| {
        /* 0x49 */
        /* BIT 1, C */
        cpu.bit8_flag(1, cpu.registers.c);
    },
    |cpu| {
        /* 0x4A */
        /* BIT 1, D */
        cpu.bit8_flag(1, cpu.registers.d);
    },
    |cpu| {
        /* 0x4B */
        /* BIT 1, E */
        cpu.bit8_flag(1, cpu.registers.e);
    },
    |cpu| {
        /* 0x4C */
        /* BIT 1, H */
        cpu.bit8_flag(1, cpu.registers.h);
    },
    |cpu| {
        /* 0x4D */
        /* BIT 1, L */
        cpu.bit8_flag(1, cpu.registers.l);
    },
    |cpu| {
        /* 0x4E */
        /* BIT 1, [HL] */
        let value = cpu.read_byte(cpu.get_16bit_register(Registers16Bit::HL));
        cpu.bit8_flag(1, value);
    },
    |cpu| {
        /* 0x4F */
        /* BIT 1, A */
        cpu.bit8_flag(1, cpu.registers.a);
    },
    |cpu| {
        /* 0x50 */
        /* BIT 2, B */
        cpu.bit8_flag(2, cpu.registers.b);
    },
    |cpu| {
        /* 0x51 */
        /* BIT 2, C */
        cpu.bit8_flag(2, cpu.registers.c);
    },
    |cpu| {
        /* 0x52 */
        /* BIT 2, D */
        cpu.bit8_flag(2, cpu.registers.d);
    },
    |cpu| {
        /* 0x53 */
        /* BIT 2, E */
        cpu.bit8_flag(2, cpu.registers.e);
    },
    |cpu| {
        /* 0x54 */
        /* BIT 2, H */
        cpu.bit8_flag(2, cpu.registers.h);
    },
    |cpu| {
        /* 0x55 */
        /* BIT 2, L */
        cpu.bit8_flag(2, cpu.registers.l);
    },
    |cpu| {
        /* 0x56 */
        /* BIT 2, [HL] */
        let value = cpu.read_byte(cpu.get_16bit_register(Registers16Bit::HL));
        cpu.bit8_flag(2, value);
    },
    |cpu| {
        /* 0x57 */
        /* BIT 2, A */
        cpu.bit8_flag(2, cpu.registers.a);
    },
    |cpu| {
        /* 0x58 */
        /* BIT 3, B */
        cpu.bit8_flag(3, cpu.registers.b);
    },
    |cpu| {
        /* 0x59 */
        /* BIT 3, C */
        cpu.bit8_flag(3, cpu.registers.c);
    },
    |cpu| {
        /* 0x5A */
        /* BIT 3, D */
        cpu.bit8_flag(3, cpu.registers.d);
    },
    |cpu| {
        /* 0x5B */
        /* BIT 3, E */
        cpu.bit8_flag(3, cpu.registers.e);
    },
    |cpu| {
        /* 0x5C */
        /* BIT 3, H */
        cpu.bit8_flag(3, cpu.registers.h);
    },
    |cpu| {
        /* 0x5D */
        /* BIT 3, L */
        cpu.bit8_flag(3, cpu.registers.l);
    },
    |cpu| {
        /* 0x5E */
        /* BIT 3, [HL] */
        let value = cpu.read_byte(cpu.get_16bit_register(Registers16Bit::HL));
        cpu.bit8_flag(3, value);
    },
    |cpu| {
        /* 0x5F */
        /* BIT 3, A */
        cpu.bit8_flag(3, cpu.registers.a);
    },
    |cpu| {
        /* 0x60 */
        /* BIT 4, B */
        cpu.bit8_flag(4, cpu.registers.b);
    },
    |cpu| {
        /* 0x61 */
        /* BIT 4, C */
        cpu.bit8_flag(4, cpu.registers.c);
    },
    |cpu| {
        /* 0x62 */
        /* BIT 4, D */
        cpu.bit8_flag(4, cpu.registers.d);
    },
    |cpu| {
        /* 0x63 */
        /* BIT 4, E */
        cpu.bit8_flag(4, cpu.registers.e);
    },
    |cpu| {
        /* 0x64 */
        /* BIT 4, H */
        cpu.bit8_flag(4, cpu.registers.h);
    },
    |cpu| {
        /* 0x65 */
        /* BIT 4, L */
        cpu.bit8_flag(4, cpu.registers.l);
    },
    |cpu| {
        /* 0x66 */
        /* BIT 4, [HL] */
        let value = cpu.read_byte(cpu.get_16bit_register(Registers16Bit::HL));
        cpu.bit8_flag(4, value);
    },
    |cpu| {
        /* 0x67 */
        /* BIT 4, A */
        cpu.bit8_flag(4, cpu.registers.a);
    },
    |cpu| {
        /* 0x68 */
        /* BIT 5, B */
        cpu.bit8_flag(5, cpu.registers.b);
    },
    |cpu| {
        /* 0x69 */
        /* BIT 5, C */
        cpu.bit8_flag(5, cpu.registers.c);
    },
    |cpu| {
        /* 0x6A */
        /* BIT 5, D */
        cpu.bit8_flag(5, cpu.registers.d);
    },
    |cpu| {
        /* 0x6B */
        /* BIT 5, E */
        cpu.bit8_flag(5, cpu.registers.e);
    },
    |cpu| {
        /* 0x6C */
        /* BIT 5, H */
        cpu.bit8_flag(5, cpu.registers.h);
    },
    |cpu| {
        /* 0x6D */
        /* BIT 5, L */
        cpu.bit8_flag(5, cpu.registers.l);
    },
    |cpu| {
        /* 0x6E */
        /* BIT 5, [HL] */
        let value = cpu.read_byte(cpu.get_16bit_register(Registers16Bit::HL));
        cpu.bit8_flag(5, value);
    },
    |cpu| {
        /* 0x6F */
        /* BIT 5, A */
        cpu.bit8_flag(5, cpu.registers.a);
    },
    |cpu| {
        /* 0x70 */
        /* BIT 6, B */
        cpu.bit8_flag(6, cpu.registers.b);
    },
    |cpu| {
        /* 0x71 */
        /* BIT 6, C */
        cpu.bit8_flag(6, cpu.registers.c);
    },
    |cpu| {
        /* 0x72 */
        /* BIT 6, D */
        cpu.bit8_flag(6, cpu.registers.d);
    },
    |cpu| {
        /* 0x73 */
        /* BIT 6, E */
        cpu.bit8_flag(6, cpu.registers.e);
    },
    |cpu| {
        /* 0x74 */
        /* BIT 6, H */
        cpu.bit8_flag(6, cpu.registers.h);
    },
    |cpu| {
        /* 0x75 */
        /* BIT 6, L */
        cpu.bit8_flag(6, cpu.registers.l);
    },
    |cpu| {
        /* 0x76 */
        /* BIT 6, [HL] */
        let value = cpu.read_byte(cpu.get_16bit_register(Registers16Bit::HL));
        cpu.bit8_flag(6, value);
    },
    |cpu| {
        /* 0x77 */
        /* BIT 6, A */
        cpu.bit8_flag(6, cpu.registers.a);
    },
    |cpu| {
        /* 0x78 */
        /* BIT 7, B */
        cpu.bit8_flag(7, cpu.registers.b);
    },
    |cpu| {
        /* 0x79 */
        /* BIT 7, C */
        cpu.bit8_flag(7, cpu.registers.c);
    },
    |cpu| {
        /* 0x7A */
        /* BIT 7, D */
        cpu.bit8_flag(7, cpu.registers.d);
    },
    |cpu| {
        /* 0x7B */
        /* BIT 7, E */
        cpu.bit8_flag(7, cpu.registers.e);
    },
    |cpu| {
        /* 0x7C */
        /* BIT 7, H */
        cpu.bit8_flag(7, cpu.registers.h);
    },
    |cpu| {
        /* 0x7D */
        /* BIT 7, L */
        cpu.bit8_flag(7, cpu.registers.l);
    },
    |cpu| {
        /* 0x7E */
        /* BIT 7, [HL] */
        let value = cpu.read_byte(cpu.get_16bit_register(Registers16Bit::HL));
        cpu.bit8_flag(7, value);
    },
    |cpu| {
        /* 0x7F */
        /* BIT 7, A */
        cpu.bit8_flag(7, cpu.registers.a);
    },
    |cpu| {
        /* 0x80 */
        /* RES 0, B */
        cpu.registers.b &= !(1 << 0);
    },
    |cpu| {
        /* 0x81 */
        /* RES 0, C */
        cpu.registers.c &= !(1 << 0);
    },
    |cpu| {
        /* 0x82 */
        /* RES 0, D */
        cpu.registers.d &= !(1 << 0);
    },
    |cpu| {
        /* 0x83 */
        /* RES 0, E */
        cpu.registers.e &= !(1 << 0);
    },
    |cpu| {
        /* 0x84 */
        /* RES 0, H */
        cpu.registers.h &= !(1 << 0);
    },
    |cpu| {
        /* 0x85 */
        /* RES 0, L */
        cpu.registers.l &= !(1 << 0);
    },
    |cpu| {
        /* 0x86 */
        /* RES 0, [HL] */
        let mut value = cpu.read_byte(cpu.get_16bit_register(Registers16Bit::HL));

        value &= !(1 << 0);

        cpu.write_byte(cpu.get_16bit_register(Registers16Bit::HL), value);
    },
    |cpu| {
        /* 0x87 */
        /* RES 0, A */
        cpu.registers.a &= !(1 << 0);
    },
    |cpu| {
        /* 0x88 */
        /* RES 1, B */
        cpu.registers.b &= !(1 << 1);
    },
    |cpu| {
        /* 0x89 */
        /* RES 1, C */
        cpu.registers.c &= !(1 << 1);
    },
    |cpu| {
        /* 0x8A */
        /* RES 1, D */
        cpu.registers.d &= !(1 << 1);
    },
    |cpu| {
        /* 0x8B */
        /* RES 1, E */
        cpu.registers.e &= !(1 << 1);
    },
    |cpu| {
        /* 0x8C */
        /* RES 1, H */
        cpu.registers.h &= !(1 << 1);
    },
    |cpu| {
        /* 0x8D */
        /* RES 1, L */
        cpu.registers.l &= !(1 << 1);
    },
    |cpu| {
        /* 0x8E */
        /* RES 1, [HL] */
        let mut value = cpu.read_byte(cpu.get_16bit_register(Registers16Bit::HL));

        value &= !(1 << 1);

        cpu.write_byte(cpu.get_16bit_register(Registers16Bit::HL), value);
    },
    |cpu| {
        /* 0x8F */
        /* RES 1, A */
        cpu.registers.a &= !(1 << 1);
    },
    |cpu| {
        /* 0x90 */
        /* RES 2, B */
        cpu.registers.b &= !(1 << 2);
    },
    |cpu| {
        /* 0x91 */
        /* RES 2, C */
        cpu.registers.c &= !(1 << 2);
    },
    |cpu| {
        /* 0x92 */
        /* RES 2, D */
        cpu.registers.d &= !(1 << 2);
    },
    |cpu| {
        /* 0x93 */
        /* RES 2, E */
        cpu.registers.e &= !(1 << 2);
    },
    |cpu| {
        /* 0x94 */
        /* RES 2, H */
        cpu.registers.h &= !(1 << 2);
    },
    |cpu| {
        /* 0x95 */
        /* RES 2, L */
        cpu.registers.l &= !(1 << 2);
    },
    |cpu| {
        /* 0x96 */
        /* RES 2, [HL] */
        let mut value = cpu.read_byte(cpu.get_16bit_register(Registers16Bit::HL));

        value &= !(1 << 2);

        cpu.write_byte(cpu.get_16bit_register(Registers16Bit::HL), value);
    },
    |cpu| {
        /* 0x97 */
        /* RES 2, A */
        cpu.registers.a &= !(1 << 2);
    },
    |cpu| {
        /* 0x98 */
        /* RES 3, B */
        cpu.registers.b &= !(1 << 3);
    },
    |cpu| {
        /* 0x99 */
        /* RES 3, C */
        cpu.registers.c &= !(1 << 3);
    },
    |cpu| {
        /* 0x9A */
        /* RES 3, D */
        cpu.registers.d &= !(1 << 3);
    },
    |cpu| {
        /* 0x9B */
        /* RES 3, E */
        cpu.registers.e &= !(1 << 3);
    },
    |cpu| {
        /* 0x9C */
        /* RES 3, H */
        cpu.registers.h &= !(1 << 3);
    },
    |cpu| {
        /* 0x9D */
        /* RES 3, L */
        cpu.registers.l &= !(1 << 3);
    },
    |cpu| {
        /* 0x9E */
        /* RES 3, [HL] */
        let mut value = cpu.read_byte(cpu.get_16bit_register(Registers16Bit::HL));

        value &= !(1 << 3);

        cpu.write_byte(cpu.get_16bit_register(Registers16Bit::HL), value);
    },
    |cpu| {
        /* 0x9F */
        /* RES 3, A */
        cpu.registers.a &= !(1 << 3);
    },
    |cpu| {
        /* 0xA0 */
        /* RES 4, B */
        cpu.registers.b &= !(1 << 4);
    },
    |cpu| {
        /* 0xA1 */
        /* RES 4, C */
        cpu.registers.c &= !(1 << 4);
    },
    |cpu| {
        /* 0xA2 */
        /* RES 4, D */
        cpu.registers.d &= !(1 << 4);
    },
    |cpu| {
        /* 0xA3 */
        /* RES 4, E */
        cpu.registers.e &= !(1 << 4);
    },
    |cpu| {
        /* 0xA4 */
        /* RES 4, H */
        cpu.registers.h &= !(1 << 4);
    },
    |cpu| {
        /* 0xA5 */
        /* RES 4, L */
        cpu.registers.l &= !(1 << 4);
    },
    |cpu| {
        /* 0xA6 */
        /* RES 4, [HL] */
        let mut value = cpu.read_byte(cpu.get_16bit_register(Registers16Bit::HL));

        value &= !(1 << 4);

        cpu.write_byte(cpu.get_16bit_register(Registers16Bit::HL), value);
    },
    |cpu| {
        /* 0xA7 */
        /* RES 4, A */
        cpu.registers.a &= !(1 << 4);
    },
    |cpu| {
        /* 0xA8 */
        /* RES 5, B */
        cpu.registers.b &= !(1 << 5);
    },
    |cpu| {
        /* 0xA9 */
        /* RES 5, C */
        cpu.registers.c &= !(1 << 5);
    },
    |cpu| {
        /* 0xAA */
        /* RES 5, D */
        cpu.registers.d &= !(1 << 5);
    },
    |cpu| {
        /* 0xAB */
        /* RES 5, E */
        cpu.registers.e &= !(1 << 5);
    },
    |cpu| {
        /* 0xAC */
        /* RES 5, H */
        cpu.registers.h &= !(1 << 5);
    },
    |cpu| {
        /* 0xAD */
        /* RES 5, L */
        cpu.registers.l &= !(1 << 5);
    },
    |cpu| {
        /* 0xAE */
        /* RES 5, [HL] */
        let mut value = cpu.read_byte(cpu.get_16bit_register(Registers16Bit::HL));

        value &= !(1 << 5);

        cpu.write_byte(cpu.get_16bit_register(Registers16Bit::HL), value);
    },
    |cpu| {
        /* 0xAF */
        /* RES 5, A */
        cpu.registers.a &= !(1 << 5);
    },
    |cpu| {
        /* 0xB0 */
        /* RES 6, B */
        cpu.registers.b &= !(1 << 6);
    },
    |cpu| {
        /* 0xB1 */
        /* RES 6, C */
        cpu.registers.c &= !(1 << 6);
    },
    |cpu| {
        /* 0xB2 */
        /* RES 6, D */
        cpu.registers.d &= !(1 << 6);
    },
    |cpu| {
        /* 0xB3 */
        /* RES 6, E */
        cpu.registers.e &= !(1 << 6);
    },
    |cpu| {
        /* 0xB4 */
        /* RES 6, H */
        cpu.registers.h &= !(1 << 6);
    },
    |cpu| {
        /* 0xB5 */
        /* RES 6, L */
        cpu.registers.l &= !(1 << 6);
    },
    |cpu| {
        /* 0xB6 */
        /* RES 6, [HL] */
        let mut value = cpu.read_byte(cpu.get_16bit_register(Registers16Bit::HL));

        value &= !(1 << 6);

        cpu.write_byte(cpu.get_16bit_register(Registers16Bit::HL), value);
    },
    |cpu| {
        /* 0xB7 */
        /* RES 6, A */
        cpu.registers.a &= !(1 << 6);
    },
    |cpu| {
        /* 0xB8 */
        /* RES 7, B */
        cpu.registers.b &= !(1 << 7);
    },
    |cpu| {
        /* 0xB9 */
        /* RES 7, C */
        cpu.registers.c &= !(1 << 7);
    },
    |cpu| {
        /* 0xBA */
        /* RES 7, D */
        cpu.registers.d &= !(1 << 7);
    },
    |cpu| {
        /* 0xBB */
        /* RES 7, E */
        cpu.registers.e &= !(1 << 7);
    },
    |cpu| {
        /* 0xBC */
        /* RES 7, H */
        cpu.registers.h &= !(1 << 7);
    },
    |cpu| {
        /* 0xBD */
        /* RES 7, L */
        cpu.registers.l &= !(1 << 7);
    },
    |cpu| {
        /* 0xBE */
        /* RES 7, [HL] */
        let mut value = cpu.read_byte(cpu.get_16bit_register(Registers16Bit::HL));

        value &= !(1 << 7);

        cpu.write_byte(cpu.get_16bit_register(Registers16Bit::HL), value);
    },
    |cpu| {
        /* 0xBF */
        /* RES 7, A */
        cpu.registers.a &= !(1 << 7);
    },
    |cpu| {
        /* 0xC0 */
        /* SET 0, B */
        cpu.registers.b |= 1 << 0;
    },
    |cpu| {
        /* 0xC1 */
        /* SET 0, C */
        cpu.registers.c |= 1 << 0;
    },
    |cpu| {
        /* 0xC2 */
        /* SET 0, D */
        cpu.registers.d |= 1 << 0;
    },
    |cpu| {
        /* 0xC3 */
        /* SET 0, E */
        cpu.registers.e |= 1 << 0;
    },
    |cpu| {
        /* 0xC4 */
        /* SET 0, H */
        cpu.registers.h |= 1 << 0;
    },
    |cpu| {
        /* 0xC5 */
        /* SET 0, L */
        cpu.registers.l |= 1 << 0;
    },
    |cpu| {
        /* 0xC6 */
        /* SET 0, [HL] */
        let mut value = cpu.read_byte(cpu.get_16bit_register(Registers16Bit::HL));

        value |= 1 << 0;

        cpu.write_byte(cpu.get_16bit_register(Registers16Bit::HL), value);
    },
    |cpu| {
        /* 0xC7 */
        /* SET 0, A */
        cpu.registers.a |= 1 << 0;
    },
    |cpu| {
        /* 0xC8 */
        /* SET 1, B */
        cpu.registers.b |= 1 << 1;
    },
    |cpu| {
        /* 0xC9 */
        /* SET 1, C */
        cpu.registers.c |= 1 << 1;
    },
    |cpu| {
        /* 0xCA */
        /* SET 1, D */
        cpu.registers.d |= 1 << 1;
    },
    |cpu| {
        /* 0xCB */
        /* SET 1, E */
        cpu.registers.e |= 1 << 1;
    },
    |cpu| {
        /* 0xCC */
        /* SET 1, H */
        cpu.registers.h |= 1 << 1;
    },
    |cpu| {
        /* 0xCD */
        /* SET 1, L */
        cpu.registers.l |= 1 << 1;
    },
    |cpu| {
        /* 0xCE */
        /* SET 1, [HL] */
        let mut value = cpu.read_byte(cpu.get_16bit_register(Registers16Bit::HL));

        value |= 1 << 1;

        cpu.write_byte(cpu.get_16bit_register(Registers16Bit::HL), value);
    },
    |cpu| {
        /* 0xCF */
        /* SET 1, A */
        cpu.registers.a |= 1 << 1;
    },
    |cpu| {
        /* 0xD0 */
        /* SET 2, B */
        cpu.registers.b |= 1 << 2;
    },
    |cpu| {
        /* 0xD1 */
        /* SET 2, C */
        cpu.registers.c |= 1 << 2;
    },
    |cpu| {
        /* 0xD2 */
        /* SET 2, D */
        cpu.registers.d |= 1 << 2;
    },
    |cpu| {
        /* 0xD3 */
        /* SET 2, E */
        cpu.registers.e |= 1 << 2;
    },
    |cpu| {
        /* 0xD4 */
        /* SET 2, H */
        cpu.registers.h |= 1 << 2;
    },
    |cpu| {
        /* 0xD5 */
        /* SET 2, L */
        cpu.registers.l |= 1 << 2;
    },
    |cpu| {
        /* 0xD6 */
        /* SET 2, [HL] */
        let mut value = cpu.read_byte(cpu.get_16bit_register(Registers16Bit::HL));

        value |= 1 << 2;

        cpu.write_byte(cpu.get_16bit_register(Registers16Bit::HL), value);
    },
    |cpu| {
        /* 0xD7 */
        /* SET 2, A */
        cpu.registers.a |= 1 << 2;
    },
    |cpu| {
        /* 0xD8 */
        /* SET 3, B */
        cpu.registers.b |= 1 << 3;
    },
    |cpu| {
        /* 0xD9 */
        /* SET 3, C */
        cpu.registers.c |= 1 << 3;
    },
    |cpu| {
        /* 0xDA */
        /* SET 3, D */
        cpu.registers.d |= 1 << 3;
    },
    |cpu| {
        /* 0xDB */
        /* SET 3, E */
        cpu.registers.e |= 1 << 3;
    },
    |cpu| {
        /* 0xDC */
        /* SET 3, H */
        cpu.registers.h |= 1 << 3;
    },
    |cpu| {
        /* 0xDD */
        /* SET 3, L */
        cpu.registers.l |= 1 << 3;
    },
    |cpu| {
        /* 0xDE */
        /* SET 3, [HL] */
        let mut value = cpu.read_byte(cpu.get_16bit_register(Registers16Bit::HL));

        value |= 1 << 3;

        cpu.write_byte(cpu.get_16bit_register(Registers16Bit::HL), value);
    },
    |cpu| {
        /* 0xDF */
        /* SET 3, A */
        cpu.registers.a |= 1 << 3;
    },
    |cpu| {
        /* 0xE0 */
        /* SET 4, B */
        cpu.registers.b |= 1 << 4;
    },
    |cpu| {
        /* 0xE1 */
        /* SET 4, C */
        cpu.registers.c |= 1 << 4;
    },
    |cpu| {
        /* 0xE2 */
        /* SET 4, D */
        cpu.registers.d |= 1 << 4;
    },
    |cpu| {
        /* 0xE3 */
        /* SET 4, E */
        cpu.registers.e |= 1 << 4;
    },
    |cpu| {
        /* 0xE4 */
        /* SET 4, H */
        cpu.registers.h |= 1 << 4;
    },
    |cpu| {
        /* 0xE5 */
        /* SET 4, L */
        cpu.registers.l |= 1 << 4;
    },
    |cpu| {
        /* 0xE6 */
        /* SET 4, [HL] */
        let mut value = cpu.read_byte(cpu.get_16bit_register(Registers16Bit::HL));

        value |= 1 << 4;

        cpu.write_byte(cpu.get_16bit_register(Registers16Bit::HL), value);
    },
    |cpu| {
        /* 0xE7 */
        /* SET 4, A */
        cpu.registers.a |= 1 << 4;
    },
    |cpu| {
        /* 0xE8 */
        /* SET 5, B */
        cpu.registers.b |= 1 << 5;
    },
    |cpu| {
        /* 0xE9 */
        /* SET 5, C */
        cpu.registers.c |= 1 << 5;
    },
    |cpu| {
        /* 0xEA */
        /* SET 5, D */
        cpu.registers.d |= 1 << 5;
    },
    |cpu| {
        /* 0xEB */
        /* SET 5, E */
        cpu.registers.e |= 1 << 5;
    },
    |cpu| {
        /* 0xEC */
        /* SET 5, H */
        cpu.registers.h |= 1 << 5;
    },
    |cpu| {
        /* 0xED */
        /* SET 5, L */
        cpu.registers.l |= 1 << 5;
    },
    |cpu| {
        /* 0xEE */
        /* SET 5, [HL] */
        let mut value = cpu.read_byte(cpu.get_16bit_register(Registers16Bit::HL));

        value |= 1 << 5;

        cpu.write_byte(cpu.get_16bit_register(Registers16Bit::HL), value);
    },
    |cpu| {
        /* 0xEF */
        /* SET 5, A */
        cpu.registers.a |= 1 << 5;
    },
    |cpu| {
        /* 0xF0 */
        /* SET 6, B */
        cpu.registers.b |= 1 << 6;
    },
    |cpu| {
        /* 0xF1 */
        /* SET 6, C */
        cpu.registers.c |= 1 << 6;
    },
    |cpu| {
        /* 0xF2 */
        /* SET 6, D */
        cpu.registers.d |= 1 << 6;
    },
    |cpu| {
        /* 0xF3 */
        /* SET 6, E */
        cpu.registers.e |= 1 << 6;
    },
    |cpu| {
        /* 0xF4 */
        /* SET 6, H */
        cpu.registers.h |= 1 << 6;
    },
    |cpu| {
        /* 0xF5 */
        /* SET 6, L */
        cpu.registers.l |= 1 << 6;
    },
    |cpu| {
        /* 0xF6 */
        /* SET 6, [HL] */
        let mut value = cpu.read_byte(cpu.get_16bit_register(Registers16Bit::HL));

        value |= 1 << 6;

        cpu.write_byte(cpu.get_16bit_register(Registers16Bit::HL), value);
    },
    |cpu| {
        /* 0xF7 */
        /* SET 6, A */
        cpu.registers.a |= 1 << 6;
    },
    |cpu| {
        /* 0xF8 */
        /* SET 7, B */
        cpu.registers.b |= 1 << 7;
    },
    |cpu| {
        /* 0xF9 */
        /* SET 7, C */
        cpu.registers.c |= 1 << 7;
    },
    |cpu| {
        /* 0xFA */
        /* SET 7, D */
        cpu.registers.d |= 1 << 7;
    },
    |cpu| {
        /* 0xFB */
        /* SET 7, E */
        cpu.registers.e |= 1 << 7;
    },
    |cpu| {
        /* 0xFC */
        /* SET 7, H */
        cpu.registers.h |= 1 << 7;
    },
    |cpu| {
        /* 0xFD */
        /* SET 7, L */
        cpu.registers.l |= 1 << 7;
    },
    |cpu| {
        /* 0xFE */
        /* SET 7, [HL] */
        let mut value = cpu.read_byte(cpu.get_16bit_register(Registers16Bit::HL));

        value |= 1 << 7;

        cpu.write_byte(cpu.get_16bit_register(Registers16Bit::HL), value);
    },
    |cpu| {
        /* 0xFF */
        /* SET 7, A */
        cpu.registers.a |= 1 << 7;
    },
];

#[cfg(feature = "debug")]
pub static OPCODES_STRING: [fn(pfx: u8) -> &'static str; 0x100] = [
    |_| "NOP",
    |_| "LD BC, n16",
    |_| "LD [BC], A",
    |_| "INC BC",
    |_| "INC B",
    |_| "DEC B",
    |_| "LD B, n8",
    |_| "RLCA",
    |_| "LD [a16], SP",
    |_| "ADD HL, BC",
    |_| "LD A, [BC]",
    |_| "DEC BC",
    |_| "INC C",
    |_| "DEC C",
    |_| "LD C, n8",
    |_| "RRCA",
    |_| "STOP n8",
    |_| "LD DE, n16",
    |_| "LD [DE], A",
    |_| "INC DE",
    |_| "INC D",
    |_| "DEC D",
    |_| "LD D, n8",
    |_| "RLA",
    |_| "JR e8",
    |_| "ADD HL, DE",
    |_| "LD A, [DE]",
    |_| "DEC DE",
    |_| "INC E",
    |_| "DEC E",
    |_| "LD E, n8",
    |_| "RRA",
    |_| "JR NZ, e8",
    |_| "LD HL, n16",
    |_| "LD [HL+], A",
    |_| "INC HL",
    |_| "INC H",
    |_| "DEC H",
    |_| "LD H, n8",
    |_| "DAA",
    |_| "JR Z, e8",
    |_| "ADD HL, HL",
    |_| "LD A, [HL+]",
    |_| "DEC HL",
    |_| "INC L",
    |_| "DEC L",
    |_| "LD L, n8",
    |_| "CPL",
    |_| "JR NC, e8",
    |_| "LD SP, n16",
    |_| "LD [HL-], A",
    |_| "INC SP",
    |_| "INC [HL]",
    |_| "DEC [HL]",
    |_| "LD [HL], n8",
    |_| "SCF",
    |_| "JR C, e8",
    |_| "ADD HL, SP",
    |_| "LD A, [HL-]",
    |_| "DEC SP",
    |_| "INC A",
    |_| "DEC A",
    |_| "LD A, n8",
    |_| "CCF",
    |_| "LD B, B",
    |_| "LD B, C",
    |_| "LD B, D",
    |_| "LD B, E",
    |_| "LD B, H",
    |_| "LD B, L",
    |_| "LD B, [HL]",
    |_| "LD B, A",
    |_| "LD C, B",
    |_| "LD C, C",
    |_| "LD C, D",
    |_| "LD C, E",
    |_| "LD C, H",
    |_| "LD C, L",
    |_| "LD C, [HL]",
    |_| "LD C, A",
    |_| "LD D, B",
    |_| "LD D, C",
    |_| "LD D, D",
    |_| "LD D, E",
    |_| "LD D, H",
    |_| "LD D, L",
    |_| "LD D, [HL]",
    |_| "LD D, A",
    |_| "LD E, B",
    |_| "LD E, C",
    |_| "LD E, D",
    |_| "LD E, E",
    |_| "LD E, H",
    |_| "LD E, L",
    |_| "LD E, [HL]",
    |_| "LD E, A",
    |_| "LD H, B",
    |_| "LD H, C",
    |_| "LD H, D",
    |_| "LD H, E",
    |_| "LD H, H",
    |_| "LD H, L",
    |_| "LD H, [HL]",
    |_| "LD H, A",
    |_| "LD L, B",
    |_| "LD L, C",
    |_| "LD L, D",
    |_| "LD L, E",
    |_| "LD L, H",
    |_| "LD L, L",
    |_| "LD L, [HL]",
    |_| "LD L, A",
    |_| "LD [HL], B",
    |_| "LD [HL], C",
    |_| "LD [HL], D",
    |_| "LD [HL], E",
    |_| "LD [HL], H",
    |_| "LD [HL], L",
    |_| "HALT",
    |_| "LD [HL], A",
    |_| "LD A, B",
    |_| "LD A, C",
    |_| "LD A, D",
    |_| "LD A, E",
    |_| "LD A, H",
    |_| "LD A, L",
    |_| "LD A, [HL]",
    |_| "LD A, A",
    |_| "ADD A, B",
    |_| "ADD A, C",
    |_| "ADD A, D",
    |_| "ADD A, E",
    |_| "ADD A, H",
    |_| "ADD A, L",
    |_| "ADD A, [HL]",
    |_| "ADD A, A",
    |_| "ADC A, B",
    |_| "ADC A, C",
    |_| "ADC A, D",
    |_| "ADC A, E",
    |_| "ADC A, H",
    |_| "ADC A, L",
    |_| "ADC A, [HL]",
    |_| "ADC A, A",
    |_| "SUB A, B",
    |_| "SUB A, C",
    |_| "SUB A, D",
    |_| "SUB A, E",
    |_| "SUB A, H",
    |_| "SUB A, L",
    |_| "SUB A, [HL]",
    |_| "SUB A, A",
    |_| "SBC A, B",
    |_| "SBC A, C",
    |_| "SBC A, D",
    |_| "SBC A, E",
    |_| "SBC A, H",
    |_| "SBC A, L",
    |_| "SBC A, [HL]",
    |_| "SBC A, A",
    |_| "AND A, B",
    |_| "AND A, C",
    |_| "AND A, D",
    |_| "AND A, E",
    |_| "AND A, h",
    |_| "AND A, L",
    |_| "AND A, [HL]",
    |_| "AND A, A",
    |_| "XOR A, B",
    |_| "XOR A, C",
    |_| "XOR A, D",
    |_| "XOR A, E",
    |_| "XOR A, H",
    |_| "XOR A, L",
    |_| "XOR A, [HL]",
    |_| "XOR A, A",
    |_| "OR A, B",
    |_| "OR A, C",
    |_| "OR A, D",
    |_| "OR A, E",
    |_| "OR A, H",
    |_| "OR A, L",
    |_| "OR A, [HL]",
    |_| "OR A, A",
    |_| "CP A, B",
    |_| "CP A, C",
    |_| "CP A, D",
    |_| "CP A, E",
    |_| "CP A, H",
    |_| "CP A, L",
    |_| "CP A, [HL]",
    |_| "CP A, A",
    |_| "RET NZ",
    |_| "POP BC",
    |_| "JP NZ, a16",
    |_| "JP a16",
    |_| "CALL NZ, a16",
    |_| "PUSH BC",
    |_| "ADD A, n8",
    |_| "RST $00",
    |_| "RET Z",
    |_| "RET",
    |_| "JP Z, a16",
    |pfx| PREFIXED_OPCODES_STRING[pfx as usize],
    |_| "CALL Z, a16",
    |_| "CALL a16",
    |_| "ADC A, n8",
    |_| "RST $08",
    |_| "RET NC",
    |_| "POP DE",
    |_| "JP NC, a16",
    |_| "_",
    |_| "CALL NC, a16",
    |_| "PUSH DE",
    |_| "SUB A, n8",
    |_| "RST $10",
    |_| "RET C",
    |_| "RETI",
    |_| "JP C, a16",
    |_| "_",
    |_| "CALL C, a16",
    |_| "_",
    |_| "SBC A, n8",
    |_| "RST $18",
    |_| "LDH [a8], A",
    |_| "POP HL",
    |_| "LDH [C], A",
    |_| "_",
    |_| "_",
    |_| "PUSH HL",
    |_| "AND A, n8",
    |_| "RST $20",
    |_| "ADD SP, e8",
    |_| "JP HL",
    |_| "LD [a16], A",
    |_| "_",
    |_| "_",
    |_| "_",
    |_| "XOR A, n8",
    |_| "RST $28",
    |_| "LDH A, [a8]",
    |_| "POP AF",
    |_| "LDH A, [C]",
    |_| "DI",
    |_| "_",
    |_| "PUSH AF",
    |_| "OR A, n8",
    |_| "RST $30",
    |_| "LD HL, SP + e8",
    |_| "LD SP, HL",
    |_| "LD A, [a16]",
    |_| "EI",
    |_| "_",
    |_| "_",
    |_| "CP A, n8",
    |_| "RST $38",
];

#[cfg(feature = "debug")]
pub static PREFIXED_OPCODES_STRING: [&str; 0x100] = [
    "RLC B",
    "RLC C",
    "RLC D",
    "RLC E",
    "RLC H",
    "RLC L",
    "RLC [HL]",
    "RLC A",
    "RRC B",
    "RRC C",
    "RRC D",
    "RRC E",
    "RRC H",
    "RRC L",
    "RRC [HL]",
    "RRC A",
    "RL B",
    "RL C",
    "RL D",
    "RL E",
    "RL H",
    "RL L",
    "RL [HL]",
    "RL A",
    "RR B",
    "RR C",
    "RR D",
    "RR E",
    "RR H",
    "RR L",
    "RR [HL]",
    "RR A",
    "SLA B",
    "SLA C",
    "SLA D",
    "SLA E",
    "SLA H",
    "SLA L",
    "SLA [HL]",
    "SLA A",
    "SRA B",
    "SRA C",
    "SRA D",
    "SRA E",
    "SRA H",
    "SRA L",
    "SRA [HL]",
    "SRA A",
    "SWAP B",
    "SWAP C",
    "SWAP D",
    "SWAP E",
    "SWAP H",
    "SWAP L",
    "SWAP [HL]",
    "SWAP A",
    "SRL B",
    "SRL C",
    "SRL D",
    "SRL E",
    "SRL H",
    "SRL L",
    "SRL [HL]",
    "SRL A",
    "BIT 0, B",
    "BIT 0, C",
    "BIT 0, D",
    "BIT 0, E",
    "BIT 0, H",
    "BIT 0, L",
    "BIT 0, [HL]",
    "BIT 0, A",
    "BIT 1, B",
    "BIT 1, C",
    "BIT 1, D",
    "BIT 1, E",
    "BIT 1, H",
    "BIT 1, L",
    "BIT 1, [HL]",
    "BIT 1, A",
    "BIT 2, B",
    "BIT 2, C",
    "BIT 2, D",
    "BIT 2, E",
    "BIT 2, H",
    "BIT 2, L",
    "BIT 2, [HL]",
    "BIT 2, A",
    "BIT 3, B",
    "BIT 3, C",
    "BIT 3, D",
    "BIT 3, E",
    "BIT 3, H",
    "BIT 3, L",
    "BIT 3, [HL]",
    "BIT 3, A",
    "BIT 4, B",
    "BIT 4, C",
    "BIT 4, D",
    "BIT 4, E",
    "BIT 4, H",
    "BIT 4, L",
    "BIT 4, [HL]",
    "BIT 4, A",
    "BIT 5, B",
    "BIT 5, C",
    "BIT 5, D",
    "BIT 5, E",
    "BIT 5, H",
    "BIT 5, L",
    "BIT 5, [HL]",
    "BIT 5, A",
    "BIT 6, B",
    "BIT 6, C",
    "BIT 6, D",
    "BIT 6, E",
    "BIT 6, H",
    "BIT 6, L",
    "BIT 6, [HL]",
    "BIT 6, A",
    "BIT 7, B",
    "BIT 7, C",
    "BIT 7, D",
    "BIT 7, E",
    "BIT 7, H",
    "BIT 7, L",
    "BIT 7, [HL]",
    "BIT 7, A",
    "RES 0, B",
    "RES 0, C",
    "RES 0, D",
    "RES 0, E",
    "RES 0, H",
    "RES 0, L",
    "RES 0, [HL]",
    "RES 0, A",
    "RES 1, B",
    "RES 1, C",
    "RES 1, D",
    "RES 1, E",
    "RES 1, H",
    "RES 1, L",
    "RES 1, [HL]",
    "RES 1, A",
    "RES 2, B",
    "RES 2, C",
    "RES 2, D",
    "RES 2, E",
    "RES 2, H",
    "RES 2, L",
    "RES 2, [HL]",
    "RES 2, A",
    "RES 3, B",
    "RES 3, C",
    "RES 3, D",
    "RES 3, E",
    "RES 3, H",
    "RES 3, L",
    "RES 3, [HL]",
    "RES 3, A",
    "RES 4, B",
    "RES 4, C",
    "RES 4, D",
    "RES 4, E",
    "RES 4, H",
    "RES 4, L",
    "RES 4, [HL]",
    "RES 4, A",
    "RES 5, B",
    "RES 5, C",
    "RES 5, D",
    "RES 5, E",
    "RES 5, H",
    "RES 5, L",
    "RES 5, [HL]",
    "RES 5, A",
    "RES 6, B",
    "RES 6, C",
    "RES 6, D",
    "RES 6, E",
    "RES 6, H",
    "RES 6, L",
    "RES 6, [HL]",
    "RES 6, A",
    "RES 7, B",
    "RES 7, C",
    "RES 7, D",
    "RES 7, E",
    "RES 7, H",
    "RES 7, L",
    "RES 7, [HL]",
    "RES 7, A",
    "SET 0, B",
    "SET 0, C",
    "SET 0, D",
    "SET 0, E",
    "SET 0, H",
    "SET 0, L",
    "SET 0, [HL]",
    "SET 0, A",
    "SET 1, B",
    "SET 1, C",
    "SET 1, D",
    "SET 1, E",
    "SET 1, H",
    "SET 1, L",
    "SET 1, [HL]",
    "SET 1, A",
    "SET 2, B",
    "SET 2, C",
    "SET 2, D",
    "SET 2, E",
    "SET 2, H",
    "SET 2, L",
    "SET 2, [HL]",
    "SET 2, A",
    "SET 3, B",
    "SET 3, C",
    "SET 3, D",
    "SET 3, E",
    "SET 3, H",
    "SET 3, L",
    "SET 3, [HL]",
    "SET 3, A",
    "SET 4, B",
    "SET 4, C",
    "SET 4, D",
    "SET 4, E",
    "SET 4, H",
    "SET 4, L",
    "SET 4, [HL]",
    "SET 4, A",
    "SET 5, B",
    "SET 5, C",
    "SET 5, D",
    "SET 5, E",
    "SET 5, H",
    "SET 5, L",
    "SET 5, [HL]",
    "SET 5, A",
    "SET 6, B",
    "SET 6, C",
    "SET 6, D",
    "SET 6, E",
    "SET 6, H",
    "SET 6, L",
    "SET 6, [HL]",
    "SET 6, A",
    "SET 7, B",
    "SET 7, C",
    "SET 7, D",
    "SET 7, E",
    "SET 7, H",
    "SET 7, L",
    "SET 7, [HL]",
    "SET 7, A",
];
