#ifndef EMU_GAMEBOY_PPU_H
#define EMU_GAMEBOY_PPU_H

#include <cstdint>

#include "../logging/Logger.h"
#include "Screen.h"
#include "Bus.h"

// Color palette
#define DARK_GRAY 0x555555
#define LIGHT_GRAY 0xAAAAAA

class Ppu {
public:
    explicit Ppu();

    void operator()();

    uint8_t read(uint16_t addr) const;
    void write(uint16_t addr, uint8_t data);

public:
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

    union {
        struct {
            bool index0 : 2; // WHITE
            bool index1 : 2; // LIGHT GRAY
            bool index2 : 2; // DARK GRAY
            bool index3 : 2; // BLACK
        };
        uint8_t raw;
    } BGP{}; // 0xFF47

    union {
        struct {
            bool index0 : 2; // WHITE
            bool index1 : 2; // LIGHT GRAY
            bool index2 : 2; // DARK GRAY
            bool index3 : 2; // BLACK
        };
        uint8_t raw;
    } OBP0{}; // 0xFF48

    union {
        struct {
            bool index0 : 2; // WHITE
            bool index1 : 2; // LIGHT GRAY
            bool index2 : 2; // DARK GRAY
            bool index3 : 2; // BLACK
        };
        uint8_t raw;
    } OBP1{}; // 0xFF49

    uint8_t vram[RAM_SIZE]{};
};

#endif //EMU_GAMEBOY_PPU_H
