#ifndef EMU_GAMEBOY_SCREEN_H
#define EMU_GAMEBOY_SCREEN_H

#define DEFAULT_WIDTH 160
#define DEFAULT_HEIGHT 144
#define WINDOW_NAME "Emu_GameBoy"
#define LOGGER_NAME "Rendering"

#include <cstdio>
#include <GLFW/glfw3.h>
#include <GL/gl.h>
#include <thread>

#include "../logging/Logger.h"
#include "Ppu.h"

class Screen {
public:
    Screen(class Ppu *ppu);

    void operator()();

private:
    class Ppu *mPpu;
    GLFWwindow * window{};

};


#endif //EMU_GAMEBOY_SCREEN_H
