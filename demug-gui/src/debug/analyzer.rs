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