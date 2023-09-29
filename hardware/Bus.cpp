#include "Bus.h"

Bus::Bus() {
    screen = new Screen;
    for (uint8_t &i : ram) {
        i = 0x00;
    }
}

void Bus::write(uint16_t addr, uint8_t data) {
    if (addr >= 0x8000 && addr <= 0x9FFF) { // VRAM
        ram[addr - 0x8000] = data;
    }
    if (addr >= 0xC000 && addr <= 0xDFFF) { // WRAM
        ram[addr - 0xC000] = data;
    }
}

uint8_t Bus::read(uint16_t addr) {
    if (addr >= 0x8000 && addr <= 0x9FFF) { // VRAM
        return ram[addr - 0x8000];
    }
    if (addr >= 0xC000 && addr <= 0xDFFF) { // WRAM
        return ram[addr - 0xC000];
    }

    return 0x00;
}
