#include "Bus.h"

bool Bus::GLOBAL_HALT = false;

Bus::Bus() {
    logger = Logger::getInstance("Bus");
    gameRom = new uint8_t[32*1024];
    romName = nullptr;
    readGameRom("Tetris.gb");
    readBootRom();
    ppu = new Ppu;
    std::thread ppuThread(std::ref(*ppu));
    cpu = new SharpSM83(this);
    std::thread cpuThread(std::ref(*cpu));

    cpuThread.join();
    ppuThread.join();
}

void Bus::write(uint16_t addr, uint8_t data) {
    if (addr == 0xFF47) logger->log(Logger::DEBUG, "%sWRITE%s: %X at %X", Colors::LOG_DARK_YELLOW, Colors::LOG_DEFAULT, data, addr);
    if (addr <= 0x00FF) { // DMG BOOT ROM
        // We should not write into the boot rom (Read Only)
        logger->log(Logger::WARNING, "Trying to write in an unauthorized area: 0x%X", addr);
    }
    if (addr >= 0x8000 && addr <= 0x9FFF) { // VRAM
        ppu->write(addr, data);
    }
    if (addr >= 0xC000 && addr <= 0xDFFF) { // WRAM
        ram[addr - 0xC000] = data;
    }
    if (addr >= 0xFF40 && addr <= 0xFF49) { // PPU
        ppu->write(addr, data);
    }
    if (addr >= 0xFF80 && addr <= 0xFFFE) { // HRAM
        hram[addr - 0xFF80] = data;
    }
}

uint8_t Bus::read(uint16_t addr) {
    uint8_t value = 0xFF;
    if (addr <= 0x00FF) value = bootRom[addr]; // DMG BOOT ROM
    if (addr >= 0x0100 && addr <= 0x1000 && romName != nullptr) value = gameRom[addr]; // Game cartridge
    if (addr >= 0x8000 && addr <= 0x9FFF) value = ppu->read(addr); // VRAM
    if (addr >= 0xC000 && addr <= 0xDFFF) value = ram[addr - 0xC000]; // WRAM
    if (addr >= 0xFF40 && addr <= 0xFF45) value = ppu->read(addr); // PPU
    if (addr >= 0xFF80 && addr <= 0xFFFE) value = hram[addr - 0xFF80]; // HRAM

    //if (addr == 0xFF44) logger->log(Logger::DEBUG, "%sREAD%s: %X at %X", Colors::GREEN, Colors::DEFAULT, value, addr);
    return value;
}

void Bus::readGameRom(const char *filename) {
    int i;
    logger->log(Logger::DEBUG, "Reading game : %s", filename);
    std::ifstream file(filename, std::ios::binary);

    if (!file.good()) {
        logger->log(Logger::CRITICAL, "File not found: %s", filename);
        return;
    }
    romName = filename;

    char byte;

    while (!file.eof()) {
        file.read(&byte, 1);
        gameRom[i++] = (uint8_t)byte;
    }
    i--;

    file.close();
}

void Bus::readBootRom() {
    uint16_t i;
    logger->log(Logger::DEBUG, "Reading boot rom...");
    std::ifstream file(BOOT_ROM_LOCATION, std::ios::binary);

    if (!file.good()) {
        logger->log(Logger::CRITICAL, "File not found: %s", BOOT_ROM_LOCATION);
        exit(1);
    }

    char byte;

    while (!file.eof()) {
        file.read(&byte, 1);
        bootRom[i++] = (uint8_t)byte;
    }
    i--;

    file.close();
}

void Bus::sendPpuWorkSignal() {
    ppu->bufferScreen = true;
}
