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

#ifndef DEMUG_ROMREADER_H
#define DEMUG_ROMREADER_H

#include <cstdint>
#include <cstring>
#include <fstream>
#include <array>

#include "../../misc/logging/Logger.h"

class RomReader {
public:
    inline static std::array<uint8_t, 32*1024> romData;

    static void loadRom(char *romPath);
    static bool isLoaded();

private:
    static inline char *currLoadedGamePath;
    static inline bool loaded;

    static void loadBootRom();
};


#endif //DEMUG_ROMREADER_H
