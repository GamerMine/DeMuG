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

#ifndef DEMUG_CODEVIEWER_H
#define DEMUG_CODEVIEWER_H

#include <imgui.h>
#include <memory>

#include "../utils/RomReader.h"
#include "../../misc/ipc/SharedMemoryReader.h"

class CodeViewer {
public:
    static void InitCodeViewer();
    static void ShowCodeViewer();

private:
    struct dbgCpuStatus {
        uint16_t PC;
    };

    inline static std::vector<uint8_t> *romsData;
    inline static SharedMemoryReader *smr;
    inline static dbgCpuStatus *v_dbgCpuStatus;

    static void tryLoadRoms();
};


#endif //DEMUG_CODEVIEWER_H
