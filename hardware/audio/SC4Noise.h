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

#ifndef EMU_GAMEBOY_SC4NOISE_H
#define EMU_GAMEBOY_SC4NOISE_H

#include <cstdint>

class SC4Noise {
public:
    SC4Noise() {
        NR41.raw = 0xFF;
        NR42.raw = 0x00;
        NR43.raw = 0x00;
        NR44.raw = 0xBF;

        audioStream = LoadAudioStream(44100, 16, 2);
    }

    inline static union {
        struct {
            uint8_t initialLengthTimer : 6;
            uint8_t unused : 2;
        };
        uint8_t raw;
    } NR41{}; // 0xFF20: Channel 4 length timer

    inline static union {
        struct {
            bool sweepPace : 3;
            bool envDir : 1;
            bool initialVolume : 4;
        };
        uint8_t raw;
    } NR42{}; // 0xFF21: Channel 4 volume & envelope

    inline static union {
        struct {
            bool clockDivider : 3;
            bool LFSRWidth : 1;
            bool clockShift : 4;
        };
        uint8_t raw;
    } NR43{}; // 0xFF22: Channel 4 frequency & randomness

    inline static union {
        struct {
            uint8_t unused : 6;
            uint8_t lengthEnable : 1;
            uint8_t trigger : 1;
        };
        uint8_t raw;
    } NR44{}; // 0xFF23: Channel 4 control

    AudioStream audioStream{};
};

#endif //EMU_GAMEBOY_SC4NOISE_H
