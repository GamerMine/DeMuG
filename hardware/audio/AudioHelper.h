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

#ifndef EMU_GAMEBOY_AUDIOHELPER_H
#define EMU_GAMEBOY_AUDIOHELPER_H

#include <cstdint>
#include <vector>

class AudioHelper {
public:
    static std::vector<uint8_t> increaseDepth(const std::vector<uint8_t>& initialInputBuffer) {
        // Convert 4bit to 8bit
        std::vector<uint8_t> inputBuffer;
        for (uint8_t value : initialInputBuffer) {
            inputBuffer.push_back(value & 0xF0);
            inputBuffer.push_back(value << 4);
        }

        return inputBuffer;
    }
};

#endif //EMU_GAMEBOY_AUDIOHELPER_H
