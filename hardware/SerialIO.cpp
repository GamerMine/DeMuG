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

#include "SerialIO.h"

SerialIO::SerialIO(class Bus *bus) {
    mBus = bus;
    SB = 0x00;
    SC.raw = 0x7E;

    currentTime = 0;
    lastTime = 0;
}

void SerialIO::tick() {
    currentTime = GetTime();
    if (currentTime - lastTime >= 1.0 / 8192) { // Internal clock is 8192Hz
        lastTime = currentTime;
        if (SC.transferEnable && SC.clockSelect) {
            SB <<= 1;
            SB |= 0x01;
            if (SB == 0xFF) {
                SC.transferEnable = 0;
                SharpSM83::IF.serial = 1;
            }
        }
    }
}
