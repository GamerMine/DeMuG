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

#ifndef EMU_GAMEBOY_TIMER_H
#define EMU_GAMEBOY_TIMER_H

#include <raylib.h>

#include "Bus.h"

class Timer {
public:
    explicit Timer(class Bus *bus);

    void tick(uint8_t mCycle);

    inline static uint16_t DIV; // 0xFF04: Divider register
    inline static uint8_t TIMA; // 0xFF05: Timer counter
    inline static uint8_t TMA; // 0xFF06: Timer modulo

    inline static union {
        struct {
            bool clockSelect : 2;
            bool enable : 1;
            bool unused : 5;
        };
        uint8_t raw;
    } TAC{}; // 0xFF07: Timer Control

private:
    class Bus *mBus;

    uint16_t internalCounter;
    inline static std::array<uint16_t, 4> clockSpeed = {256, 4, 16, 64};
};


#endif //EMU_GAMEBOY_TIMER_H
