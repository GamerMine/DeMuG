#ifndef EMU_GAMEBOY_PPU_H
#define EMU_GAMEBOY_PPU_H

#include <cstdint>

#include "../logging/Logger.h"
#include "Screen.h"
#include "Bus.h"

class Ppu {
public:
    inline explicit Ppu() {
        LCDC.raw = 0x00;
        STAT.raw = 0x00;
        SCY = 0x00;
        SCX = 0x00;
        LY = 0x00;
        LYC = 0x00;

        screen = new Screen();
    }

    inline void operator()() {
        std::thread renderThread(std::ref(*screen));

        renderThread.join();
    }

    inline uint8_t read(uint16_t addr) {
        uint8_t value = 0xFF;
        if (addr == 0xFF40) value = LCDC.raw;
        if (addr == 0xFF41) value = STAT.raw;
        if (addr == 0xFF42) value = SCY;
        if (addr == 0xFF43) value = SCX;
        if (addr == 0xFF44) value = LY;
        if (addr == 0xFF45) value = LYC;

        return value;
    }

    inline void write(uint16_t addr, uint8_t data) {
        if (addr == 0xFF40) LCDC.raw = data;
        if (addr == 0xFF41) STAT.raw = data;
        if (addr == 0xFF42) SCY = data;
        if (addr == 0xFF43) SCX = data;
        if (addr == 0xFF44) Logger::getInstance("PPU")->log(Logger::WARNING, "LY register is read-only");
        if (addr == 0xFF45) LYC = data;
    }

private:
    class Screen *screen;

    union {
        struct {
            bool bgWinEnable : 1;
            bool objEnable : 1;
            bool objSize : 1;
            bool tileMapArea : 1;
            bool tileDataArea : 1;
            bool windowEnable : 1;
            bool tilemapArea : 1;
            bool lcdEnable : 1;
        };
        uint8_t raw;
    } LCDC {}; // PPU Control Register | 0xFF40

    union {
        struct {
            bool lcdMode : 2;
            bool LYCequalLY : 1;
            bool modeHBLANK : 1;
            bool modeVBLANK : 1;
            bool modeOAM : 1;
            bool modeLYCequalLY : 1;
            bool unused : 1;
        };
        uint8_t raw;
    } STAT {}; // PPU Stat Register | 0xFF41

    uint8_t SCY, SCX; // Vertical & Horizontal Scroll Registers | 0xFF42 & 0xFF43
    uint8_t LY, LYC; // Scanline Register & Scanline Compare Register | 0xFF44 & 0xFF45
};

#endif //EMU_GAMEBOY_PPU_H
