#ifndef EMU_GAMEBOY_LOGGER_H
#define EMU_GAMEBOY_LOGGER_H

#include <chrono>
#include <cstdarg>
#include <cstdio>
#include <future>
#include <map>
#include <thread>

class Logger {
public:
    enum LOG_LEVEL {
        DEBUG = 0,
        INFO = 1,
        WARNING = 2,
        CRITICAL = 3,
    };

    ~Logger();

    static Logger *getInstance(const char *loggerName);
    static bool removeInstance(const char *loggerName);

    void log(LOG_LEVEL logLevel, const char *format, ...);
    void setLogLevel(LOG_LEVEL logLevel);

private:
    static LOG_LEVEL currentLogLevel;
    const char *currentLoggerName;
    const char *levelNames[4] = {"DEBUG", "INFO", "WARNING", "CRITICAL"};

    static std::map<const char *, Logger *> loggers;

    explicit Logger(const char *loggerName);
    void logAsync(LOG_LEVEL logLevel, const char *format, va_list args);
};


#endif //EMU_GAMEBOY_LOGGER_H
