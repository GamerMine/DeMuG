#include "Logger.h"

Logger *Logger::getInstance() {
    if (Logger::instance == nullptr) {
        Logger::instance = new Logger;
    }
    return Logger::instance;
}

/**
 * Logs something into stdout asynchronously with the defined LOG_LEVEL
 *
 * @param logLevel
 * @param loggerName
 * @param format
 * @param ...
 */
void Logger::log(Logger::LOG_LEVEL logLevel, const char *loggerName, const char *format, ...) {
    va_list args;
    va_start(args, format);
    std::future<void> ret = std::async(std::launch::async, &Logger::logAsync, this, logLevel, loggerName, format, args);
    va_end(args);
}

void Logger::logAsync(Logger::LOG_LEVEL logLevel, const char *loggerName, const char *format, va_list args) {
    if (currentLogLevel >= logLevel) {
        auto t = std::time(nullptr);
        auto tm = *std::localtime(&t);
        std::stringstream out;
        out << std::put_time(&tm, "%d-%m-%Y %H:%M:%S");

        printf("[%s %s] %s ", out.str().c_str(), stringify(logLevel), loggerName);
        vprintf(format, args);
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
