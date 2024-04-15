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

#include "hardware/Bus.h"
#include "logging/Logger.h"
#include <raylib.h>
#define RAYGUI_IMPLEMENTATION
#include "gui/raygui.h"

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

int main(int argc, const char *argv[]) {
    SetTraceLogCallback(raylogCallback);

    Bus bus(argc <= 1 ? "" : argv[1]); // Launch with game if supplied else launch without game

    bus.startEmulation();

    return 0;
}
