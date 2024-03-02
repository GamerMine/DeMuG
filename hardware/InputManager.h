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

#ifndef EMU_GAMEBOY_INPUTMANAGER_H
#define EMU_GAMEBOY_INPUTMANAGER_H

#include <cstdint>
#include <raylib.h>
#include <vector>

#include "Bus.h"

class InputManager {
public:
    inline static int BTN_A = KEY_A;
    inline static int BTN_B = KEY_Z;
    inline static int BTN_SELECT = KEY_TAB;
    inline static int BTN_START = KEY_ENTER;
    inline static int DPAD_LEFT = KEY_LEFT;
    inline static int DPAD_RIGHT = KEY_RIGHT;
    inline static int DPAD_DOWN = KEY_DOWN;
    inline static int DPAD_UP = KEY_UP;

    inline static union {
        struct {
            bool A : 1;
            bool B : 1;
            bool select : 1;
            bool start : 1;
            bool unused : 4;
        };
        uint8_t raw;
    } JOY_BTN {};

    inline static union {
        struct {
            bool right : 1;
            bool left : 1;
            bool up : 1;
            bool down : 1;
            bool unused : 4;
        };
        uint8_t raw;
    } JOY_DPAD {};

public:
    explicit InputManager(class Bus *bus);

    void tick();
    static void registerKey(int key);
    static void unregisterKey(int key);

protected:
    void keyPressed(int key);
    void keyReleased(int key);

private:
    Bus *mBus;

    struct sKey {
        int key{};
        bool newStatus = false;
        bool oldStatus = false;
    };

    static std::vector<sKey> registeredKeys;
};


#endif //EMU_GAMEBOY_INPUTMANAGER_H
