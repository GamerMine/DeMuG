/*
 *  ____
 * /\  _`\                                       /'\_/`\  __
 * \ \ \L\_\     __      ___ ___      __   _ __ /\      \/\_\    ___      __
 *  \ \ \L_L   /'__`\  /' __` __`\  /'__`\/\`'__\ \ \__\ \/\ \ /' _ `\  /'__`\
 *   \ \ \/, \/\ \L\.\_/\ \/\ \/\ \/\  __/\ \ \/ \ \ \_/\ \ \ \/\ \/\ \/\  __/
 *    \ \____/\ \__/.\_\ \_\ \_\ \_\ \____\\ \_\  \ \_\\ \_\ \_\ \_\ \_\ \____\
 *     \/___/  \/__/\/_/\/_/\/_/\/_/\/____/ \/_/   \/_/ \/_/\/_/\/_/\/_/\/____/
 *
 * Copyright (c) 2023-2024 GamerMine <maxime-sav@outlook.fr>
 *
 * This Source Code From is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/ .
 */

#ifndef EMU_GAMEBOY_PPU_H
#define EMU_GAMEBOY_PPU_H

#include <cstdint>

#include "../logging/Logger.h"
#include "Screen.h"
#include "Bus.h"

#define OAM_SIZE 0xA0

class Ppu {
public:
    explicit Ppu(class Bus *bus);

    class Bus *mBus;

    void reset();
    void tick(uint8_t mCycle) const;

    [[nodiscard]] uint8_t read(uint16_t addr) const;
    void write(uint16_t addr, uint8_t data);

private:
    void startTransfer();

public:
    class Screen *screen;

    union {
        struct {
            bool bgWinEnable : 1; // Bit 0
            bool objEnable : 1; // Bit 1
            bool objSize : 1; // Bit 2
            bool tileMapArea : 1; // Bit 3
            bool tileDataArea : 1; // Bit 4
            bool windowEnable : 1; // Bit 5
            bool tilemapArea : 1; // Bit 6
            bool lcdEnable : 1; // Bit 7
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
    uint8_t WY, WX; // Window screen coordinate | 0xFF4A & 0xFF4B
    uint8_t DMA; // Start a DMA transfer when writing to this register, Direct Memory Access

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

    std::array<uint8_t, RAM_SIZE> vram{};
    std::array<uint8_t, OAM_SIZE> OAM{};

    void runPpu() const;

    void closeConnection();
};

#endif //EMU_GAMEBOY_PPU_H
