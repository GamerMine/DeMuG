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
    internalCounter = 0x00;
}

void Timer::tick(uint8_t mCycle) {
    for (uint8_t i = 0; i < mCycle * 4; i++) {
        uint16_t oldDIV = DIV;
        DIV++;

        if (((oldDIV >> 12) & 0x01) && !((DIV >> 12) & 0x01)) mBus->tickApu();
    }

    if (TAC.enable) {
        for (uint8_t i = 0; i < mCycle; i++) {
            internalCounter++;
            if (internalCounter % clockSpeed[TAC.clockSelect] == 0) TIMA++;
            if (internalCounter == clockSpeed[TAC.clockSelect]) internalCounter = 0;
            if (TIMA == 0xFF) {
                SharpSM83::IF.timer = 1;
                TIMA = TMA;
            }
        }
    }
}