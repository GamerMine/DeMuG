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

#include "Screen.h"

Screen::Screen(class Ppu *ppu) {
    mPpu = ppu;
    xPos = 0x00;
    yPos = 0x00;
    dots = 0x0000;

    screenPixelArray = new Pixel[DEFAULT_WIDTH * DEFAULT_HEIGHT];
    tilesDataPixelArray  = new Pixel[16 * 8 * 16 * 8]; // There are 256 tiles to render, so a 16x16 square is sufficient, but a tile is 8x8 pixels

    // Initialize screen pixel array
    for (long i = 0; i < DEFAULT_WIDTH*DEFAULT_HEIGHT; i++) screenPixelArray[i] = Pixel(0x00, 0x00, 0x00);

    // Initialize tileset pixel array
    for (long i = 0; i < 16*8*16*8; i++) tilesDataPixelArray[i] = Pixel(0x00, 0x00, 0x00);

    InitWindow(Bus::ENABLE_DEBUG ? 1280 : 800, 720, WINDOW_NAME);
    SetWindowMonitor(0);
#ifndef __APPLE__
    SetWindowIcon(LoadImage("resources/icon.png"));
#endif

    Image gameRender = {.data = screenPixelArray, .width = DEFAULT_WIDTH, .height = DEFAULT_HEIGHT, .mipmaps = 1, .format = PIXELFORMAT_UNCOMPRESSED_R8G8B8};
    Image tilesDataRender = {.data = tilesDataPixelArray, .width = 16 * 8, .height = 16 * 8, .mipmaps = 1, .format = PIXELFORMAT_UNCOMPRESSED_R8G8B8};

    gameTexture = LoadTextureFromImage(gameRender);
    tilesDataTexture = LoadTextureFromImage(tilesDataRender);
}

void Screen::run() {
    render();
}

uint16_t tileDataBlock = 0x8000;
void Screen::render() {
    BeginDrawing();
    ClearBackground(BLACK);
    if (mPpu->LCDC.lcdEnable && !VIEW_MEMORY) {
        DrawTexturePro(gameTexture,
                       (Rectangle) {0, 0, static_cast<float>(gameTexture.width),
                                    static_cast<float>(gameTexture.height)},
                       (Rectangle) {0, 0, static_cast<float>(gameTexture.width * 5),
                                    static_cast<float>(gameTexture.height * 5)},
                       (Vector2) {0, 0},
                       0,
                       RAYWHITE);
    }
    if (Bus::ENABLE_DEBUG) {
        DrawTexturePro(tilesDataTexture,
                       (Rectangle) {0, 0, static_cast<float>(tilesDataTexture.width),
                                    static_cast<float>(tilesDataTexture.height)},
                       (Rectangle) {810, static_cast<float>(GetRenderHeight() - tilesDataTexture.height * 2 - 10),
                                    static_cast<float>(tilesDataTexture.width * 2),
                                    static_cast<float>(tilesDataTexture.height * 2)},
                       (Vector2) {0, 0},
                       0,
                       RAYWHITE);
        DrawInstructions(820, 0);
        DrawFlags(1100, 0);
        DrawRegisters(1100, 100);
    }
    if (VIEW_MEMORY) DrawMemory(0, 0, MEMORY_PAGE);
    EndDrawing();
}

void Screen::DrawInstructions(int x, int y) {
    std::stringstream ss;
    ss << std::uppercase << std::hex << (SharpSM83::DEBUG_INFO.currentAddr - 4) << " " << SharpSM83::opcodeStr[mPpu->mBus->read(SharpSM83::DEBUG_INFO.currentAddr - 4)];
    DrawText(ss.str().c_str(), x, (y + 0) * 32, 30, WHITE);
    ss.str("");
    ss.clear();
    ss << std::uppercase << std::hex << (SharpSM83::DEBUG_INFO.currentAddr - 3) << " " << SharpSM83::opcodeStr[mPpu->mBus->read(SharpSM83::DEBUG_INFO.currentAddr - 3)];
    DrawText(ss.str().c_str(), x, (y + 1) * 32, 30, WHITE);
    ss.str("");
    ss.clear();
    ss << std::uppercase << std::hex << (SharpSM83::DEBUG_INFO.currentAddr - 2) << " " << SharpSM83::opcodeStr[mPpu->mBus->read(SharpSM83::DEBUG_INFO.currentAddr - 2)];
    DrawText(ss.str().c_str(), x, (y + 2) * 32, 30, WHITE);
    ss.str("");
    ss.clear();
    ss << std::uppercase << std::hex << (SharpSM83::DEBUG_INFO.currentAddr - 1) << " " << SharpSM83::opcodeStr[mPpu->mBus->read(SharpSM83::DEBUG_INFO.currentAddr - 1)];
    DrawText(ss.str().c_str(), x, (y + 3) * 32, 30, WHITE);
    ss.str("");
    ss.clear();
    ss << std::uppercase << std::hex << SharpSM83::DEBUG_INFO.currentAddr << " " << SharpSM83::DEBUG_INFO.currentInstr;
    DrawText(ss.str().c_str(), x, (y + 4) * 32, 30, YELLOW);
    ss.str("");
    ss.clear();
    ss << std::uppercase << std::hex << (SharpSM83::DEBUG_INFO.currentAddr + 1) << " " << SharpSM83::opcodeStr[mPpu->mBus->read(SharpSM83::DEBUG_INFO.currentAddr + 1)];
    DrawText(ss.str().c_str(), x, (y + 5) * 32, 30, WHITE);
    ss.str("");
    ss.clear();
    ss << std::uppercase << std::hex << (SharpSM83::DEBUG_INFO.currentAddr + 2) << " " << SharpSM83::opcodeStr[mPpu->mBus->read(SharpSM83::DEBUG_INFO.currentAddr + 2)];
    DrawText(ss.str().c_str(), x, (y + 6) * 32, 30, WHITE);
    ss.str("");
    ss.clear();
    ss << std::uppercase << std::hex << (SharpSM83::DEBUG_INFO.currentAddr + 3) << " " << SharpSM83::opcodeStr[mPpu->mBus->read(SharpSM83::DEBUG_INFO.currentAddr + 3)];
    DrawText(ss.str().c_str(), x, (y + 7) * 32, 30, WHITE);
    ss.str("");
    ss.clear();
    ss << std::uppercase << std::hex << (SharpSM83::DEBUG_INFO.currentAddr + 4) << " " << SharpSM83::opcodeStr[mPpu->mBus->read(SharpSM83::DEBUG_INFO.currentAddr + 4)];
    DrawText(ss.str().c_str(), x, (y + 8) * 32, 30, WHITE);
}

void Screen::DrawFlags(int x, int y) {
    DrawText("Z", x, y * 32, 30, SharpSM83::DEBUG_INFO.Z ? GREEN : RED);
    DrawText("C", x, (y + 1) * 32, 30, SharpSM83::DEBUG_INFO.C ? GREEN : RED);
    DrawText("N", x, (y + 2) * 32, 30, SharpSM83::DEBUG_INFO.N ? GREEN : RED);
    DrawText("HC", x, (y + 3) * 32, 30, SharpSM83::DEBUG_INFO.HC ? GREEN : RED);
}

void Screen::DrawRegisters(int x, int y) {
    std::stringstream ss;
    ss << " A = " << std::uppercase << std::hex << SharpSM83::DEBUG_INFO.regA;
    DrawText(ss.str().c_str(), x, 0 * 32 + y, 30, WHITE);
    ss.str("");
    ss.clear();
    ss << " B = " << std::uppercase << std::hex << SharpSM83::DEBUG_INFO.regB;
    DrawText(ss.str().c_str(), x, 1 * 32 + y, 30, WHITE);
    ss.str("");
    ss.clear();
    ss << " C = " << std::uppercase << std::hex << SharpSM83::DEBUG_INFO.regC;
    DrawText(ss.str().c_str(), x, 2 * 32 + y, 30, WHITE);
    ss.str("");
    ss.clear();
    ss << " D = " << std::uppercase << std::hex << SharpSM83::DEBUG_INFO.regD;
    DrawText(ss.str().c_str(), x, 3 * 32 + y, 30, WHITE);
    ss.str("");
    ss.clear();
    ss << " E = " << std::uppercase << std::hex << SharpSM83::DEBUG_INFO.regE;
    DrawText(ss.str().c_str(), x, 4 * 32 + y, 30, WHITE);
    ss.str("");
    ss.clear();
    ss << " H = " << std::uppercase << std::hex << SharpSM83::DEBUG_INFO.regH;
    DrawText(ss.str().c_str(), x, 5 * 32 + y, 30, WHITE);
    ss.str("");
    ss.clear();
    ss << " L = " << std::uppercase << std::hex << SharpSM83::DEBUG_INFO.regL;
    DrawText(ss.str().c_str(), x, 6 * 32 + y, 30, WHITE);
    ss.str("");
    ss.clear();
    ss << "SP = " << std::uppercase << std::hex << SharpSM83::DEBUG_INFO.regSP;
    DrawText(ss.str().c_str(), x, 7 * 32 + y, 30, WHITE);
}

void Screen::DrawMemory(int x, int y, int page) {
    std::stringstream ss;
    for (uint8_t line = 0; line < 16; line++) {
        ss << std::uppercase << std::hex << 0xC000 + line * 16 + page * 16 * 16 << ": ";
        for (uint8_t value = 0; value < 16; value++) {
            auto data = (uint16_t)mPpu->mBus->read(0xC000 + line * 16 + value + page * 16 * 16);
            ss << std::uppercase << std::hex << (data < 0x10 ? "0" : "") << data << " ";
        }
        DrawText(ss.str().c_str(), x, line * 28 + y, 26, WHITE);
        ss.str("");
        ss.clear();
    }
}


void Screen::bufferTilesData() {
    for (uint16_t tile = 0; tile < 0x100; tile++) {
        for (uint8_t y = 0; y < 8; y++){
            for (uint8_t x = 0; x < 8; x++) {
                uint8_t pixelID = getPixel(tile, x, y);
                tilesDataPixelArray[(tile / 16 * 8 + y) * 128 + (tile % 16 * 8 + x)] = getBGPPixelFromID(pixelID);
            }
        }
    }
    UpdateTexture(tilesDataTexture, tilesDataPixelArray);
}

uint8_t Screen::getPixel(uint16_t tileIndex, uint8_t tilePixelX, uint8_t tilePixelY) {
    tileDataBlock = mPpu->LCDC.tileDataArea ? 0x8000 : 0x9000;
    uint16_t currentTileBlock = tileIndex < 0x80 ? tileDataBlock : 0x8800;
    uint8_t firstByte = mPpu->read((currentTileBlock + (tileIndex % 128) * 16) + tilePixelY*2);
    uint8_t secondByte = mPpu->read((currentTileBlock + (tileIndex % 128) * 16) + 1 + tilePixelY*2);

    return ((firstByte & 0x1 << (7-tilePixelX)) >> (7-tilePixelX)) | ((secondByte & 0x1 << (7-tilePixelX)) >> (7-tilePixelX) << 1);
}

uint8_t Screen::getPixelObj(uint16_t tileIndex, uint8_t tilePixelX, uint8_t tilePixelY) {
    tileDataBlock = 0x8000;
    /*if (mPpu->LCDC.objSize) {
        if (tilePixelY < 8) tileIndex &= 0xFE;
        else tileIndex |= 0x01;
    }*/
    uint16_t currentTileBlock = tileIndex < 0x80 ? tileDataBlock : 0x8800;
    uint8_t firstByte = mPpu->read((currentTileBlock + (tileIndex % 128) * 16) + tilePixelY*2);
    uint8_t secondByte = mPpu->read((currentTileBlock + (tileIndex % 128) * 16) + 1 + tilePixelY*2);

    return ((firstByte & 0x1 << (7-tilePixelX)) >> (7-tilePixelX)) | ((secondByte & 0x1 << (7-tilePixelX)) >> (7-tilePixelX) << 1);
}

Screen::Pixel Screen::getBackgroundPixelAt(uint8_t x, uint8_t y) {
    uint16_t tileID = (y / 8) * 32 + (x / 8);
    uint8_t pixelID = getPixel(mPpu->read(0x9800 + tileID), x % 8, y % 8);

    return getBGPPixelFromID(pixelID);
}

Screen::Pixel Screen::getWindowPixelAt(uint8_t x, uint8_t y) {
    uint16_t tileID = (y / 8) * 32 + (x / 8);
    uint8_t pixelID = getPixel(mPpu->read(0x9C00 + tileID), x % 8, y % 8);

    return getBGPPixelFromID(pixelID);
}

void Screen::setObjects() {
    for (uint8_t o = 0; o < 40; o++) { // There are 40 objects in the OAM (Object Attribute Memory)
        Object obj{};
        obj.Ypos = mPpu->read(0xFE00 + (o * 4) + 0);
        obj.Xpos = mPpu->read(0xFE00 + (o * 4) + 1);
        obj.tileIndex = mPpu->read(0xFE00 + (o * 4) + 2);
        obj.attributes = mPpu->read(0xFE00 + (o * 4) + 3);
        obj.isReal = true;
        objects[o] = obj;
    }
}

static bool generateData = true;
void Screen::tick(uint8_t mCycle) {
    if (generateData) {
        generateData = false;
        setObjects();
        bufferTilesData();
    }

    for (uint8_t i = 0; i < mCycle * 4; i++) { // NOTE: 4 is correct only on normal speed
        if (dots < 80 && mPpu->STAT.modeOAM) SharpSM83::IF.lcd = 1;

        if (dots == 4) {
            mPpu->LY = yPos;
            mPpu->STAT.LYCequalLY = (mPpu->LY == mPpu->LYC);
            if (mPpu->STAT.modeLYCequalLY && mPpu->STAT.LYCequalLY) SharpSM83::IF.lcd = 1;
            if (yPos == 144 && (mPpu->STAT.lcdMode == 0b01)) { SharpSM83::IF.vblank = 1; }
        }

        if (dots < 80) {
            dots++;
            mPpu->STAT.lcdMode = 0b10;
        }
        else {
            if (xPos < DEFAULT_WIDTH && yPos < DEFAULT_HEIGHT) {
                mPpu->STAT.lcdMode = 0b11;
                std::array<Object, 10> objs{};
                getObjectToRender(objs, yPos);

                // First drawn layer is the background
                if (mPpu->LCDC.bgWinEnable) {
                    if (mPpu->LCDC.bgTileMap)
                        screenPixelArray[yPos * DEFAULT_WIDTH + xPos] = getWindowPixelAt(mPpu->SCX + xPos,
                                                                                         (mPpu->SCY + yPos) % 254);
                    else
                        screenPixelArray[yPos * DEFAULT_WIDTH + xPos] = getBackgroundPixelAt(mPpu->SCX + xPos,
                                                                                             (mPpu->SCY + yPos) % 254);

                    // Second drawn layer is the window
                    if (mPpu->LCDC.windowEnable && xPos >= mPpu->WX - 7 && yPos >= mPpu->WY) {
                        if (mPpu->LCDC.windowTileMap)
                            screenPixelArray[yPos * DEFAULT_WIDTH + xPos] = getWindowPixelAt(xPos - (mPpu->WX - 7),
                                                                                             yPos - mPpu->WY);
                        else
                            screenPixelArray[yPos * DEFAULT_WIDTH + xPos] = getBackgroundPixelAt(xPos - (mPpu->WX - 7),
                                                                                                 yPos - mPpu->WY);
                    }
                }

                // Third draw layer is the objects
                if (mPpu->LCDC.objEnable) {
                    for (Object &obj: objs) {
                        if (obj.isReal) {
                            if (xPos >= (obj.Xpos - 8) && xPos <= obj.Xpos - 1) {
                                uint8_t pixelID;
                                uint8_t objectPixelX = xPos - (obj.Xpos - 8);
                                uint8_t objectPixelY = yPos - (obj.Ypos - 16);

                                if (obj.xFlip) objectPixelX = abs(7 - (xPos - (obj.Xpos - 8)));
                                if (obj.yFlip) objectPixelY = abs(7 - (yPos - (obj.Ypos - (mPpu->LCDC.objSize ? 8 : 16))));

                                pixelID = getPixelObj(obj.tileIndex, objectPixelX, objectPixelY);

                                if (pixelID != 0x00) {
                                    if (obj.priority) screenPixelArray[yPos * DEFAULT_WIDTH + xPos] = getBGPPixelFromID(pixelID);
                                    else screenPixelArray[yPos * DEFAULT_WIDTH + xPos] = getOBPPixelFromID(pixelID,obj.dmgPalette);
                                }
                            }
                        }
                    }
                }
            } else {
                if (yPos >= DEFAULT_HEIGHT) { mPpu->STAT.lcdMode = 0b01; }
                else mPpu->STAT.lcdMode = 0b00;
            }
            xPos++;
            dots++;

            if (dots >= 376 && mPpu->STAT.modeHBLANK) SharpSM83::IF.lcd = 1; //TODO: This should be dots - mode 3's taken dots. https://gbdev.io/pandocs/Rendering.html#mode-3-length

            if (dots == 456) {
                dots = 0x00;
                yPos++;
                xPos = 0x00;
            }
            if (yPos >= DEFAULT_HEIGHT && yPos <= DEFAULT_HEIGHT + 9 && mPpu->STAT.modeVBLANK) SharpSM83::IF.lcd = 1;
            if (yPos >= DEFAULT_HEIGHT + 9) {
                yPos = 0x00;
                generateData = true;
                UpdateTexture(gameTexture, screenPixelArray);
            }
        }
    }
}

void Screen::getObjectToRender(std::array<Object, 10> &out, uint8_t currentY) {
    uint8_t objCount = 0;
    for (Object obj : objects) {

        if (obj.isReal) {
            if (mPpu->LCDC.objSize) { // 8x16 objects mode
                if (obj.Ypos >= 1 && obj.Ypos < 160) {
                    if (currentY >= obj.Ypos - 16 && currentY < obj.Ypos) {
                        out[objCount] = obj;
                        objCount++;
                    }
                }
            } else { // 8x8 objects mode
                if (obj.Ypos >= 16 && obj.Ypos < 160) {
                    if (currentY >= obj.Ypos - 16 && currentY < obj.Ypos - 8) {
                        out[objCount] = obj;
                        objCount++;
                    }
                }
            }
        }
        if (objCount == 10) break;
    }
}

Screen::Pixel Screen::getBGPPixelFromID(uint8_t pixelID) const {
    Pixel pixel{};
    if (((mPpu->BGP.raw >> pixelID * 2) & 0x03) == 0b11) {
        pixel.r = 0x00;
        pixel.g = 0x00;
        pixel.b = 0x00;
    }
    else if (((mPpu->BGP.raw >> pixelID * 2) & 0x03) == 0b10) {
        pixel.r = 0x55;
        pixel.g = 0x55;
        pixel.b = 0x55;
    }
    else if (((mPpu->BGP.raw >> pixelID * 2) & 0x03) == 0b01) {
        pixel.r = 0xAA;
        pixel.g = 0xAA;
        pixel.b = 0xAA;
    }
    else if (((mPpu->BGP.raw >> pixelID * 2) & 0x03) == 0b00) {
        pixel.r = 0xFF;
        pixel.g = 0xFF;
        pixel.b = 0xFF;
    }

    return pixel;
}

Screen::Pixel Screen::getOBPPixelFromID(uint8_t pixelID, bool palette) const {
    Pixel pixel {};
    if (palette) {
        if (((mPpu->OBP1.raw >> pixelID * 2) & 0x03) == 0b11) {
            pixel.r = 0x00;
            pixel.g = 0x00;
            pixel.b = 0x00;
        }
        else if (((mPpu->OBP1.raw >> pixelID * 2) & 0x03) == 0b10) {
            pixel.r = 0x55;
            pixel.g = 0x55;
            pixel.b = 0x55;
        }
        else if (((mPpu->OBP1.raw >> pixelID * 2) & 0x03) == 0b01) {
            pixel.r = 0xAA;
            pixel.g = 0xAA;
            pixel.b = 0xAA;
        }
        else if (((mPpu->OBP1.raw >> pixelID * 2) & 0x03) == 0b00) {
            pixel.r = 0xFF;
            pixel.g = 0xFF;
            pixel.b = 0xFF;
        }
    } else {
        if (((mPpu->OBP0.raw >> pixelID * 2) & 0x03) == 0b11) {
            pixel.r = 0x00;
            pixel.g = 0x00;
            pixel.b = 0x00;
        }
        else if (((mPpu->OBP0.raw >> pixelID * 2) & 0x03) == 0b10) {
            pixel.r = 0x55;
            pixel.g = 0x55;
            pixel.b = 0x55;
        }
        else if (((mPpu->OBP0.raw >> pixelID * 2) & 0x03) == 0b01) {
            pixel.r = 0xAA;
            pixel.g = 0xAA;
            pixel.b = 0xAA;
        }
        else if (((mPpu->OBP0.raw >> pixelID * 2) & 0x03) == 0b00) {
            pixel.r = 0xFF;
            pixel.g = 0xFF;
            pixel.b = 0xFF;
        }
    }

    return pixel;
}

void Screen::reset() {
    screenPixelArray = new Pixel[DEFAULT_WIDTH * DEFAULT_HEIGHT];
    tilesDataPixelArray  = new Pixel[16 * 8 * 16 * 8]; // There are 256 tiles to render, so a 16x16 square is sufficient, but a tile is 8x8 pixels
}

void Screen::close() {
    UnloadTexture(gameTexture);
    UnloadTexture(tilesDataTexture);
    CloseWindow();
}
