fn main() {
    //let demug = libdemug::Demug::init();
    
    let mut a: u16 = 0x9100;
    let orig = a;
    
    a = a.wrapping_add(0xFF00);
    add16_flag(orig, a);
    
    println!("{:#X}", a);
}

fn add16_flag(base_value: u16, result_value: u16) {
    if result_value < base_value {
        println!("HALF CARRY + CARRY");
    } else if result_value >> 12 > base_value >> 12 {
        println!("HALF CARRY");
    }
}