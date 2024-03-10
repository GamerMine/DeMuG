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
#include "MBC1.h"

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
            case 0x00: cartridge = std::make_unique<NoMBC>(); break;
            case 0x01: cartridge = std::make_unique<MBC1>(); break;
            case 0x02: cartridge = std::make_unique<MBC1>(); break;
            case 0x03: cartridge = std::make_unique<MBC1>(); break;
            default: logger->log(Logger::WARNING, "Mapper type %X is not implemented!", mapperType); break;
        }

        cartridge->loadGame(filename);

        return cartridge;
    }
};

#endif //EMU_GAMEBOY_CARTRIDGEHELPER_H
