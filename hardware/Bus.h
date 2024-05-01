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
#define BOOT_ROM_SIZE 256
#define HRAM_SIZE 128

#include <array>
#include <cstring>
#include <fstream>

#include "../debug/Debug.h"
#include "Apu.h"
#include "../gui/MainWindow.h"
#include "InputManager.h"
#include "mappers/Cartridge.h"
#include "mappers/CartridgeHelper.h"
#include "Ppu.h"
#include "Screen.h"
#include "SerialIO.h"
#include "SharpSM83.h"
#include "Timer.h"

#ifdef __APPLE__
#include <CoreServices/CoreServices.h>
#endif

class Bus {
private:
    static std::string getBootROM();

public:
    explicit Bus(const char *filename);

    inline static std::string BOOT_ROM_LOCATION = getBootROM();
    static bool GLOBAL_HALT;
    static bool ENABLE_DEBUG;

    void startEmulation();
    void write(uint16_t addr, uint8_t data);
    uint8_t read(uint16_t addr);
    void tickApu();
    void tick(uint8_t mCycle);
    void runPpu();

    void reset();

    void loadGameROM(const char *filename);

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

    inline static bool gameLaunched;
    bool disableBootRom;

private:
    class Apu *apu;
    class SharpSM83 *cpu;
    class Ppu *ppu;
    class InputManager *inputManager;
    class Timer *timer;
    class SerialIO *serial;
    class MainWindow *mainWindow;
    Logger *logger;

    std::unique_ptr<Cartridge> cartridge;

    std::array<uint8_t, BOOT_ROM_SIZE> bootRom{};
    std::array<uint8_t, RAM_SIZE> wram{};
    std::array<uint8_t, HRAM_SIZE> hram{};

    void readBootRom();
};


#endif //EMU_GAMEBOY_BUS_H
