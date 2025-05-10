use std::cell::RefCell;
use std::rc::Rc;
use crate::Demug;
use crate::hardware::cpu::Interrupts;
use crate::utils::Register;

const CLOCK_SPEEDS: [u16; 4] = [256, 4, 16, 64];

#[repr(u8)]
enum TacRegister {
    Enable = 2,
    ClockSelectHi = 1,
    ClockSelectLo = 0,
}

struct TimerRegisters {
    div: u16,
    tima: u8,
    tma: u8,
    tac: Register,
}

pub struct Timer {
    registers: TimerRegisters,
    bus: Rc<RefCell<Demug>>,
    counter: u16,
}

impl Timer {
    pub fn init(bus: Rc<RefCell<Demug>>) -> Self {
        Self {
            registers: TimerRegisters {
                div: 0xAB,
                tima: 0x00,
                tma: 0x00,
                tac: Register::new(0xF8),
            },
            bus,
            counter: 0x0000,
        }
    }

    pub fn read(&self, addr: u16) -> u8 {
        match addr {
            0xFF04 => (self.registers.div >> 8) as u8,
            0xFF05 => self.registers.tima,
            0xFF06 => self.registers.tma,
            0xFF07 => self.registers.tac.value(),
            _ => { unreachable!() }
        }
    }

    pub fn write(&mut self, addr: u16, data: u8) {
        match addr {
            0xFF04 => self.registers.div = 0x0000,
            0xFF05 => self.registers.tima = data,
            0xFF06 => self.registers.tma = data,
            0xFF07 => self.registers.tac.set_value(data),
            _ => { unreachable!() }
        }
    }

    // FIXME: DOES NOT WORK AND I DON'T KNOW WHY.
    pub fn tick(&mut self, m_cycles: u64) {
        for _ in 0..m_cycles * 4 {
            self.registers.div = self.registers.div.wrapping_add(1);
        }

        if self.registers.tac.bit(TacRegister::Enable as u8) == 0b1 {
            for _ in 0..m_cycles {
                self.counter += 1;

                let clock_select = self.registers.tac.bit(TacRegister::ClockSelectHi as u8) << 1 | self.registers.tac.bit(TacRegister::ClockSelectLo as u8); 
                
                if self.counter % CLOCK_SPEEDS[clock_select as usize] == 0 { self.registers.tima += 1 }
                if self.counter == CLOCK_SPEEDS[clock_select as usize] { self.counter = 0x0000 }
                if self.registers.tima == 0xFF {
                    self.bus.borrow().trigger_interrupt(Interrupts::Timer);
                    self.registers.tima = self.registers.tma;
                }
            }
        }
    }
}