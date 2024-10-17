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

#ifndef DEMUG_EMULATORCONTROLS_H
#define DEMUG_EMULATORCONTROLS_H

#include <imgui.h>

#include "../fonts/IconsFontAwesome5.h"
#include "../../misc/ipc/SharedMemoryWriter.h"
#include "../../misc/ipc/SharedMemoryReader.h"

class EmulatorControls {
public:
    static void InitEmulatorControls();
    static void ShowEmulatorControls();

private:
    struct dbgEmuControls {
        bool isPaused;
    };

    struct dbgBusStatus {
        char currentGamePath[4096];
        bool isPaused;
    };

    inline static SharedMemoryWriter *smw;
    inline static SharedMemoryReader *smr;
    inline static dbgEmuControls *v_dbgEmuControlsW;
    inline static dbgBusStatus *v_dbgBusStatus;
};


#endif //DEMUG_EMULATORCONTROLS_H
