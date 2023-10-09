#include "hardware/Bus.h"
#include "logging/Logger.h"
#include "raylib.h"

void raylogCallback(int msgType, const char *text, va_list args) {
    switch (msgType) {
        case LOG_DEBUG: {
            Logger::getInstance("Raylog")->log(Logger::DEBUG, text, args);
            break;
        }
        case LOG_INFO: {
            Logger::getInstance("Raylog")->log(Logger::INFO, text, args);
            break;
        }
        case LOG_WARNING: {
            Logger::getInstance("Raylog")->log(Logger::WARNING, text, args);
            break;
        }
        case LOG_ERROR: {
            Logger::getInstance("Raylog")->log(Logger::CRITICAL, text, args);
            break;
        }
        case LOG_FATAL: {
            Logger::getInstance("Raylog")->log(Logger::CRITICAL, text, args);
            break;
        }
    }
}

int main() {
    SetTraceLogCallback(raylogCallback);
    Bus bus;

    return 0;
}
