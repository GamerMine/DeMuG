/*
 *  ____
 * /\  _`\                                       /'\_/`\  __
 * \ \ \L\_\     __      ___ ___      __   _ __ /\      \/\_\    ___      __
 *  \ \ \L_L   /'__`\  /' __` __`\  /'__`\/\`'__\ \ \__\ \/\ \ /' _ `\  /'__`\
 *   \ \ \/, \/\ \L\.\_/\ \/\ \/\ \/\  __/\ \ \/ \ \ \_/\ \ \ \/\ \/\ \/\  __/
 *    \ \____/\ \__/.\_\ \_\ \_\ \_\ \____\\ \_\  \ \_\\ \_\ \_\ \_\ \_\ \____\
 *     \/___/  \/__/\/_/\/_/\/_/\/_/\/____/ \/_/   \/_/ \/_/\/_/\/_/\/_/\/____/
 *
 * Copyright (c) 2024-2024 GamerMine <maxime-sav@outlook.fr>
 *
 * This Source Code From is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/ .
 */

#ifndef EMU_GAMEBOY_APU_H
#define EMU_GAMEBOY_APU_H

#include <cmath>
#include <cstdint>
#include <raylib.h>

#include "Bus.h"

class Apu {
public:
    explicit Apu(class Bus *bus);

    void operator()();

    // Global control registers
    inline static union {
        struct {
            bool ch1On : 1;
            bool ch2On : 1;
            bool ch3On : 1;
            bool ch4On : 1;
            bool unused  : 3;
            bool audioOnOff : 1;
        };
        uint8_t raw;
    } NR52{}; // 0xFF26: Audio master control

    inline static union {
        struct {
            bool ch1Right : 1;
            bool ch2Right : 1;
            bool ch3Right : 1;
            bool ch4Right : 1;
            bool ch1Left : 1;
            bool ch2Left : 1;
            bool ch3Left : 1;
            bool ch4Left : 1;
        };
        uint8_t raw;
    } NR51{}; // 0xFF25: Sound panning

    inline static union {
        struct {
            bool rightVolume : 3;
            bool vinRight : 1;
            bool leftVolume : 3;
            bool vinLeft : 1;
        };
        uint8_t raw;
    } NR50{}; // 0xFF24: Master volume & VIN panning

private:
    class Bus *mBus;

    static void pulseWaveCallback(void *buffer, unsigned int frames);

    AudioStream channel1;
};


#endif //EMU_GAMEBOY_APU_H
