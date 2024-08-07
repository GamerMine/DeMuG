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
    static inline char *gamePath;

    static std::unique_ptr<Cartridge> readGameRom(const char *filename) {
        Logger *logger = Logger::getInstance("Cartridge");
        uint8_t mapperType = 0;

        logger->log(Logger::DEBUG, "Reading game : %s", filename);
        strcpy(gamePath, filename);
        std::ifstream file(filename, std::ios::binary);

        if (!file.good()) {
            logger->log(Logger::CRITICAL, "File not found: %s", filename);
            exit(Debug::GAME_NOT_FOUND_EXIT_CODE);
        }

        file.seekg(0x0147);
        file.read(reinterpret_cast<char *>(&mapperType), sizeof(uint8_t) * 1);
        file.close();

        readGameRomData(filename);

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

    static void readGameRomData(const char *filename) {
        std::ifstream file(filename, std::ios::binary);
        uint8_t value;

        // Parsing ROM header
        for (uint16_t i = 0; i < 15; i++) { // Title
            file.seekg(0x0134 + i);
            file.read(reinterpret_cast<char *>(&value), sizeof(uint8_t) * 1);
            Debug::CARTRIDGE_INFO.title[i] = (char)value;
        }
        file.seekg(0x0144);
        file.read(reinterpret_cast<char *>(&value), sizeof(uint8_t) * 1);
        Debug::CARTRIDGE_INFO.newLicenseCode = value;
        file.seekg(0x147);
        file.read(reinterpret_cast<char *>(&value), sizeof(uint8_t) * 1);
        Debug::CARTRIDGE_INFO.cartridgeType = value;
        file.seekg(0x0148);
        file.read(reinterpret_cast<char *>(&value), sizeof(uint8_t) * 1);
        Debug::CARTRIDGE_INFO.romSize = value;
        file.seekg(0x0149);
        file.read(reinterpret_cast<char *>(&value), sizeof(uint8_t) * 1);
        Debug::CARTRIDGE_INFO.ramSize = value;
        file.seekg(0x014A);
        file.read(reinterpret_cast<char *>(&value), sizeof(uint8_t) * 1);
        Debug::CARTRIDGE_INFO.destinationCode = value;
        file.seekg(0x014B);
        file.read(reinterpret_cast<char *>(&value), sizeof(uint8_t) * 1);
        Debug::CARTRIDGE_INFO.oldLicenseCode = value;

        Debug::printRomHeaderData();

        std::string windowTitle = "Emulating ";
        windowTitle = windowTitle.append(Debug::CARTRIDGE_INFO.title);
        while (!IsWindowReady()) {}
        SetWindowTitle(windowTitle.c_str());
    }

    static bool checkRomValidity(const char *filename) {
        const std::string strFilename(filename);

        if (strcmp(filename, "") == 0) return false;
        if (strFilename.substr(strFilename.length() - 3) != ".gb") return false;

        return true;
    }
};

#endif //EMU_GAMEBOY_CARTRIDGEHELPER_H
