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

#[derive(Clone, Copy, Debug)]
pub struct Register {
    value: u8
}

impl Register {
    pub fn new(value: u8) -> Self {
        Self {
            value
        }
    }
    pub fn bit(&self, bit: u8) -> u8 {
        self.value >> bit & 0x01
    }
    pub fn set(&mut self, bit: u8) {
        self.value |= 1 << bit;
    }
    pub fn set_conditional(&mut self, bit: u8, condition: bool) {
        if condition {
            self.set(bit);
        } else {
            self.clear(bit);
        }
    }
    pub fn clear(&mut self, bit: u8) {
        self.value &= !(1 << bit);
    }
    pub fn value(&self) -> u8 {
        self.value
    }
    pub fn set_value(&mut self, value: u8) {
        self.value = value;
    }
}