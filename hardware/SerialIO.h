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

#ifndef EMU_GAMEBOY_SERIALIO_H
#define EMU_GAMEBOY_SERIALIO_H


#include <cstdint>

#include "Bus.h"

class SerialIO {
public:
    explicit SerialIO(class Bus *bus);

    void tick();

    inline static uint8_t SB; // 0xFF01: Serial transfer data

    inline static union {
        struct {
            bool clockSelect : 1; // 1 = Transfer requested or in progress
            bool unused : 6;
            bool transferEnable : 1; // 0 = External (slave) or 1 = internal (master) clock
        };
        uint8_t raw;
    } SC{}; // 0xFF02: Serial transfer control

private:
    class Bus *mBus;

    double currentTime;
    double lastTime;
};


#endif //EMU_GAMEBOY_SERIALIO_H
