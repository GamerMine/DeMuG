#include "Screen.h"

Screen::Screen() {
    GLFWwindow* window;
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

    while (!glfwWindowShouldClose(window)) {

    }

    glfwTerminate();
}
