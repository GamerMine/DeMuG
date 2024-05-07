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
        uint8_t Ypos = 0x00;
        uint8_t Xpos = 0x00;
        uint8_t tileIndex = 0x00;
        union {
            struct {
                bool unused : 4;
                bool dmgPalette : 1;
                bool xFlip : 1;
                bool yFlip : 1;
                bool priority : 1;
            };
            uint8_t attributes = 0x00;
        };
        bool isReal = false;
        uint8_t index = 0;
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
	    Pixel() : r(0x00), g(0x00), b(0x00){}
	    Pixel(uint8_t red, uint8_t green, uint8_t blue) : r(red), g(green), b(blue) {}
    };

    Pixel *screenPixelArray{};

    uint8_t selectedObjCount;
    uint8_t discardedDotsCount;
    std::array<Object, 10> selectedObjects {};

    Texture2D gameTexture{}, tilesDataTexture{};

    void DrawInstructions(int x, int y);
    static void DrawFlags(int x, int y);
    void DrawRegisters(int x, int y);
    void DrawMemory(int x, int y, int page);

    void render();
    void oamScan();
    bool hasPriority(Object obj, uint8_t index);
    [[nodiscard]] Screen::Pixel getBGPPixelFromID(uint8_t pixelID) const;
    [[nodiscard]] static uint8_t getIDFromBGPPixel(Pixel pixel) ;
    [[nodiscard]] Screen::Pixel getOBPPixelFromID(uint8_t pixelID, bool palette) const;

    uint8_t getPixel(uint16_t tileIndex, uint8_t tilePixelX, uint8_t tilePixelY);
    uint8_t getPixelObj(uint16_t tileIndex, uint8_t tilePixelX, uint8_t tilePixelY);
    Pixel getBackgroundPixelAt(uint8_t x, uint8_t y);
    Pixel getWindowPixelAt(uint8_t x, uint8_t y);

};


#endif //EMU_GAMEBOY_SCREEN_H
