#include "Screen.h"

Screen::Screen(class Ppu *ppu) {
    mPpu = ppu;
}

void Screen::operator()() {
    InitWindow(1280, 720, WINDOW_NAME);
    SetWindowMonitor(0);

    screenPixelArray = new Pixel[DEFAULT_WIDTH * DEFAULT_HEIGHT];
    tilesDataPixelArray  = new Pixel[16 * 8 * 16 * 8]; // There are 256 tiles to render, so a 16x16 square is sufficient, but a tile is 8x8 pixels
    tilesMapPixelArray = new Pixel[32 * 8 * 32 * 8];  // There are 1024 tiles to render, so a 32x32 square is sufficient, but a tile is 8x8 pixels

    // Initialize screen pixel array
    for (long i = 0; i < DEFAULT_WIDTH*DEFAULT_HEIGHT; i++) screenPixelArray[i] = Pixel(0x00, 0x00, 0x00);

    // Initialize tileset pixel array
    for (long i = 0; i < 16*8*16*8; i++) tilesDataPixelArray[i] = Pixel(0x00, 0x00, 0x00);

    // Initialize tile map pixel array
    for (long i = 0; i < 32 * 8 * 32 * 8; i++) tilesMapPixelArray[i] = Pixel(0x00, 0x00, 0x00);

    Image gameRender = {.data = screenPixelArray, .width = DEFAULT_WIDTH, .height = DEFAULT_HEIGHT, .mipmaps = 1, .format = PIXELFORMAT_UNCOMPRESSED_R8G8B8};
    Image tilesDataRender = {.data = tilesDataPixelArray, .width = 16 * 8, .height = 16 * 8, .mipmaps = 1, .format = PIXELFORMAT_UNCOMPRESSED_R8G8B8};
    Image tileMapRender = {.data = tilesMapPixelArray, .width = 32 * 8, .height = 32 * 8, .mipmaps = 1, .format = PIXELFORMAT_UNCOMPRESSED_R8G8B8};
    gameTexture = LoadTextureFromImage(gameRender);
    tilesDataTexture = LoadTextureFromImage(tilesDataRender);
    tilesMapTexture = LoadTextureFromImage(tileMapRender);

    double currentTime;
    double lastTime;

    while (!WindowShouldClose()) {
        currentTime = GetTime();

        // Render buffering
        // ----------------------------------------------------------------
        setTileData();
        generateTileMap1();
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
    UnloadTexture(tilesMapTexture);
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
        /*DrawTexturePro(tilesMapTexture,
                       (Rectangle){0, 0, static_cast<float>(tilesMapTexture.width), static_cast<float>(tilesMapTexture.height)},
                       (Rectangle){820, 0, static_cast<float>(tilesMapTexture.width), static_cast<float>(tilesMapTexture.height)},
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

void Screen::generateTileMap1() {
    for (uint16_t value = 0; value < 1024; value++) {
        for (uint8_t y = 0; y < 8; y++) {
            for (uint8_t x = 0; x < 8; x++) {
                uint8_t pixelID = tilesData[mPpu->read(0x9800 + value)][y][x];
                if (pixelID == mPpu->BGP.index3) tilesMapPixelArray[((value / 32) * 8 + y) * 32*8 + (value % 32 * 8 + x)] = Pixel(0x00, 0x00, 0x00);
                else if (pixelID == mPpu->BGP.index2) tilesMapPixelArray[((value / 32) * 8 + y) * 32*8 + (value % 32 * 8 + x)] = Pixel(0x55, 0x55, 0x55);
                else if (pixelID == mPpu->BGP.index1) tilesMapPixelArray[((value / 32) * 8 + y) * 32*8 + (value % 32 * 8 + x)] = Pixel(0xAA, 0xAA, 0xAA);
                else if (pixelID == mPpu->BGP.index0) tilesMapPixelArray[((value / 32) * 8 + y) * 32*8 + (value % 32 * 8 + x)] = Pixel(0xFF, 0xFF, 0xFF);
            }
        }
    }
    UpdateTexture(tilesMapTexture, tilesMapPixelArray);
}

void Screen::bufferScreen() {
    mPpu->LY = 0x00;
    for (uint8_t y = 0; y < DEFAULT_HEIGHT + 9; y++) { // 9 the number of vertical blanking scanlines
        for (uint8_t x = 0; x < DEFAULT_WIDTH; x++) {
            if (y < DEFAULT_HEIGHT) {
                screenPixelArray[y * DEFAULT_WIDTH + x] = tilesMapPixelArray[(mPpu->SCY + y) * 32 * 8 +
                                                                             (mPpu->SCX + x)];
            }
        }
        mPpu->LY = mPpu->LY + 1;
    }
    UpdateTexture(gameTexture, screenPixelArray);
    mPpu->bufferScreen = false;
}

void Screen::reset() {
    screenPixelArray = new Pixel[DEFAULT_WIDTH * DEFAULT_HEIGHT];
    tilesDataPixelArray  = new Pixel[16 * 8 * 16 * 8]; // There are 256 tiles to render, so a 16x16 square is sufficient, but a tile is 8x8 pixels
    tilesMapPixelArray = new Pixel[32 * 8 * 32 * 8];  // There are 1024 tiles to render, so a 32x32 square is sufficient, but a tile is 8x8 pixels
}
