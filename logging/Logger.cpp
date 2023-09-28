#include "Logger.h"

std::map<const char *, Logger *> Logger::loggers;

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
    if (currentLogLevel >= logLevel) {
        auto t = std::time(nullptr);
        auto tm = *std::localtime(&t);
        std::stringstream out;
        out << std::put_time(&tm, "%d-%m-%Y %H:%M:%S");

        printf("[%s %s] <%s>: ", out.str().c_str(), levelNames[logLevel], currentLoggerName);
        vprintf(format, args);
        printf("\n");
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
