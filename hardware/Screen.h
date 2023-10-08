#ifndef EMU_GAMEBOY_SCREEN_H
#define EMU_GAMEBOY_SCREEN_H

#define DEFAULT_WIDTH 160
#define DEFAULT_HEIGHT 144
#define FRAMERATE 59.7
#define WINDOW_NAME "Emu_GameBoy"
#define LOGGER_NAME "Rendering"

#include <cstdio>
#include "raylib.h"
#include <thread>

#include "../logging/Logger.h"
#include "Ppu.h"

class Screen {
public:
    Screen(class Ppu *ppu);

    void operator()();

private:
    class Ppu *mPpu;

    struct Pixel {
        uint8_t r;
        uint8_t g;
        uint8_t b;
    };

    Pixel *screen{};
    Pixel *tiles{};

    Texture2D gameTexture, tilesTexture;

    void render();
    void renderTiles();

};


#endif //EMU_GAMEBOY_SCREEN_H
