#include "Screen.h"

Screen::Screen(class Ppu *ppu) {
    mPpu = ppu;
}

void Screen::operator()() {
    InitWindow(1280, 720, WINDOW_NAME);
    SetWindowMonitor(0);

    screenPixelArray = new Pixel[DEFAULT_WIDTH * DEFAULT_HEIGHT];
    tilesDataPixelArray  = new Pixel[16 * 8 * 16 * 8]; // There are 256 tiles to render, so a 16x16 square is sufficient, but a tile is 8x8 pixels
    backgroundMapPixelArray = new Pixel[32 * 8 * 32 * 8];  // There are 1024 tiles to render, so a 32x32 square is sufficient, but a tile is 8x8 pixels
    windowMapPixelArray = new Pixel[32 * 8 * 32 * 8];  // There are 1024 tiles to render, so a 32x32 square is sufficient, but a tile is 8x8 pixels

    // Initialize screen pixel array
    for (long i = 0; i < DEFAULT_WIDTH*DEFAULT_HEIGHT; i++) screenPixelArray[i] = Pixel(0x00, 0x00, 0x00);

    // Initialize tileset pixel array
    for (long i = 0; i < 16*8*16*8; i++) tilesDataPixelArray[i] = Pixel(0x00, 0x00, 0x00);

    // Initialize background map pixel array
    for (long i = 0; i < 32 * 8 * 32 * 8; i++) backgroundMapPixelArray[i] = Pixel(0x00, 0x00, 0x00);

    // Initialize window map pixel array
    for (long i = 0; i < 32 * 8 * 32 * 8; i++) windowMapPixelArray[i] = Pixel(0x00, 0x00, 0x00);

    Image gameRender = {.data = screenPixelArray, .width = DEFAULT_WIDTH, .height = DEFAULT_HEIGHT, .mipmaps = 1, .format = PIXELFORMAT_UNCOMPRESSED_R8G8B8};
    Image tilesDataRender = {.data = tilesDataPixelArray, .width = 16 * 8, .height = 16 * 8, .mipmaps = 1, .format = PIXELFORMAT_UNCOMPRESSED_R8G8B8};
    Image backgroundMapRender = {.data = backgroundMapPixelArray, .width = 32 * 8, .height = 32 * 8, .mipmaps = 1, .format = PIXELFORMAT_UNCOMPRESSED_R8G8B8};
    Image windowMapRender = {.data = windowMapPixelArray, .width = 32 * 8, .height = 32 * 8, .mipmaps = 1, .format = PIXELFORMAT_UNCOMPRESSED_R8G8B8};
    gameTexture = LoadTextureFromImage(gameRender);
    tilesDataTexture = LoadTextureFromImage(tilesDataRender);
    backgroundMapTexture = LoadTextureFromImage(backgroundMapRender);
    windowMapTexture = LoadTextureFromImage(windowMapRender);

    double currentTime;
    double lastTime;

    while (!WindowShouldClose()) {
        currentTime = GetTime();

        // Render buffering
        // ----------------------------------------------------------------
        setTileData();
        setObjects();
        generateBackgroundTileMap();
        generateWindowTileMap();
        if (mPpu->bufferScreen) bufferScreen();

        // Render
        // ----------------------------------------------------------------
        if (currentTime - lastTime >= 1.0 / FRAMERATE) {
            lastTime = currentTime;
            render();
        }
    }

    UnloadTexture(gameTexture);
    UnloadTexture(tilesDataTexture);
    UnloadTexture(backgroundMapTexture);
    UnloadTexture(windowMapTexture);
    Bus::GLOBAL_HALT = true;
    CloseWindow();
}

uint16_t tileDataBlock = 0x8000;
bool test = true;
void Screen::render() {
    if (mPpu->LCDC.lcdEnable) {
        renderTilesData();

        BeginDrawing();
        ClearBackground(DARKBLUE);
        DrawTexturePro(gameTexture,
                       (Rectangle){0, 0, static_cast<float>(gameTexture.width), static_cast<float>(gameTexture.height)},
                       (Rectangle){0, 0, static_cast<float>(gameTexture.width * 5), static_cast<float>(gameTexture.height * 5)},
                       (Vector2){0, 0},
                       0,
                       RAYWHITE);
        DrawTexturePro(tilesDataTexture,
                       (Rectangle){0, 0, static_cast<float>(tilesDataTexture.width), static_cast<float>(tilesDataTexture.height)},
                       (Rectangle){810, static_cast<float>(GetRenderHeight() - tilesDataTexture.height * 2 - 10), static_cast<float>(tilesDataTexture.width * 2), static_cast<float>(tilesDataTexture.height * 2)},
                       (Vector2){0, 0},
                       0,
                       RAYWHITE);
        /*DrawTexturePro(backgroundMapTexture,
                       (Rectangle){0, 0, static_cast<float>(backgroundMapTexture.width), static_cast<float>(backgroundMapTexture.height)},
                       (Rectangle){820, 0, static_cast<float>(backgroundMapTexture.width), static_cast<float>(backgroundMapTexture.height)},
                       (Vector2){0, 0},
                       0,
                       RAYWHITE);*/
        /*DrawTexturePro(windowMapTexture,
                       (Rectangle){0, 0, static_cast<float>(windowMapTexture.width), static_cast<float>(windowMapTexture.height)},
                       (Rectangle){820, 0, static_cast<float>(windowMapTexture.width), static_cast<float>(windowMapTexture.height)},
                       (Vector2){0, 0},
                       0,
                       RAYWHITE);*/
        DrawInstructions(820, 0);
        DrawFlags(1100, 0);
        EndDrawing();
    }
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


void Screen::renderTilesData() {
    for (uint8_t tile = 0; tile < 0xFF; tile++) {
        for (uint8_t y = 0; y < 8; y++){
            for (uint8_t x = 0; x < 8; x++) {
                uint8_t pixelID = tilesData[tile][y][x];
                if (pixelID == mPpu->BGP.index3) tilesDataPixelArray[(tile / 16 * 8 + y) * 128 + (tile % 16 * 8 + x)] = Pixel(0x00, 0x00, 0x00);
                else if (pixelID == mPpu->BGP.index2) tilesDataPixelArray[(tile / 16 * 8 + y) * 128 + (tile % 16 * 8 + x)] = Pixel(0x55, 0x55, 0x55);
                else if (pixelID == mPpu->BGP.index1) tilesDataPixelArray[(tile / 16 * 8 + y) * 128 + (tile % 16 * 8 + x)] = Pixel(0xAA, 0xAA, 0xAA);
                else if (pixelID == mPpu->BGP.index0) tilesDataPixelArray[(tile / 16 * 8 + y) * 128 + (tile % 16 * 8 + x)] = Pixel(0xFF, 0xFF, 0xFF);
            }
        }
    }
    UpdateTexture(tilesDataTexture, tilesDataPixelArray);
}

void Screen::setTileData() {
    tileDataBlock = mPpu->LCDC.tileDataArea ? 0x8000 : 0x9000;
    for (uint8_t tile = 0x00; tile < 0xFF; tile++) { // There are 255 tilesDataPixelArray
        uint16_t currentTileBlock = tile < 0x80 ? tileDataBlock : 0x8800;
        for (uint8_t byte = 0; byte < 8; byte++) { // Each tile is 16 bytes long, but 8 because one line is 2 bytes long
            uint8_t firstByte = mPpu->read((currentTileBlock + (tile % 128) * 16) + byte*2);
            uint8_t secondByte = mPpu->read((currentTileBlock + (tile % 128) * 16) + 1 + byte*2);
            for (uint8_t pixel = 0; pixel < 8; pixel++) {
                uint8_t pixelID = ((firstByte & 0x1 << (7-pixel)) >> (7-pixel)) | ((secondByte & 0x1 << (7-pixel)) >> (7-pixel) << 1);
                tilesData[tile][byte][pixel] = pixelID;
            }
        }
    }
}

void Screen::setObjects() {
    for (uint8_t o = 0; o < 40; o++) { // There are 40 objects in the OAM (Object Attribute Memory)
        Object obj{};
        obj.Ypos = mPpu->read(0xFE00 + (o * 4) + 0);
        obj.Xpos = mPpu->read(0xFE00 + (o * 4) + 1);
        obj.tileIndex = mPpu->read(0xFE00 + (o * 4) + 2);
        obj.attributes = mPpu->read(0xFE00 + (o * 4) + 3);
        objects[o] = obj;
    }
}

void Screen::generateBackgroundTileMap() {
    for (uint16_t value = 0; value < 1024; value++) {
        for (uint8_t y = 0; y < 8; y++) {
            for (uint8_t x = 0; x < 8; x++) {
                uint8_t pixelID = tilesData[mPpu->read(0x9800 + value)][y][x];
                if (pixelID == mPpu->BGP.index3) backgroundMapPixelArray[((value / 32) * 8 + y) * 32*8 + (value % 32 * 8 + x)] = Pixel(0x00, 0x00, 0x00);
                else if (pixelID == mPpu->BGP.index2) backgroundMapPixelArray[((value / 32) * 8 + y) * 32*8 + (value % 32 * 8 + x)] = Pixel(0x55, 0x55, 0x55);
                else if (pixelID == mPpu->BGP.index1) backgroundMapPixelArray[((value / 32) * 8 + y) * 32*8 + (value % 32 * 8 + x)] = Pixel(0xAA, 0xAA, 0xAA);
                else if (pixelID == mPpu->BGP.index0) backgroundMapPixelArray[((value / 32) * 8 + y) * 32*8 + (value % 32 * 8 + x)] = Pixel(0xFF, 0xFF, 0xFF);
            }
        }
    }
    UpdateTexture(backgroundMapTexture, backgroundMapPixelArray);
}

void Screen::generateWindowTileMap() {
    for (uint16_t value = 0; value < 1024; value++) {
        for (uint8_t y = 0; y < 8; y++) {
            for (uint8_t x = 0; x < 8; x++) {
                uint8_t pixelID = tilesData[mPpu->read(0x9C00 + value)][y][x];
                if (pixelID == mPpu->BGP.index3) windowMapPixelArray[((value / 32) * 8 + y) * 32*8 + (value % 32 * 8 + x)] = Pixel(0x00, 0x00, 0x00);
                else if (pixelID == mPpu->BGP.index2) windowMapPixelArray[((value / 32) * 8 + y) * 32*8 + (value % 32 * 8 + x)] = Pixel(0x55, 0x55, 0x55);
                else if (pixelID == mPpu->BGP.index1) windowMapPixelArray[((value / 32) * 8 + y) * 32*8 + (value % 32 * 8 + x)] = Pixel(0xAA, 0xAA, 0xAA);
                else if (pixelID == mPpu->BGP.index0) windowMapPixelArray[((value / 32) * 8 + y) * 32*8 + (value % 32 * 8 + x)] = Pixel(0xFF, 0xFF, 0xFF);
            }
        }
    }
    UpdateTexture(windowMapTexture, windowMapPixelArray);
}

void Screen::bufferScreen() {
    mPpu->LY = 0x00;
    for (uint8_t y = 0; y < DEFAULT_HEIGHT + 9; y++) { // 9 the number of vertical blanking scanlines
        std::array<Object, 10> objs = getObjectToRender(y);
        for (uint8_t x = 0; x < DEFAULT_WIDTH; x++) {
            if (y < DEFAULT_HEIGHT) {
                screenPixelArray[y * DEFAULT_WIDTH + x] = backgroundMapPixelArray[(mPpu->SCY + y) * 32 * 8 +
                                                                             (mPpu->SCX + x)];
                if (mPpu->LCDC.windowEnable) {
                    screenPixelArray[y * DEFAULT_WIDTH + x] = windowMapPixelArray[y * 32 * 8 + x];
                }
            }
            if (y == 144) SharpSM83::IF.vblank = 1;
        }
        mPpu->LY = mPpu->LY + 1;
    }
    UpdateTexture(gameTexture, screenPixelArray);
    mPpu->bufferScreen = false;
}

std::array<Screen::Object, 10> Screen::getObjectToRender(uint8_t currentY) {
    std::array<Screen::Object, 10> objs{};

    uint8_t objCount = 0;
    for (Object &obj : objects) {
        if (obj.Ypos >= 16 && obj.Ypos < 160) { // The Ypos is actually stored as currentY coord + 16, since we cannot draw out of the screen, do not get them
            if (currentY >= obj.Ypos - 16 && currentY <= obj.Ypos - (mPpu->LCDC.objSize ? 0 : 8)) {
                objCount++;
                objs[objCount - 1] = obj;
            }
        }
        if (objCount == 10) break;
    }
}

void Screen::reset() {
    screenPixelArray = new Pixel[DEFAULT_WIDTH * DEFAULT_HEIGHT];
    tilesDataPixelArray  = new Pixel[16 * 8 * 16 * 8]; // There are 256 tiles to render, so a 16x16 square is sufficient, but a tile is 8x8 pixels
    backgroundMapPixelArray = new Pixel[32 * 8 * 32 * 8];  // There are 1024 tiles to render, so a 32x32 square is sufficient, but a tile is 8x8 pixels
    windowMapPixelArray = new Pixel[32 * 8 * 32 * 8];  // There are 1024 tiles to render, so a 32x32 square is sufficient, but a tile is 8x8 pixels
}
