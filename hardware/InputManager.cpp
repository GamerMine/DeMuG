#include "InputManager.h"

std::vector<InputManager::sKey> InputManager::registeredKeys = {};

InputManager::InputManager(class Bus *bus) {
    mBus = bus;
    JOY_DPAD.raw = 0xFF;
    JOY_BTN.raw = 0xFF;
    registerKey(KEY_R);
    registerKey(KEY_SPACE);
    registerKey(KEY_N);

    registerKey(BTN_A);      // A
    registerKey(BTN_B);      // B
    registerKey(BTN_SELECT); // SELECT
    registerKey(BTN_START);  // START
    registerKey(DPAD_LEFT);  // LEFT
    registerKey(DPAD_RIGHT); // RIGHT
    registerKey(DPAD_DOWN);  // DOWN
    registerKey(DPAD_UP);    // UP
}

void InputManager::operator()() {
    while (!Bus::GLOBAL_HALT) {
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
        default: {
            if (BTN_A == key)       JOY_BTN.A = 0;
            if (BTN_B == key)       JOY_BTN.B = 0;
            if (BTN_SELECT == key)  JOY_BTN.select = 0;
            if (BTN_START == key)   JOY_BTN.start = 0;

            if (DPAD_RIGHT == key)  JOY_DPAD.right = 0;
            if (DPAD_LEFT == key)   JOY_DPAD.left = 0;
            if (DPAD_UP == key)     JOY_DPAD.up = 0;
            if (DPAD_DOWN == key)   JOY_DPAD.down = 0;
            //SharpSM83::ENABLE_DEBUG_PRINTS = true;
            Logger::getInstance("InputManager")->log(Logger::DEBUG, "JOYP = %X\t Key pressed = %i", Bus::JOYP.raw, key);
            break;
        }
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
    Logger::getInstance("InputManager")->log(Logger::DEBUG, "JOYP = %X\tKey released = %i", Bus::JOYP.raw, key);
}

