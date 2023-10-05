#include "Screen.h"

Screen::Screen(class Ppu *ppu) {
    mPpu = ppu;
}

void Screen::operator()() {
    Logger *logger = Logger::getInstance(LOGGER_NAME);

    if (!glfwInit()) {
        logger->log(Logger::CRITICAL, "%s", "Error: Unable to initialize GLFW");
        exit(1);
    }

    window = glfwCreateWindow(DEFAULT_WIDTH, DEFAULT_HEIGHT, WINDOW_NAME, nullptr, nullptr);
    if (!window) {
        glfwTerminate();
        logger->log(Logger::CRITICAL, "%s", "Error: Unable to create GLFW window");
        exit(1);
    }

    glfwMakeContextCurrent(window);
    glViewport(0, 0, DEFAULT_WIDTH, DEFAULT_HEIGHT);

    double currentTime;
    double lastTime;

    using namespace std::chrono_literals;
    while (!glfwWindowShouldClose(window)) {
        // Rendering commands
        currentTime = glfwGetTime();

        if (currentTime - lastTime >= 1.0 / FRAMERATE) {
            lastTime = currentTime;
            render();
        }

        glfwPollEvents();
        glfwSwapBuffers(window);
    }

    glfwTerminate();
}

uint16_t primaryTileDataBlock = 0x8000;
void Screen::render() {
    if (mPpu->LCDC.lcdEnable) {
        glClearColor(0xFF, 0xFF, 0xFF, 0xFF);

        mPpu->LY = mPpu->LY + 1;
        if (mPpu->LY >= 153) mPpu->LY = 0x00;
        primaryTileDataBlock = mPpu->LCDC.tileDataArea ? 0x8000 : 0x9000;
        for (uint8_t i = 0x00; i < 0xFF; i++) {
            if (i == 0x80) primaryTileDataBlock = 0x8800;
            for (uint8_t j = 0; j < 16; j++) {
                //mPpu->vram[(primaryTileDataBlock - 0x80) + i]
            }
        }
    } else {
        glClearColor(0x00, 0x00, 0x00, 0xFF);
    }
    glClear(GL_COLOR_BUFFER_BIT);
}
