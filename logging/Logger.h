#ifndef EMU_GAMEBOY_LOGGER_H
#define EMU_GAMEBOY_LOGGER_H

#define stringify( name ) #name

#include <cstdio>
#include <chrono>
#include <thread>
#include <future>
#include <cstdarg>

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
    void setLogLevel(LOG_LEVEL logLevel);

private:
    LOG_LEVEL currentLogLevel = DEBUG;

    static Logger *instance;
    static const char *levelNames[] = {"DEBUG", "INFO", "WARNING", "CRITICAL"};

    void logAsync(LOG_LEVEL logLevel, const char *loggerName, const char *format, va_list args);
};


#endif //EMU_GAMEBOY_LOGGER_H
