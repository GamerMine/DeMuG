#include "Screen.h"

Logger *Logger::instance;

Screen::Screen() {
    GLFWwindow* window;

    Logger::getInstance()->log(Logger::DEBUG, "", "%s %s %s", "COUCOU", "CA", "FONCTIONNE");

    if (!glfwInit()) {
        Logger::getInstance()->log(Logger::CRITICAL, "", "%s", "Error: Unable to initialize GLFW");
        exit(1);
    }

    window = glfwCreateWindow(DEFAULT_WIDTH, DEFAULT_HEIGHT, WINDOW_NAME, nullptr, nullptr);
    if (!window) {
        glfwTerminate();
        Logger::getInstance()->log(Logger::CRITICAL, "", "%s", "Error: Unable to create GLFW window");
        exit(1);
    }

    glfwMakeContextCurrent(window);

    while (!glfwWindowShouldClose(window)) {

    }

    glfwTerminate();
}
