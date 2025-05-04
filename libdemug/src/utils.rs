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
    pub fn sets(&mut self, bits: u8, value: u8) {
        self.value = (self.value & !bits) | (value & bits);
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