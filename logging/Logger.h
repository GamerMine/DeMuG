/*
 *  ____
 * /\  _`\                                       /'\_/`\  __
 * \ \ \L\_\     __      ___ ___      __   _ __ /\      \/\_\    ___      __
 *  \ \ \L_L   /'__`\  /' __` __`\  /'__`\/\`'__\ \ \__\ \/\ \ /' _ `\  /'__`\
 *   \ \ \/, \/\ \L\.\_/\ \/\ \/\ \/\  __/\ \ \/ \ \ \_/\ \ \ \/\ \/\ \/\  __/
 *    \ \____/\ \__/.\_\ \_\ \_\ \_\ \____\\ \_\  \ \_\\ \_\ \_\ \_\ \_\ \____\
 *     \/___/  \/__/\/_/\/_/\/_/\/_/\/____/ \/_/   \/_/ \/_/\/_/\/_/\/_/\/____/
 *
 * Copyright (c) 2023-2023 GamerMine <maxime-sav@outlook.fr>
 *
 * This Source Code From is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/ .
 */

#ifndef EMU_GAMEBOY_LOGGER_H
#define EMU_GAMEBOY_LOGGER_H

#include <chrono>
#include <cstdarg>
#include <cstdio>
#include <future>
#include <iomanip>
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
    void log(LOG_LEVEL logLevel, const char *format, va_list args);
    static void setLogLevel(LOG_LEVEL logLevel);

private:
    static LOG_LEVEL currentLogLevel;
    const char *currentLoggerName;
    const char *levelNames[4] = {"DEBUG", "INFO", "WARNING", "CRITICAL"};
    std::string lastString;
    uint64_t nbRepeat{};

    static std::map<const char *, Logger *> loggers;

    explicit Logger(const char *loggerName);
    void logAsync(LOG_LEVEL logLevel, const char *format, va_list args);
};

class Colors {
public:
    static const char *LOG_DEFAULT;
    static const char *LOG_DARK_BLACK;
    static const char *LOG_DARK_RED;
    static const char *LOG_DARK_GREEN;
    static const char *LOG_DARK_YELLOW;
    static const char *LOG_DARK_BLUE;
    static const char *LOG_DARK_MAGENTA;
    static const char *LOG_DARK_CYAN;
    static const char *LOG_LIGHT_GRAY;
    static const char *LOG_DARK_GRAY;
    static const char *LOG_RED;
    static const char *LOG_GREEN;
    static const char *LOG_ORANGE;
    static const char *LOG_BLUE;
    static const char *LOG_MAGENTA;
    static const char *LOG_CYAN;
    static const char *LOG_WHITE;
};


#endif //EMU_GAMEBOY_LOGGER_H
