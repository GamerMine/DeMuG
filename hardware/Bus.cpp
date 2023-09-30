#include "Bus.h"

Bus::Bus() {
    logger = Logger::getInstance("Bus");
    screen = new Screen();
    std::thread renderThread(*screen);
    cpu = new SharpSM83;

    for (uint8_t &i : ram) {
        i = 0x00;
    }

    renderThread.join();
}

void Bus::write(uint16_t addr, uint8_t data) {
    if (addr >= 0x0000 && addr <= 0x00FF) { // DMG BOOT ROM
        // We should not write into the boot rom (Read Only)
        logger->log(Logger::WARNING, "Trying to write in an unauthorized area: 0x%X", addr);
    }
    if (addr >= 0x8000 && addr <= 0x9FFF) { // VRAM
        vram[addr - 0x8000] = data;
    }
    if (addr >= 0xC000 && addr <= 0xDFFF) { // WRAM
        ram[addr - 0xC000] = data;
    }
    if (addr >= 0xFF80 && addr <= 0xFFFE) { // HRAM
        hram[addr - 0xFF80] = data;
    }
}

uint8_t Bus::read(uint16_t addr) {
    if (addr >= 0x0000 && addr <= 0x00FF) return bootRom[addr]; // DMG BOOT ROM
    if (addr >= 0x8000 && addr <= 0x9FFF) return vram[addr - 0x8000]; // VRAM
    if (addr >= 0xC000 && addr <= 0xDFFF) return ram[addr - 0xC000]; // WRAM
    if (addr >= 0xFF80 && addr <= 0xFFFE) return hram[addr - 0xFF80]; // HRAM

    return 0xFF;
}
