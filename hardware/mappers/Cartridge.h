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

#ifndef EMU_GAMEBOY_CARTRIDGE_H
#define EMU_GAMEBOY_CARTRIDGE_H

#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iostream>

class Cartridge {
    friend class CartridgeHelper;

public:
    virtual uint8_t read(uint16_t addr) = 0;
    virtual void write(uint16_t, uint8_t) {};

    void save() {
        if (!gameRam.empty()) {
            std::string saveFileLocation = "./saves/";
#ifdef __APPLE__
            saveFileLocation = std::string(getenv("HOME")) + "/Library/Application Support/DeMuG/saves/";
            std::filesystem::create_directories(std::string(getenv("HOME")) + "/Library/Application Support/DeMuG");
#endif

            std::filesystem::create_directory(saveFileLocation);
            std::ofstream saveFile(saveFileLocation + Debug::CARTRIDGE_INFO.title + ".demug", std::ios::binary);

            saveFile.write(reinterpret_cast<char *>(gameRam.data()), sizeof(uint8_t) * ramSizes[ramSize] * 1024);

            saveFile.close();
        }
    };

    void loadSave() {
        std::string saveFileLocation = "./saves/";
#ifdef __APPLE__
        saveFileLocation = std::string(getenv("HOME")) + "/Library/Application Support/DeMuG/saves/";
#endif

        if (std::filesystem::exists(saveFileLocation + Debug::CARTRIDGE_INFO.title + ".demug")) {
            std::ifstream saveFile(saveFileLocation + Debug::CARTRIDGE_INFO.title + ".demug", std::ios::binary);

            saveFile.read(reinterpret_cast<char *>(gameRam.data()), sizeof(uint8_t) * ramSizes[ramSize] * 1024);

            saveFile.close();
        }
    }

    virtual ~Cartridge() = default;
protected:
    virtual void loadGame(const char *filename) = 0;

protected:
    inline static std::array<uint8_t, 6> ramSizes = {0, 0, 8, 32, 128, 64};
    std::vector<uint8_t> gameRom{};
    std::vector<uint8_t> gameRam{};
    uint8_t romSize{};
    uint16_t romBankNumber{};
    uint8_t ramSize{};

};

#endif //EMU_GAMEBOY_CARTRIDGE_H
