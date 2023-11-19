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

#include "Logger.h"

const char *Colors::LOG_DEFAULT         = "\033[39m";
const char *Colors::LOG_DARK_BLACK      = "\033[30m";
const char *Colors::LOG_DARK_RED        = "\033[31m";
const char *Colors::LOG_DARK_GREEN      = "\033[32m";
const char *Colors::LOG_DARK_YELLOW     = "\033[33m";
const char *Colors::LOG_DARK_BLUE       = "\033[34m";
const char *Colors::LOG_DARK_MAGENTA    = "\033[35m";
const char *Colors::LOG_DARK_CYAN       = "\033[36m";
const char *Colors::LOG_LIGHT_GRAY      = "\033[37m";
const char *Colors::LOG_DARK_GRAY       = "\033[90m";
const char *Colors::LOG_RED             = "\033[91m";
const char *Colors::LOG_GREEN           = "\033[92m";
const char *Colors::LOG_ORANGE          = "\033[93m";
const char *Colors::LOG_BLUE            = "\033[94m";
const char *Colors::LOG_MAGENTA         = "\033[95m";
const char *Colors::LOG_CYAN            = "\033[96m";
const char *Colors::LOG_WHITE           = "\033[97m";

std::map<const char *, Logger *> Logger::loggers;
Logger::LOG_LEVEL Logger::currentLogLevel = Logger::DEBUG;

char buffer[8192];

Logger::Logger(const char *loggerName) {
    this->currentLoggerName = loggerName;
    //setvbuf(stdout, buffer, _IOFBF, sizeof(buffer));
}

Logger::~Logger() {
    removeInstance(this->currentLoggerName);
}

Logger *Logger::getInstance(const char *loggerName) {
    if (!loggers.contains(loggerName)) {
        loggers[loggerName] = new Logger(loggerName);
    }
    return loggers[loggerName];
}

/**
 * Logs something into stdout asynchronously with the defined LOG_LEVEL
 *
 * @param logLevel
 * @param loggerName
 * @param format
 * @param ...
 */
void Logger::log(Logger::LOG_LEVEL logLevel, const char *format, ...) {
    va_list args;
    va_start(args, format);
    std::future<void> ret = std::async(std::launch::async, &Logger::logAsync, this, logLevel, format, args);
    va_end(args);
}

void Logger::log(Logger::LOG_LEVEL logLevel, const char *format, va_list args) {
    std::future<void> ret = std::async(std::launch::async, &Logger::logAsync, this, logLevel, format, args);
}

void Logger::logAsync(Logger::LOG_LEVEL logLevel, const char *format, va_list args) {
    if (Logger::currentLogLevel <= logLevel) {
        auto t = std::time(nullptr);
        auto tm = *std::localtime(&t);
        std::stringstream out;
        out << std::put_time(&tm, "%d-%m-%Y %H:%M:%S");

        switch (logLevel) {
            case DEBUG: printf("[%s %s%s%s] <%s>: ", out.str().c_str(), Colors::LOG_DARK_GRAY, levelNames[logLevel], Colors::LOG_DEFAULT, currentLoggerName); break;
            case WARNING: printf("[%s %s%s%s] <%s>: ", out.str().c_str(), Colors::LOG_ORANGE, levelNames[logLevel], Colors::LOG_DEFAULT, currentLoggerName); break;
            case CRITICAL: printf("[%s %s%s%s] <%s>: ", out.str().c_str(), Colors::LOG_DARK_RED, levelNames[logLevel], Colors::LOG_DEFAULT, currentLoggerName); break;
            default: printf("[%s %s] <%s>: ", out.str().c_str(), levelNames[logLevel], currentLoggerName); break;
        }
        vprintf(format, args);
        printf("%s\n", Colors::LOG_DEFAULT);
    }
}

/**
 * Sets the current log level
 * Example : If logeLevel=WARNING, only WARNING and CRITICAL levels of log will be printed
 *
 * @param logLevel
 */
void Logger::setLogLevel(Logger::LOG_LEVEL logLevel) {
    currentLogLevel = logLevel;
}

bool Logger::removeInstance(const char *loggerName) {
    if (loggers.contains(loggerName)) {
        loggers.erase(loggerName);
        return true;
    }
    return false;
}
