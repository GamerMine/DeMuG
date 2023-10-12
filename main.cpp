#include "hardware/Bus.h"
#include "logging/Logger.h"
#include "raylib.h"

void raylogCallback(int msgType, const char *text, va_list args) {
    Logger *logger = Logger::getInstance("Raylib");
    switch (msgType) {
        case LOG_DEBUG: {
            logger->log(Logger::DEBUG, text, args);
            break;
        }
        case LOG_INFO: {
            logger->log(Logger::INFO, text, args);
            break;
        }
        case LOG_WARNING: {
            logger->log(Logger::WARNING, text, args);
            break;
        }
        case LOG_ERROR | LOG_FATAL: {
            logger->log(Logger::CRITICAL, text, args);
            break;
        }
        default: {
            logger->log(Logger::INFO, text, args);
        }
    }
}

int main() {
    SetTraceLogCallback(raylogCallback);
    Bus bus;

    return 0;
}
