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

#include "InputManager.h"

std::vector<InputManager::sKey> InputManager::registeredKeys = {};

InputManager::InputManager(class Bus *bus) {
    mBus = bus;
    JOY_DPAD.raw = 0xFF;
    JOY_BTN.raw = 0xFF;
    registerKey(KEY_LEFT_CONTROL);

    registerKey(KEY_R);
    registerKey(KEY_SPACE);
    registerKey(KEY_N);
    registerKey(KEY_M);
    registerKey(KEY_KP_6);
    registerKey(KEY_KP_4);
    registerKey(KEY_O);
    registerKey(KEY_D);

    registerKey(BTN_A);      // A
    registerKey(BTN_B);      // B
    registerKey(BTN_SELECT); // SELECT
    registerKey(BTN_START);  // START
    registerKey(DPAD_LEFT);  // LEFT
    registerKey(DPAD_RIGHT); // RIGHT
    registerKey(DPAD_DOWN);  // DOWN
    registerKey(DPAD_UP);    // UP
}

void InputManager::tick() {
    for (sKey &key : InputManager::registeredKeys) {
        key.newStatus = IsKeyDown(key.key);
        if (key.newStatus != key.oldStatus) {
            if (key.newStatus) {
                keyPressed(key.key);
            } else {
                keyReleased(key.key);
            }
            key.oldStatus = key.newStatus;
        }
    }
}

void InputManager::registerKey(int key) {
    sKey newKey = {key, false, false};
    InputManager::registeredKeys.push_back(newKey);
}

void InputManager::unregisterKey(int key) {
    std::vector<sKey> tmpRegisteredKeys(InputManager::registeredKeys);
    InputManager::registeredKeys.clear();
    for (sKey &v_key : tmpRegisteredKeys) if (key != v_key.key) InputManager::registeredKeys.push_back(v_key);
    tmpRegisteredKeys.clear();
}

void InputManager::keyPressed(int key) {
    switch (key) {
        case KEY_SPACE: {
            SharpSM83::PAUSE = !SharpSM83::PAUSE;
            break;
        }
        case KEY_R: {
            mBus->reset();
            break;
        }
        case KEY_N: {
            SharpSM83::NEXT_INSTR = true;
            break;
        }
        case KEY_M: {
            Screen::VIEW_MEMORY = !Screen::VIEW_MEMORY;
            break;
        }
        case KEY_KP_6: {
            Screen::MEMORY_PAGE++;
            break;
        }
        case KEY_KP_4: {
            Screen::MEMORY_PAGE--;
            break;
        }
        default: {
            if (BTN_A == key)       JOY_BTN.A = 0;
            if (BTN_B == key)       JOY_BTN.B = 0;
            if (BTN_SELECT == key)  JOY_BTN.select = 0;
            if (BTN_START == key)   JOY_BTN.start = 0;

            if (DPAD_RIGHT == key)  JOY_DPAD.right = 0;
            if (DPAD_LEFT == key)   JOY_DPAD.left = 0;
            if (DPAD_UP == key)     JOY_DPAD.up = 0;
            if (DPAD_DOWN == key)   JOY_DPAD.down = 0;

            break;
        }
    }

    if (key == KEY_O && getKeyState(KEY_LEFT_CONTROL)) {
        const char *file = FileChooser::getInstance()->chooseROM();
        if (std::strcmp(file, "") != 0) mBus->loadGameROM(file);
    } else if (key == KEY_D && getKeyState(KEY_LEFT_CONTROL)) {
        Bus::ENABLE_DEBUG  = !Bus::ENABLE_DEBUG;
        if (Bus::ENABLE_DEBUG) SetWindowSize(1280, 720);
        else SetWindowSize(800, 720);
    }
}

void InputManager::keyReleased(int key) {
    if (BTN_A == key)       JOY_BTN.A = 1;
    if (BTN_B == key)       JOY_BTN.B = 1;
    if (BTN_SELECT == key)  JOY_BTN.select = 1;
    if (BTN_START == key)   JOY_BTN.start = 1;

    if (DPAD_RIGHT == key)  JOY_DPAD.right = 1;
    if (DPAD_LEFT == key)   JOY_DPAD.left = 1;
    if (DPAD_UP == key)     JOY_DPAD.up = 1;
    if (DPAD_DOWN == key)   JOY_DPAD.down = 1;

    if (KEY_N == key) SharpSM83::NEXT_INSTR = false;
}

bool InputManager::getKeyState(int key) {
    for (sKey &rkey : InputManager::registeredKeys) {
        if (rkey.key == key) return rkey.newStatus;
    }

    return false;
}

