#ifndef EMU_GAMEBOY_SCREEN_H
#define EMU_GAMEBOY_SCREEN_H

#define DEFAULT_WIDTH 160
#define DEFAULT_HEIGHT 144
#define WINDOW_NAME "Emu_GameBoy"

#include <GLFW/glfw3.h>
#include <cstdio>
#include <thread>

#include "../logging/Logger.h"

class Screen : public std::thread {
public:
    Screen();

};


#endif //EMU_GAMEBOY_SCREEN_H
