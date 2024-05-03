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
            value = gameRom[addr];
            //Logger::getInstance("Cartridge")->log(Logger::DEBUG, "Read %X at %X in bank %d", value, addr, 0);
        } else if (addr >= 0x4000 && addr <= 0x7FFF) {
            uint8_t currentRomBank;

            if (romBankRegister == 0x00) currentRomBank = 0x01;
            else currentRomBank = romBankRegister & (romBankNumber - 1);

            value = gameRom[currentRomBank * 0x4000 + (addr - 0x4000)];
            //Logger::getInstance("Cartridge")->log(Logger::DEBUG, "Read %X at %X in bank %d", value, currentRomBank * 0x4000 + (addr - 0x4000), currentRomBank);
        } else if (addr >= 0xA000 && addr <= 0xBFFF) {
            if (ramEnable) value = gameRam[ramBankRegister * 0x2000 + (addr - 0x2000)];
        } else {
            Logger::getInstance("Cartridge")->log(Logger::DEBUG, "%sShould not be here, trying to read at %X", Colors::LOG_DARK_RED, addr);
        }

        //Logger::getInstance("Cartridge")->log(Logger::DEBUG, "%sRead %X at %X", Colors::LOG_CYAN, value, addr);

        return value;
    }

    void write(uint16_t addr, uint8_t value) override {
        if (addr <= 0x1FFF) {
            if ((value & 0x0F) == 0x0A) ramEnable = true;
            else ramEnable = false;
        } else if (0x2000 && addr <= 0x3FFF) {
            romBankRegister = value & 0x1F;
        } else if (addr >= 0x4000 && addr <= 0x5FFF) {
            ramBankRegister = value & 0x03;
        } else if (addr >= 0x6000 && addr <= 0x7FFF) {
            bankingModeSelect = value & 0x01; //TODO: Use this for 1MiB ROM or larger
        } else if (addr >= 0xA000 && addr <= 0xBFFF) {
            if (ramEnable) gameRam[ramBankRegister * 0x2000 + (addr - 0x2000)] = value;
        } else {
            Logger::getInstance("Cartridge")->log(Logger::DEBUG, "%sShould not be here, trying to write %X at %X", Colors::LOG_DARK_RED, value, addr);
        }

        //Logger::getInstance("Cartridge")->log(Logger::DEBUG, "%sRead %X at %X", Colors::LOG_MAGENTA, value, addr);
    }

protected:
    void loadGame(const char *filename) override {
        std::ifstream file(filename, std::ios::binary);

        file.seekg(0x0148);
        file.read(reinterpret_cast<char *>(&romSize), sizeof(uint8_t));
        gameRom.resize(32 * 1024 * (1 << romSize));
        romBankNumber = (1 << romSize) * 2;
        if (romBankNumber > 32) Logger::getInstance("Cartridge")->log(Logger::WARNING, "Large cartridge are not yet supported!");

        file.seekg(0x0149);
        file.read(reinterpret_cast<char *>(&ramSize), sizeof(uint8_t));

        file.seekg(0x0000);
        file.read(reinterpret_cast<char *>(gameRom.data()), sizeof(uint8_t) * ((32 * 1024) * (1 << romSize)));
        file.close();

        gameRam.resize(ramSizes[ramSize] * 1024);

        std::fill(gameRam.begin(), gameRam.end(), 0x00);

        loadSave();
    }

private:
    bool ramEnable = false;
    uint8_t romBankRegister = 0x00;
    uint8_t ramBankRegister = 0x00;
    bool bankingModeSelect = 0;

};

#endif //EMU_GAMEBOY_MBC1_H
