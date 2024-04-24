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
#include <random>

class SC4Noise {
public:
    SC4Noise() {
        NR41.raw = 0xFF;
        NR42.raw = 0x00;
        NR43.raw = 0x00;
        NR44.raw = 0xBF;

        audioStream = LoadAudioStream(32, 8, 2);

        SetAudioStreamCallback(audioStream, noiseCallback);
        SetAudioStreamVolume(audioStream, 1.0f);
    }

    inline static union {
        struct {
            uint8_t initialLengthTimer : 6;
            uint8_t unused : 2;
        };
        uint8_t raw;
    } NR41{}; // 0xFF20: Channel 4 length timer

    union envControl {
        struct {
            uint8_t sweepPace : 3;
            bool nvDirection : 1;
            uint8_t initialVolume : 4;
        };
        uint8_t raw;
    }; // 0xFF21: Channel 4 volume & envelope
    inline static  envControl NR42;
    inline static  envControl NR42Temp;

    inline static union {
        struct {
            uint8_t clockDivider : 3;
            bool LFSRWidth : 1;
            uint8_t clockShift : 4;
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

    inline static bool DAC;
    inline static int8_t currentVolume;
    inline static uint8_t enveloppeTick;

    inline static AudioStream audioStream;

private:
    inline static std::mt19937 generator{std::random_device{}()};
    inline static std::uniform_int_distribution<uint8_t> distribution{0, 255};

    static void noiseCallback(void *buffer, unsigned int frameCount) {
        auto *d = (uint8_t *)buffer;
        double frequency = 262144.0 / (NR43.clockDivider * (1 << NR43.clockShift));

        SetAudioStreamPitch(audioStream, frequency / 1.0);

        uint8_t frameSkip = 8;
        for (unsigned int i = 0; i < frameCount; i+=frameSkip) {
            uint8_t sample = distribution(generator);
            for (uint8_t j = 0; j < frameSkip * 2; j++) {
                d[i * 2 + j] = sample;
            }
        }
    }
};

#endif //EMU_GAMEBOY_SC4NOISE_H
