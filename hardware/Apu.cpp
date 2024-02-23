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

#include "Apu.h"

Apu::Apu(class Bus *bus) {
    mBus = bus;

    InitAudioDevice();


    channel1 = LoadAudioStream(44100, 16, 1);

    SetAudioStreamCallback(channel1, pulseWaveCallback);
    SetAudioStreamVolume(channel1, 0.01f);

    // PlayAudioStream(channel1);
}

float sineIdx = 0.0f;
void Apu::pulseWaveCallback(void *buffer, unsigned int frameCount) {
    float incr = 440.0f/44100.0f;
    auto *d = (short *)buffer;
    auto dutyCycle = 0.25f;

    for (unsigned int i = 0; i < frameCount; i++)
    {
        float value = (sinf(2.0f * PI * sineIdx) < (2 * dutyCycle - 1.0)) ? 1.0 : -1.0;
        d[i] = (short)(32000.0f * value);
        sineIdx += incr;
        if (sineIdx > 1.0f) sineIdx -= 1.0f;
    }
}

void Apu::operator()() {
    while (!Bus::GLOBAL_HALT) {}

    CloseAudioDevice();
}
