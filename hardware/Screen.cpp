#include "Screen.h"

#pragma clang diagnostic push
#pragma ide diagnostic ignored "NullDereference"
Screen::Screen(class Ppu *ppu) {
    mPpu = ppu;
}

void Screen::operator()() {
    InitWindow(1280, 720, WINDOW_NAME);

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
    Image tilesMapRender = {.data = tilesMapPixelArray, .width = 32 * 8, .height = 32 * 8, .mipmaps = 1, .format = PIXELFORMAT_UNCOMPRESSED_R8G8B8};
    gameTexture = LoadTextureFromImage(gameRender);
    tilesDataTexture = LoadTextureFromImage(tilesDataRender);
    tilesMapTexture = LoadTextureFromImage(tilesMapRender);

    double currentTime;
    double lastTime;

    using namespace std::chrono_literals;
    while (!WindowShouldClose()) {
        // Rendering commands
        currentTime = GetTime();

        if (currentTime - lastTime >= 1.0 / FRAMERATE) {
            lastTime = currentTime;
            render();
        }

        PollInputEvents();
    }

    UnloadTexture(gameTexture);
    UnloadTexture(tilesDataTexture);
    UnloadTexture(tilesMapTexture);
    CloseWindow();
}

uint16_t tileDataBlock = 0x8000;
bool test = true;
void Screen::render() {
    if (mPpu->LCDC.lcdEnable) {
        mPpu->LY = mPpu->LY + 1;
        if (mPpu->LY >= 153) mPpu->LY = 0x00;

        setTileData();
        renderTilesData();
        renderTilesMap();

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
                       (Rectangle){820, 320, static_cast<float>(tilesDataTexture.width * 3), static_cast<float>(tilesDataTexture.height * 3)},
                       (Vector2){0, 0},
                       0,
                       RAYWHITE);
        DrawTexturePro(tilesMapTexture,
                       (Rectangle){0, 0, static_cast<float>(tilesMapTexture.width), static_cast<float>(tilesMapTexture.height)},
                       (Rectangle){820, 0, static_cast<float>(tilesMapTexture.width), static_cast<float>(tilesMapTexture.height)},
                       (Vector2){0, 0},
                       0,
                       RAYWHITE);
        EndDrawing();
    }
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

void Screen::renderTilesMap() {
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

#pragma clang diagnostic pop