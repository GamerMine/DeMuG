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

#include "Timer.h"

Timer::Timer(class Bus *bus) {
    mBus = bus;
    DIV = 0x18;
    TIMA = 0x00;
    TMA = 0x00;
    TAC.raw = 0xF8;
}

void Timer::operator()() {
    double currentTime;
    double lastTimeDiv;
    double lastTimeTima;
    bool timaOverflow = false;

    while(!Bus::GLOBAL_HALT) {
        currentTime = GetTime();

        if(timaOverflow) {
            timaOverflow = false;
            SharpSM83::IF.timer = 1;
            TIMA = TMA;
        }

        if (TAC.enable) {
            if (currentTime - lastTimeTima >= 1.0 / clockSpeed[TAC.clockSelect]) {
                lastTimeTima = currentTime;
                if (TIMA == 0xFF) {
                    TIMA = 0x00;
                    timaOverflow = true;
                } else { TIMA++; }
            }
        }

        if (currentTime - lastTimeDiv >= 1.0 / 16384) {
            lastTimeDiv = currentTime;
            DIV++;
        }
    }
}