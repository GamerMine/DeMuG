#include "Logger.h"

const char *Colors::DEFAULT         = "\033[39m";
const char *Colors::DARK_BLACK      = "\033[30m";
const char *Colors::DARK_RED        = "\033[31m";
const char *Colors::DARK_GREEN      = "\033[32m";
const char *Colors::DARK_YELLOW     = "\033[33m";
const char *Colors::DARK_BLUE       = "\033[34m";
const char *Colors::DARK_MAGENTA    = "\033[35m";
const char *Colors::DARK_CYAN       = "\033[36m";
const char *Colors::LIGHT_GRAY      = "\033[37m";
const char *Colors::DARK_GRAY       = "\033[90m";
const char *Colors::RED             = "\033[91m";
const char *Colors::GREEN           = "\033[92m";
const char *Colors::ORANGE          = "\033[93m";
const char *Colors::BLUE            = "\033[94m";
const char *Colors::MAGENTA         = "\033[95m";
const char *Colors::CYAN            = "\033[96m";
const char *Colors::WHITE           = "\033[97m";

std::map<const char *, Logger *> Logger::loggers;
Logger::LOG_LEVEL Logger::currentLogLevel = Logger::DEBUG;

Logger::Logger(const char *loggerName) {
    this->currentLoggerName = loggerName;
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

void Logger::logAsync(Logger::LOG_LEVEL logLevel, const char *format, va_list args) {
    if (Logger::currentLogLevel <= logLevel) {
        auto t = std::time(nullptr);
        auto tm = *std::localtime(&t);
        std::stringstream out;
        out << std::put_time(&tm, "%d-%m-%Y %H:%M:%S");

        switch (logLevel) {
            case DEBUG: printf("[%s %s%s%s] <%s>: ", out.str().c_str(), Colors::DARK_GRAY, levelNames[logLevel], Colors::DEFAULT, currentLoggerName); break;
            case WARNING: printf("[%s %s%s%s] <%s>: ", out.str().c_str(), Colors::ORANGE, levelNames[logLevel], Colors::DEFAULT, currentLoggerName); break;
            case CRITICAL: printf("[%s %s%s%s] <%s>: ", out.str().c_str(), Colors::DARK_RED, levelNames[logLevel], Colors::DEFAULT, currentLoggerName); break;
            default: printf("[%s %s] <%s>: ", out.str().c_str(), levelNames[logLevel], currentLoggerName); break;
        }
        vprintf(format, args);
        printf("%s\n", Colors::DEFAULT);
    }
}

/**
 * Sets the current log level
 * Example : If logeLevel=WARNING, only WARNING and CRITICAL levels of log will be printed
 *
 * @param logLevel
 */
void Logger::setLogLevel(Logger::LOG_LEVEL logLevel) {
    this->currentLogLevel = logLevel;
}

bool Logger::removeInstance(const char *loggerName) {
    if (loggers.contains(loggerName)) {
        loggers.erase(loggerName);
        return true;
    }
    return false;
}
