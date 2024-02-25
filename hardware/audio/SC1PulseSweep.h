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

#ifndef EMU_GAMEBOY_SC1PULSESWEEP_H
#define EMU_GAMEBOY_SC1PULSESWEEP_H

#include <array>
#include <cmath>
#include <cstdint>

#include "raylib.h"

class SC1PulseSweep {
public:
    SC1PulseSweep() {
        NR10.raw = 0x80;
        NR11.raw = 0xBF;
        NR12.raw = 0xF3;
        NR13 = 0xFF;
        NR14.raw = 0xBF;

        audioStream = LoadAudioStream(44100, 16, 2);

        SetAudioStreamCallback(audioStream, pulseWaveCallback);
    }

    AudioStream audioStream{};

    inline static union {
        struct {
            bool individualStep : 3;
            bool direction : 1;
            bool pace : 3;
            bool unused : 1;
        };
        uint8_t raw;
    } NR10{}; // 0xFF10: Channel 1 Sweep

    inline static union {
        struct {
            uint8_t initialLengthTimer : 6;
            uint8_t waveDuty : 2; // 00: 12.5%, 01: 25%, 10: 50%, 11: 75%
        };
        uint8_t raw;
    } NR11{}; // 0xFF11: Channel 1 Sound length time & duty cycle

    inline static union {
        struct {
            bool sweepPace : 3;
            bool envDirection : 2;
            bool initialVolume : 4;
        };
        uint8_t raw;
    } NR12{}; // 0xFF12: Channel 1 Volume & envelope

    inline static uint8_t NR13{}; // 0xFF13: Channel 1 period low

    inline static union {
        struct {
            uint8_t periodHigh : 3;
            uint8_t unused : 3;
            uint8_t lengthEnable : 1;
            uint8_t trigger : 1;
        };
        uint8_t raw;
    } NR14{}; // 0xFF14: Channel 1 period high & control

private:
    inline static std::array<float, 4> dutyCycles = {0.125f, 0.25f, 0.50f, 0.75f};

    static inline float sineIdx = 0.0f;
    static void pulseWaveCallback(void *buffer, unsigned int frameCount) {
        uint16_t frequency = 4194304 / (4 * 2 * (2048 - ((NR14.periodHigh << 8) | NR13)));
        float incr = frequency / 44100.0f;
        auto *d = (short *)buffer;
        auto dutyCycle = dutyCycles[NR11.waveDuty];

        for (unsigned int i = 0; i < frameCount; i++) {
            float value = (sinf(2.0f * PI * sineIdx) < (2 * dutyCycle - 1.0)) ? 1.0 : -1.0;
            d[i * 2] = (short)(32000.0f * value);
            d[i * 2 + 1] = d[i * 2];
            sineIdx += incr;
            if (sineIdx > 1.0f) sineIdx -= 1.0f;
        }
    }
};

#endif //EMU_GAMEBOY_SC1PULSESWEEP_H
