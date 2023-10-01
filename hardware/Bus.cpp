#include "Bus.h"

Bus::Bus() {
    logger = Logger::getInstance("Bus");
    readBootRom();
    screen = new Screen();
    std::thread renderThread(std::ref(*screen));
    cpu = new SharpSM83(this);
    std::thread cpuThread(std::ref(*cpu));

    renderThread.join();
    cpuThread.join();
}

void Bus::write(uint16_t addr, uint8_t data) {
    // logger->log(Logger::DEBUG, "WRITE: %X at %X", data, addr);
    if (addr <= 0x00FF) { // DMG BOOT ROM
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
    uint8_t value = 0xFF;
    if (addr <= 0x00FF) value = bootRom[addr]; // DMG BOOT ROM
    if (addr >= 0x8000 && addr <= 0x9FFF) value = vram[addr - 0x8000]; // VRAM
    if (addr >= 0xC000 && addr <= 0xDFFF) value = ram[addr - 0xC000]; // WRAM
    if (addr >= 0xFF80 && addr <= 0xFFFE) value = hram[addr - 0xFF80]; // HRAM

    // logger->log(Logger::DEBUG, "READ: %X at %X", value, addr);
    return value;
}

void Bus::readBootRom() {
    uint16_t i;
    logger->log(Logger::DEBUG, "Reading boot rom...");
    std::ifstream file(BOOT_ROM_LOCATION, std::ios::binary);

    if (!file.good()) {
        logger->log(Logger::CRITICAL, "File not found: %s", BOOT_ROM_LOCATION);
        return;
    }

    char byte;

    while (!file.eof()) {
        file.read(&byte, 1);
        bootRom[i++] = (uint8_t)byte;
    }
    i--;

    file.close();
}
