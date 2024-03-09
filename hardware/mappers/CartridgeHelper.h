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

#ifndef EMU_GAMEBOY_CARTRIDGEHELPER_H
#define EMU_GAMEBOY_CARTRIDGEHELPER_H

#include <fstream>

#include "../../debug/Debug.h"
#include "../../logging/Logger.h"

#include "Cartridge.h"
#include "NoMBC.h"

class CartridgeHelper {
public:
    static std::unique_ptr<Cartridge> readGameRom(const char *filename) {
        Logger *logger = Logger::getInstance("Cartridge");
        uint8_t mapperType = 0;

        logger->log(Logger::DEBUG, "Reading game : %s", filename);
        std::ifstream file(filename, std::ios::binary);

        if (!file.good()) {
            logger->log(Logger::CRITICAL, "File not found: %s", filename);
            exit(Debug::GAME_NOT_FOUND_EXIT_CODE);
        }

        file.seekg(0x0147);
        file.read(reinterpret_cast<char *>(&mapperType), sizeof(uint8_t) * 1);
        file.close();

        std::unique_ptr<Cartridge> cartridge;
        switch (mapperType) {
            case 0x00: cartridge = std::make_unique<NoMBC>();
            default: logger->log(Logger::WARNING, "Mapper type %X is not implemented!", mapperType);
        }

        cartridge->loadGame(filename);

        return cartridge;

        // Parsing TOM header
        /*logger->log(Logger::DEBUG, "Parsing ROM header...");

        for (uint16_t i = 0; i < 16; i++) Debug::CARTRIDGE_INFO.title[i] = (char)gameRom[0x0134 + i]; // Title
        Debug::CARTRIDGE_INFO.newLicenseCode = gameRom[0x0144];
        Debug::CARTRIDGE_INFO.cartridgeType = gameRom[0x0147];
        Debug::CARTRIDGE_INFO.romSize = gameRom[0x0148];
        Debug::CARTRIDGE_INFO.ramSize = gameRom[0x0149];
        Debug::CARTRIDGE_INFO.destinationCode = gameRom[0x014A];
        Debug::CARTRIDGE_INFO.oldLicenseCode = gameRom[0x014B];

        Debug::printRomHeaderData();

        std::string windowTitle = "Emulating ";
        windowTitle = windowTitle.append(Debug::CARTRIDGE_INFO.title);
        while (!IsWindowReady()) {}
        SetWindowTitle(windowTitle.c_str());*/
    }
};

#endif //EMU_GAMEBOY_CARTRIDGEHELPER_H
