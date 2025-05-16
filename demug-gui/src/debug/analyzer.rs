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

use std::collections::HashMap;
use libdemug::hardware::cpu::opcodes::OPCODES_STRING;

pub fn analyze(binary_data: &Vec<u8>) -> HashMap<u16, AnalyzedValue> {
    let mut analyzed_data = HashMap::new();
    let mut i: usize = 0;

    while i < binary_data.len() {
        let addr = i;
        let opcode = binary_data[i];

        if opcode == 0xCB {
            i += 2;
            continue
        }
        
        let opcode_string = OPCODES_STRING[opcode as usize](0x00);

        for value_type in ["n8", "n16", "e8", "[a8]", "[a16]", "a16"] {
            if opcode_string.contains(value_type) {
                let mut value: u16 = 0x0000;
                if value_type.contains("8") {
                    i += 1;
                    value = binary_data[i] as u16;
                } else if value_type.contains("16") {
                    i += 1;
                    value = binary_data[i] as u16;
                    i += 1;
                    value |= (binary_data[i] as u16) << 8;
                }
                analyzed_data.insert(addr as u16, AnalyzedValue {opcode, value_type, value});
            }
        }
        i += 1;
    }

    analyzed_data
}

pub struct AnalyzedValue {
    pub opcode: u8,
    pub value_type: &'static str,
    pub value: u16,
}