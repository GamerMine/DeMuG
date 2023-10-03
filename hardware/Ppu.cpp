#include "Ppu.h"

Ppu::Ppu() {
    LCDC.raw = 0x00;
    STAT.raw = 0x00;
    SCY = 0x00;
    SCX = 0x00;
    LY = 0x00;
    LYC = 0x00;

    screen = new Screen(this);
}

void Ppu::operator()() {
    std::thread screenThread(std::ref(*screen));
    screenThread.join();
}

uint8_t Ppu::read(uint16_t addr) const {
    uint8_t value = 0xFF;
    if (addr == 0xFF40) value = LCDC.raw;
    if (addr == 0xFF41) value = STAT.raw;
    if (addr == 0xFF42) value = SCY;
    if (addr == 0xFF43) value = SCX;
    if (addr == 0xFF44) value = LY;
    if (addr == 0xFF45) value = LYC;

    return value;
}

void Ppu::write(uint16_t addr, uint8_t data) {
    if (addr == 0xFF40) LCDC.raw = data;
    if (addr == 0xFF41) STAT.raw = data;
    if (addr == 0xFF42) SCY = data;
    if (addr == 0xFF43) SCX = data;
    if (addr == 0xFF44) Logger::getInstance("PPU")->log(Logger::WARNING, "LY register is read-only");
    if (addr == 0xFF45) LYC = data;
}
