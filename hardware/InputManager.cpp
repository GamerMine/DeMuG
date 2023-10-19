#include "InputManager.h"

std::vector<InputManager::sKey> InputManager::registeredKeys = {};

InputManager::InputManager(class Bus *bus) {
    mBus = bus;
    registerKey(KEY_R);
    registerKey(KEY_SPACE);
    registerKey(KEY_N);
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
        default: {break;}
    }
}

void InputManager::keyReleased(int key) {
    (void) key;
}

