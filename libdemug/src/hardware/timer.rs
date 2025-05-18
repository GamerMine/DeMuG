/*
 *           ____
 *          /\  _`\                                       /'\_/`\  __
 *          \ \ \L\_\     __      ___ ___      __   _ __ /\      \/\_\    ___      __
 *           \ \ \L_L   /'__`\  /' __` __`\  /'__`\/\`'__\ \ \__\ \/\ \ /' _ `\  /'__`\
 *            \ \ \/, \/\ \L\.\_/\ \/\ \/\ \/\  __/\ \ \/ \ \ \_/\ \ \ \/\ \/\ \/\  __/
 *             \ \____/\ \__/.\_\ \_\ \_\ \_\ \____\\ \_\  \ \_\\ \_\ \_\ \_\ \_\ \____\
 *              \/___/  \/__/\/_/\/_/\/_/\/_/\/____/ \/_/   \/_/ \/_/\/_/\/_/\/_/\/____/
 *  
 *      Copyright (C) 2025 GamerMine
 *  
 *      This program is free software: you can redistribute it and/or modify
 *      it under the terms of the GNU General Public License as published by
 *      the Free Software Foundation, either version 3 of the License, or
 *      (at your option) any later version.
 *  
 *      This program is distributed in the hope that it will be useful,
 *      but WITHOUT ANY WARRANTY; without even the implied warranty of
 *      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *      GNU General Public License for more details.
 *  
 *      You should have received a copy of the GNU General Public License
 *      along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

use crate::Demug;
use crate::hardware::cpu::Interrupts;
use crate::utils::Register;
use std::sync::{Arc, RwLock};

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

pub(crate) struct Timer {
    registers: TimerRegisters,
    bus: Arc<RwLock<Demug>>,
    counter: u16,
}

impl Timer {
    pub(crate) fn init(bus: Arc<RwLock<Demug>>) -> Self {
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

    pub(crate) fn read(&self, addr: u16) -> u8 {
        match addr {
            0xFF04 => (self.registers.div >> 8) as u8,
            0xFF05 => self.registers.tima,
            0xFF06 => self.registers.tma,
            0xFF07 => self.registers.tac.value(),
            _ => {
                unreachable!()
            }
        }
    }

    pub(crate) fn write(&mut self, addr: u16, data: u8) {
        match addr {
            0xFF04 => self.registers.div = 0x0000,
            0xFF05 => self.registers.tima = data,
            0xFF06 => self.registers.tma = data,
            0xFF07 => self.registers.tac.set_value(data),
            _ => {
                unreachable!()
            }
        }
    }

    // FIXME: DOES NOT WORK AND I DON'T KNOW WHY.
    pub(crate) fn tick(&mut self, m_cycles: u64) {
        for _ in 0..m_cycles * 4 {
            self.registers.div = self.registers.div.wrapping_add(1);
        }

        if self.registers.tac.is_set(TacRegister::Enable as u8) {
            for _ in 0..m_cycles {
                self.counter += 1;

                let clock_select = self.registers.tac.bit(TacRegister::ClockSelectHi as u8) << 1
                    | self.registers.tac.bit(TacRegister::ClockSelectLo as u8);

                if self.counter % CLOCK_SPEEDS[clock_select as usize] == 0 {
                    self.registers.tima += 1
                }
                if self.counter == CLOCK_SPEEDS[clock_select as usize] {
                    self.counter = 0x0000
                }
                if self.registers.tima == 0xFF {
                    self.bus.read().unwrap().trigger_interrupt(Interrupts::Timer);
                    self.registers.tima = self.registers.tma;
                }
            }
        }
    }
}
