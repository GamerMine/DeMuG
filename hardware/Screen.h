#ifndef EMU_GAMEBOY_SCREEN_H
#define EMU_GAMEBOY_SCREEN_H

#define DEFAULT_WIDTH 160
#define DEFAULT_HEIGHT 144
#define FRAMERATE 59.7
#define WINDOW_NAME "Emu_GameBoy"

#include <cstdio>
#include <raylib.h>
#include <thread>

#include "../logging/Logger.h"
#include "Ppu.h"

class Screen {
public:
    explicit Screen(class Ppu *ppu);

    void operator()();
    void reset();

private:
    class Ppu *mPpu;

    struct Pixel {
        uint8_t r;
        uint8_t g;
        uint8_t b;
    };

    struct Object {
        uint8_t Ypos;
        uint8_t Xpos;
        uint8_t tileIndex;
        union {
            struct {
                bool unused : 4;
                bool dmgPalette : 1;
                bool xFlip : 1;
                bool yFlip : 1;
                bool priority : 1;
            };
            uint8_t attributes;
        };
    };

    Pixel *screenPixelArray{};
    Pixel *tilesDataPixelArray{};
    Pixel *backgroundMapPixelArray{};
    Pixel *windowMapPixelArray{};

    uint8_t tilesData[0xFF][8][8]{};
    Object objects[40];

    Texture2D gameTexture{}, tilesDataTexture{}, backgroundMapTexture{}, windowMapTexture{};

    void DrawInstructions(int x, int y);
    static void DrawFlags(int x, int y);

    void render();
    void renderTilesData();
    void setTileData();
    void setObjects();
    void generateBackgroundTileMap();
    void generateWindowTileMap();
    void bufferScreen();

};


#endif //EMU_GAMEBOY_SCREEN_H
