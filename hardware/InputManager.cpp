#include "InputManager.h"

std::vector<InputManager::sKey> InputManager::registeredKeys = {};

InputManager::InputManager(class Bus *bus) {
    mBus = bus;
    registerKey(KEY_R);
    registerKey(KEY_SPACE);
    registerKey(KEY_N);

    /*registerKey(BTN_A);      // A
    registerKey(BTN_B);      // B
    registerKey(BTN_SELECT); // SELECT
    registerKey(BTN_START);  // START
    registerKey(DPAD_LEFT);  // LEFT
    registerKey(DPAD_RIGHT); // RIGHT
    registerKey(DPAD_DOWN);  // DOWN
    registerKey(DPAD_UP);    // UP*/
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

/*void InputManager::unregisterKey(int key) {
    std::vector<sKey> tmpRegisteredKeys(InputManager::registeredKeys);
    InputManager::registeredKeys.clear();
    for (sKey &v_key : tmpRegisteredKeys) if (key != v_key.key) InputManager::registeredKeys.push_back(v_key);
    tmpRegisteredKeys.clear();
}*/

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
            /*if (BTN_A) {

            } else if (BTN_B) {

            } else if (BTN_START) {

            } else if (BTN_SELECT) {

            } else if (DPAD_LEFT) {

            } else if (DPAD_RIGHT) {

            } else if (DPAD_UP) {

            } else if (DPAD_DOWN) {

            }*/
            break;
        }
    }
}

void InputManager::keyReleased(int key) {
    (void) key;
}

