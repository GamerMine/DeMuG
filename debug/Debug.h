/*
 *  ____
 * /\  _`\                                       /'\_/`\  __
 * \ \ \L\_\     __      ___ ___      __   _ __ /\      \/\_\    ___      __
 *  \ \ \L_L   /'__`\  /' __` __`\  /'__`\/\`'__\ \ \__\ \/\ \ /' _ `\  /'__`\
 *   \ \ \/, \/\ \L\.\_/\ \/\ \/\ \/\  __/\ \ \/ \ \ \_/\ \ \ \/\ \/\ \/\  __/
 *    \ \____/\ \__/.\_\ \_\ \_\ \_\ \____\\ \_\  \ \_\\ \_\ \_\ \_\ \_\ \____\
 *     \/___/  \/__/\/_/\/_/\/_/\/_/\/____/ \/_/   \/_/ \/_/\/_/\/_/\/_/\/____/
 *
 * Copyright (c) 2024-2024 GamerMine <maxime-sav@outlook.fr>
 *
 * This Source Code From is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/ .
 */

#ifndef EMU_GAMEBOY_DEBUG_H
#define EMU_GAMEBOY_DEBUG_H

#include <cstdint>
#include "../logging/Logger.h"

class Debug {
public:
    inline static struct {
        char title[16];
        uint8_t cartridgeType;
        uint8_t romSize;
        uint8_t ramSize;
        uint8_t destinationCode;
        uint8_t oldLicenseCode;
        uint8_t newLicenseCode;
    } CARTRIDGE_INFO;

    inline static int GAME_NOT_FOUND_EXIT_CODE = 1;

    static void printRomHeaderData() {
        Logger *logger = Logger::getInstance("Debug");

        logger->log(Logger::DEBUG, "%sROM Infos :", Colors::LOG_DARK_YELLOW);
        logger->log(Logger::DEBUG, "\t%sTitle : %s", Colors::LOG_DARK_YELLOW, CARTRIDGE_INFO.title);
        logger->log(Logger::DEBUG, "\t%sCartridge Type : %X", Colors::LOG_DARK_YELLOW, CARTRIDGE_INFO.cartridgeType);
        logger->log(Logger::DEBUG, "\t%sROM Size : %X", Colors::LOG_DARK_YELLOW, CARTRIDGE_INFO.romSize);
        logger->log(Logger::DEBUG, "\t%sRAM Size : %X", Colors::LOG_DARK_YELLOW, CARTRIDGE_INFO.ramSize);
        logger->log(Logger::DEBUG, "\t%sDestination Code : %X", Colors::LOG_DARK_YELLOW, CARTRIDGE_INFO.destinationCode);
        logger->log(Logger::DEBUG, "\t%sOld License Code : %X", Colors::LOG_DARK_YELLOW, CARTRIDGE_INFO.oldLicenseCode);
        logger->log(Logger::DEBUG, "\t%sNew License Code : %X", Colors::LOG_DARK_YELLOW, CARTRIDGE_INFO.newLicenseCode);
    }
};


#endif //EMU_GAMEBOY_DEBUG_H
