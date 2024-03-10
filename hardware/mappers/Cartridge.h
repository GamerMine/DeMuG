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

class Cartridge {
    friend class CartridgeHelper;

public:
    virtual uint8_t read(uint16_t addr) = 0;
    virtual void write(uint16_t, uint8_t) {};

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
