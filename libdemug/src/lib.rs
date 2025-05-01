use std::cell::RefCell;
use crate::hardware::cpu::Cpu;
use std::sync::Arc;
use crate::hardware::memory::Memory;

mod hardware;

pub struct Demug {
    cpu: Option<Cpu>,
    memory: Option<Memory>
}

impl Demug {
    pub fn init() -> Arc<RefCell<Self>> {
        let demug = Arc::new(RefCell::new(Demug { cpu: None, memory: None }));

        // Create all necessary 'devices'
        let cpu = Cpu::new(demug.clone());
        let memory = Memory::init();

        demug.borrow_mut().cpu = Some(cpu);
        demug.borrow_mut().memory = Some(memory);

        demug
    }
    
    fn read(&self, addr: u16) -> u8 {
        let mut data: u8 = 0x00;
        
        data
    }
    
    fn write(&mut self, addr: u16, data: u8) {
        
    }
}
