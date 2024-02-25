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

#ifndef EMU_GAMEBOY_BUS_H
#define EMU_GAMEBOY_BUS_H

#define RAM_SIZE 8192
#define BOOT_ROM_SIZE 0xFF
#define HRAM_SIZE 12
#define BOOT_ROM_LOCATION "boot/DMG_ROM.bin"

#include <array>
#include <fstream>

#include "Apu.h"
#include "InputManager.h"
#include "Ppu.h"
#include "Screen.h"
#include "SharpSM83.h"
#include "Timer.h"
#include "SerialIO.h"
#include "../debug/Debug.h"

class Bus {
public:
    Bus();

    static bool GLOBAL_HALT;

    void write(uint16_t addr, uint8_t data);
    uint8_t read(uint16_t addr);
    void tickApu();

    void reset();

    inline static union {
        struct {
            bool aOrRight : 1;
            bool bOrLeft : 1;
            bool selectOrUp : 1;
            bool startOrDown : 1;
            bool selectDpad : 1;
            bool selectButtons : 1;
            bool unused : 2;
        };
        uint8_t raw;
    } JOYP;

private:
    class Apu *apu;
    class SharpSM83 *cpu;
    class Ppu *ppu;
    class InputManager *inputManager;
    class Timer *timer;
    class SerialIO * serial;
    Logger *logger;

    const char *romName;
    uint8_t disableBootRom;

    uint8_t bootRom[BOOT_ROM_SIZE]{};
    uint8_t ram[RAM_SIZE]{};
    uint8_t hram[HRAM_SIZE]{};
    inline static std::array<uint8_t, 32*1024> gameRom;

    void readGameRom(const char *filename);
    void readBootRom();
};


#endif //EMU_GAMEBOY_BUS_H
