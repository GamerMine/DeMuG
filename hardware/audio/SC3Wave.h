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

#ifndef EMU_GAMEBOY_SC3WAVE_H
#define EMU_GAMEBOY_SC3WAVE_H

#include <cstdint>

class SC3Wave {
public:
    SC3Wave() {
        NR30.raw = 0x7F;
        NR31 = 0xFF;
        NR32.raw = 0x9F;
        NR33 = 0xFF;
        NR34.raw = 0xBF;

        audioStream = LoadAudioStream(44100, 16, 2);
    }

    inline static union {
        struct {
            uint8_t unused : 7;
            uint8_t DACOnOff : 1;
        };
        uint8_t raw;
    } NR30{}; // 0xFF1A: Channel 3 DAC enable

    inline static uint8_t NR31{}; // 0xFF1B: Channel 3 length timer

    inline static union {
        struct {
            bool unused : 5;
            bool outputLevel : 2;
            bool unused2 : 1;
        };
        uint8_t raw;
    } NR32{}; // 0xFF1C: Channel 3 output level

    inline static uint8_t NR33{}; // 0xFF1D: Channel 3 period low

    inline static union {
        struct {
            uint8_t periodHigh : 3;
            uint8_t unused : 3;
            uint8_t lengthEnable : 1;
            uint8_t trigger : 1;
        };
        uint8_t raw;
    } NR34{}; // 0xFF1E: Channel 3 period high & control

    AudioStream audioStream{};
};

#endif //EMU_GAMEBOY_SC3WAVE_H
