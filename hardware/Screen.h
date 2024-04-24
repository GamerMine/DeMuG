/*
 *  ____
 * /\  _`\                                       /'\_/`\  __
 * \ \ \L\_\     __      ___ ___      __   _ __ /\      \/\_\    ___      __
 *  \ \ \L_L   /'__`\  /' __` __`\  /'__`\/\`'__\ \ \__\ \/\ \ /' _ `\  /'__`\
 *   \ \ \/, \/\ \L\.\_/\ \/\ \/\ \/\  __/\ \ \/ \ \ \_/\ \ \ \/\ \/\ \/\  __/
 *    \ \____/\ \__/.\_\ \_\ \_\ \_\ \____\\ \_\  \ \_\\ \_\ \_\ \_\ \_\ \____\
 *     \/___/  \/__/\/_/\/_/\/_/\/_/\/____/ \/_/   \/_/ \/_/\/_/\/_/\/_/\/____/
 *
 * Copyright (c) 2023-2024 GamerMine <maxime-sav@outlook.fr>
 *
 * This Source Code From is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/ .
 */

#ifndef EMU_GAMEBOY_SCREEN_H
#define EMU_GAMEBOY_SCREEN_H

#define DEFAULT_WIDTH 160
#define DEFAULT_HEIGHT 144
#define FRAMERATE 59.7
#define WINDOW_NAME "DeMuG"

#include <cstdio>
#include <raylib.h>
#include <thread>

#include "../logging/Logger.h"
#include "Ppu.h"

class Screen {
public:
    explicit Screen(class Ppu *ppu);

    void run();
    void reset();

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
        bool isReal;
    };

    inline static bool VIEW_MEMORY = false;
    inline static uint8_t MEMORY_PAGE = 0;

    void tick(uint8_t mCycle);

    void close();

private:
    class Ppu *mPpu;

    uint8_t xPos;
    uint8_t yPos;
    uint16_t dots;

    struct Pixel {
        uint8_t r;
        uint8_t g;
        uint8_t b;
	    Pixel(){}
	    Pixel(uint8_t red, uint8_t green, uint8_t blue) : r(red), g(green), b(blue) {}
    };

    Pixel *screenPixelArray{};
    Pixel *tilesDataPixelArray{};
    Pixel *backgroundMapPixelArray{};
    Pixel *windowMapPixelArray{};

    uint8_t tileDataObj[0x100][8][8]{};
    uint8_t tilesDataNormal[0x100][8][8]{};
    std::array<Object, 40> objects {};

    Texture2D gameTexture{}, tilesDataTexture{}, backgroundMapTexture{}, windowMapTexture{};

    void DrawInstructions(int x, int y);
    static void DrawFlags(int x, int y);
    void DrawRegisters(int x, int y);
    void DrawMemory(int x, int y, int page);

    void render();
    void bufferTilesData();
    void setTileData();
    void setTileDataObj();
    void setObjects();
    void generateBackgroundTileMap();
    void generateWindowTileMap();
    void getObjectToRender(std::array<Object, 10> &out, uint8_t currentY);
    [[nodiscard]] Screen::Pixel getBGPPixelFromID(uint8_t pixelID) const;
    [[nodiscard]] Screen::Pixel getOBPPixelFromID(uint8_t pixelID, bool palette) const;

};


#endif //EMU_GAMEBOY_SCREEN_H
