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

#ifndef EMU_GAMEBOY_MBC1_H
#define EMU_GAMEBOY_MBC1_H

#include "Cartridge.h"

class MBC1 : public Cartridge {
public:
    uint8_t read(uint16_t addr) override {
        uint8_t value = 0xFF;

        if (addr <= 0x3FFF) {
            if (!bankingMode) value = gameRom[addr];
        } else if (addr >= 0x4000 && addr <= 0x7FFF) {
            value = gameRom[romBankSelect * 0x4000 + (addr - 0x4000)];
        } else if (addr >= 0xA000 && addr <= 0xBFFF) {
            if (ramEnable) value = gameRam[ramBankSelect * 0x2000 + (addr - 0xA000)];
        }

        return value;
    }

    void write(uint16_t addr, uint8_t value) override {
        if (addr <= 0x1FFF) {
            if ((value & 0x0F) == 0x0A) ramEnable = true;
            else ramEnable = false;
        } else if (addr >= 0x2000 && addr <= 0x3FFF) {
            if ((value & 0x1F) == 0x00) romBankSelect = 0x01;
            else if ((value & 0x1F) == 0x10 && romBankNumber <= 16) romBankSelect = 0x00;
            else romBankSelect = (value/* & romBankNumber*/);
        } else if (addr >= 0x4000 && addr <= 0x5FFF) {
            if (ramSize == 0x03) ramBankSelect = value & 0x03;
        } else if (addr >= 0x6000 && addr <= 0x7FFF) {
            bankingMode = value & 0x01;
        } else if (addr >= 0xA000 && addr <= 0xBFFF) {
            if (ramEnable) {
                if (!bankingMode) gameRam[addr - 0xA000] = value;
                else gameRam[ramBankSelect * 0x2000 + (addr - 0xA000)] = value;
            }
        }
    }

protected:
    void loadGame(const char *filename) override {
        std::ifstream file(filename, std::ios::binary);

        file.seekg(0x0148);
        file.read(reinterpret_cast<char *>(&romSize), sizeof(uint8_t));
        gameRom.resize(32 * 1024 * (1 << romSize));
        romBankNumber = (1 << romSize) * 2;
        if (romBankNumber > 16) Logger::getInstance("Cartridge")->log(Logger::WARNING, "Large cartridge are not yet supported!");

        file.seekg(0x0149);
        file.read(reinterpret_cast<char *>(&ramSize), sizeof(uint8_t));

        file.seekg(0x0000);
        file.read(reinterpret_cast<char *>(gameRom.data()), sizeof(uint8_t) * 32 * 1024 * (1 << romSize));
        file.close();

        gameRam.resize(ramSizes[ramSize] * 1024);

        // Parsing TOM header
        /*for (uint16_t i = 0; i < 16; i++) Debug::CARTRIDGE_INFO.title[i] = (char)gameRom[0x0134 + i]; // Title
        Debug::CARTRIDGE_INFO.newLicenseCode = gameRom[0x0144];
        Debug::CARTRIDGE_INFO.cartridgeType = gameRom[0x0147];
        Debug::CARTRIDGE_INFO.romSize = gameRom[0x0148];
        Debug::CARTRIDGE_INFO.ramSize = gameRom[0x0149];
        Debug::CARTRIDGE_INFO.destinationCode = gameRom[0x014A];
        Debug::CARTRIDGE_INFO.oldLicenseCode = gameRom[0x014B];

        Debug::printRomHeaderData();
        Logger::getInstance("Cartridge")->log(Logger::DEBUG, "%sNumber of ROM banks: %d", Colors::LOG_DARK_YELLOW, romBankNumber);
        Logger::getInstance("Cartridge")->log(Logger::DEBUG, "%sRAM size: %d", Colors::LOG_DARK_YELLOW, ramSizes[ramSize]);

        std::string windowTitle = "Emulating ";
        windowTitle = windowTitle.append(Debug::CARTRIDGE_INFO.title);
        while (!IsWindowReady()) {}
        SetWindowTitle(windowTitle.c_str());*/
    }

private:
    bool ramEnable = false;
    uint8_t romBankSelect = 0x01;
    uint8_t ramBankSelect = 0x00;
    bool bankingMode = 0;

};

#endif //EMU_GAMEBOY_MBC1_H
