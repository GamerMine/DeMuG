fn main() {
    //let demug = libdemug::Demug::init();
    
    let mut a: u8 = 0x35;

    a = !a;

    println!("{:#X}", a);
}