#ifndef EMU_GAMEBOY_INPUTMANAGER_H
#define EMU_GAMEBOY_INPUTMANAGER_H

#include <raylib.h>
#include <vector>

#include "Bus.h"

class InputManager {
public:
    explicit InputManager(class Bus *bus);

    void operator()();
    static void registerKey(int key);

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
