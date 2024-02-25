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

#ifndef EMU_GAMEBOY_SC2PULSE_H
#define EMU_GAMEBOY_SC2PULSE_H

#include <cstdint>

class SC2Pulse {
public:
    SC2Pulse() {
        NR21.raw = 0x3F;
        NR22.raw = 0x00;
        NR23 = 0xFF;
        NR24.raw = 0xBF;

        audioStream = LoadAudioStream(44100, 16, 2);
    }

    inline static union {
        struct {
            uint8_t initialLengthTimer : 6;
            uint8_t waveDuty : 2; // 00: 12.5%, 01: 25%, 10: 50%, 11: 75%
        };
        uint8_t raw;
    } NR21{}; // 0xFF11: Channel 1 Sound length time & duty cycle

    inline static union {
        struct {
            bool sweepPace : 3;
            bool envDirection : 2;
            bool initialVolume : 4;
        };
        uint8_t raw;
    } NR22{}; // 0xFF12: Channel 1 Volume & envelope

    inline static uint8_t NR23{}; // 0xFF13: Channel 1 period low

    inline static union {
        struct {
            uint8_t periodHigh : 3;
            uint8_t unused : 3;
            uint8_t lengthEnable : 1;
            uint8_t trigger : 1;
        };
        uint8_t raw;
    } NR24{}; // 0xFF14: Channel 1 period high & control

    AudioStream audioStream{};
};

#endif //EMU_GAMEBOY_SC2PULSE_H
