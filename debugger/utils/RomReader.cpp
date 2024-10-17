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

#include "RomReader.h"

void RomReader::loadRom(char *romPath) {
    if (romPath == currLoadedGamePath || strcmp(romPath, "") == 0) return;
    Logger::getInstance("RomReader")->log(Logger::DEBUG, "Loading rom: %s", romPath);
    uint8_t mapperType = 0;

    std::ifstream file(romPath, std::ios::binary);

    file.seekg(0x0147);
    file.read(reinterpret_cast<char *>(&mapperType), sizeof(uint8_t) * 1);

    file.seekg(0x0000);
    file.read(reinterpret_cast<char *>(romData.data()), sizeof(uint8_t) * 32*1024);
    file.close();
    currLoadedGamePath = romPath;
    loaded = true;
}

bool RomReader::isLoaded() {
    return loaded;
}


