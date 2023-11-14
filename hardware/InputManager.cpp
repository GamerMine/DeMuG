#include "InputManager.h"

std::vector<InputManager::sKey> InputManager::registeredKeys = {};

InputManager::InputManager(class Bus *bus) {
    mBus = bus;
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
            Bus::JOYP.selectButtons = !(BTN_A == key || BTN_B == key || BTN_SELECT == key || BTN_START == key);
            Bus::JOYP.selectDpad = !(DPAD_DOWN == key || DPAD_LEFT == key || DPAD_RIGHT == key || DPAD_UP == key);

            if (!Bus::JOYP.selectButtons) {
                if (BTN_A == key) Bus::JOYP.aOrRight = 0;
                if (BTN_B == key) Bus::JOYP.bOrLeft = 0;
                if (BTN_SELECT == key) Bus::JOYP.selectOrUp = 0;
                if (BTN_START == key) Bus::JOYP.startOrDown = 0;
            }

            if (!Bus::JOYP.selectDpad) {
                if (DPAD_RIGHT == key) Bus::JOYP.aOrRight = 0;
                if (DPAD_LEFT == key) Bus::JOYP.bOrLeft = 0;
                if (DPAD_UP == key) Bus::JOYP.selectOrUp = 0;
                if (DPAD_DOWN == key) Bus::JOYP.startOrDown = 0;
            }
            Logger::getInstance("InputManager")->log(Logger::DEBUG, "JOYP = %X\t Key pressed = %i", Bus::JOYP.raw, key);
            break;
        }
    }
}

void InputManager::keyReleased(int key) {
    if (!Bus::JOYP.selectButtons) {
        Bus::JOYP.selectButtons = 1;
        if (BTN_A == key) Bus::JOYP.aOrRight = 1;
        if (BTN_B == key) Bus::JOYP.bOrLeft = 1;
        if (BTN_SELECT == key) Bus::JOYP.selectOrUp = 1;
        if (BTN_START == key) Bus::JOYP.startOrDown = 1;
    }

    if (!Bus::JOYP.selectDpad) {
        Bus::JOYP.selectDpad = 1;
        if (DPAD_RIGHT == key) Bus::JOYP.aOrRight = 1;
        if (DPAD_LEFT == key) Bus::JOYP.bOrLeft = 1;
        if (DPAD_UP == key) Bus::JOYP.selectOrUp = 1;
        if (DPAD_DOWN == key) Bus::JOYP.startOrDown = 1;
    }
    Logger::getInstance("InputManager")->log(Logger::DEBUG, "JOYP = %X\tKey released = %i", Bus::JOYP.raw, key);
}

