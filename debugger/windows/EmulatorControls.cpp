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

#include "EmulatorControls.h"

void EmulatorControls::InitEmulatorControls() {
    smw = new SharedMemoryWriter("/demugger_emu_controls", sizeof(dbgEmuControls));
    v_dbgEmuControlsW = (dbgEmuControls*)smw->shm_ptr;

    smr = new SharedMemoryReader("/demug_bus", sizeof(dbgBusStatus));
    v_dbgBusStatus = (dbgBusStatus*) smr->shm_ptr;
}

void EmulatorControls::ShowEmulatorControls() {
    ImGui::Begin("Emulator Controls");

    if (ImGui::Button(v_dbgBusStatus->isPaused ? ICON_FA_PLAY " Run" : ICON_FA_PAUSE " Pause")) {
        v_dbgEmuControlsW->isPaused = !v_dbgEmuControlsW->isPaused;
    }

    ImGui::End();
}
