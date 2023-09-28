#ifndef EMU_GAMEBOY_LOGGER_H
#define EMU_GAMEBOY_LOGGER_H

#include <cstdio>
#include <chrono>
#include <thread>
#include <future>
#include <stdarg.h>

class Logger {
public:
    enum LOG_LEVEL {
        DEBUG = 0,
        INFO = 1,
        WARNING = 2,
        CRITICAL = 3,
    };

    static Logger *getInstance();
    void log(LOG_LEVEL logLevel, const char *loggerName, const char *format, ...);

private:
    LOG_LEVEL currentLogLevel = DEBUG;

    static Logger *instance;

    void logAsync(LOG_LEVEL logLevel, const char *loggerName, const char *format, va_list args);
};


#endif //EMU_GAMEBOY_LOGGER_H
