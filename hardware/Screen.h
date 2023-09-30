#ifndef EMU_GAMEBOY_SCREEN_H
#define EMU_GAMEBOY_SCREEN_H

#define DEFAULT_WIDTH 160
#define DEFAULT_HEIGHT 144
#define WINDOW_NAME "Emu_GameBoy"
#define LOGGER_NAME "Rendering"

#include <cstdio>
#include <GLFW/glfw3.h>
#include <thread>

#include "../logging/Logger.h"

class Screen {
public:
    void operator()();

private:
    GLFWwindow * window;

};


#endif //EMU_GAMEBOY_SCREEN_H
