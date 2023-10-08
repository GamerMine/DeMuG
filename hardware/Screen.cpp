#include "Screen.h"

Screen::Screen(class Ppu *ppu) {
    mPpu = ppu;
}

void Screen::operator()() {
    Logger *logger = Logger::getInstance(LOGGER_NAME);

    InitWindow(1280, 720, "Emulateur GameBoy");

    screen = new Pixel[DEFAULT_WIDTH * DEFAULT_HEIGHT];
    tiles  = new Pixel[16 * 8 * 16 * 8]; /*There are 256 tiles to render, so a 16x16 square is sufficient, but a tile is 8x8 pixels*/

    Image gameRender = {.data = screen, .width = DEFAULT_WIDTH, .height = DEFAULT_HEIGHT, .mipmaps = 1, .format = PIXELFORMAT_UNCOMPRESSED_R8G8B8};
    Image tilesRender = {.data = tiles, .width = 16 * 8, .height = 16 * 8, .mipmaps = 1, .format = PIXELFORMAT_UNCOMPRESSED_R8G8B8};
    gameTexture = LoadTextureFromImage(gameRender);
    tilesTexture = LoadTextureFromImage(tilesRender);

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
    UnloadTexture(tilesTexture);
    CloseWindow();
}

uint16_t tileDataBlock = 0x8000;
void Screen::render() {
    if (mPpu->LCDC.lcdEnable) {
        mPpu->LY = mPpu->LY + 1;
        if (mPpu->LY >= 153) mPpu->LY = 0x00;

        renderTiles();

        BeginDrawing();
        ClearBackground(DARKBLUE);
        DrawTexturePro(gameTexture,
                       (Rectangle){0, 0, static_cast<float>(gameTexture.width), static_cast<float>(gameTexture.height)},
                       (Rectangle){0, 0, static_cast<float>(gameTexture.width * 5), static_cast<float>(gameTexture.height * 5)},
                       (Vector2){0, 0},
                       0,
                       RAYWHITE);
        DrawTexturePro(tilesTexture,
                       (Rectangle){0, 0, static_cast<float>(tilesTexture.width), static_cast<float>(tilesTexture.height)},
                       (Rectangle){805, 0, static_cast<float>(tilesTexture.width * 3), static_cast<float>(tilesTexture.height * 3)},
                       (Vector2){0, 0},
                       0,
                       RAYWHITE);
        EndDrawing();
        printf("");
    }
}

void Screen::renderTiles() {
    tileDataBlock = mPpu->LCDC.tileDataArea ? 0x8000 : 0x9000;
    for (uint8_t tile = 0x00; tile < 0xFF; tile++) { // There are 255 tiles
        uint16_t currentTileBlock = tile < 0x80 ? tileDataBlock : 0x8800;
        for (uint8_t byte = 0; byte < 8; byte++) { // Each tile is 16 bytes long, but 8 because one line is 2 bytes long
            uint8_t firstByte = mPpu->read((currentTileBlock + (tile % 128) * 16) + byte*2);
            uint8_t secondByte = mPpu->read((currentTileBlock + (tile % 128) * 16) + 1 + byte*2);
            for (uint8_t pixel = 0; pixel < 8; pixel++) {
                uint8_t pixelID = ((firstByte & 0x1 << (7-pixel)) >> (7-pixel)) | ((secondByte & 0x1 << (7-pixel)) >> (7-pixel) << 1);
                if (pixelID == mPpu->BGP.index3) {
                    tiles[((tile+1) / 16 * 8 + byte) * 128 + (tile % 16 * 8 + pixel)] = Pixel(0x00, 0x00, 0x00);
                }
                else if (pixelID == mPpu->BGP.index2) {
                    tiles[((tile+1) / 16 * 8 + byte) * 128 + (tile % 16 * 8 + pixel)] = Pixel(0x55, 0x55, 0x55);
                }
                else if (pixelID == mPpu->BGP.index1) {
                    tiles[((tile+1) / 16 * 8 + byte) * 128 + (tile % 16 * 8 + pixel)] = Pixel(0xAA, 0xAA, 0xAA);
                }
                else if (pixelID == mPpu->BGP.index0) {
                    tiles[((tile+1) / 16 * 8 + byte) * 128 + (tile % 16 * 8 + pixel)] = Pixel(0xFF, 0xFF, 0xFF);
                }
            }
        }
    }
    UpdateTexture(tilesTexture, tiles);
}