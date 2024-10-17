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

#ifndef DEMUG_DEBUGGERWINDOW_H
#define DEMUG_DEBUGGERWINDOW_H

#include <GLFW/glfw3.h>
#include <memory>

#include "backends/imgui_impl_glfw.h"
#include <backends/imgui_impl_opengl3.h>
#include <imgui.h>

#include "windows/CodeViewer.h"
#include "windows/EmulatorControls.h"

class DebuggerWindow {
public:
    static void startDebugger();

private:
    static void setup();
    static void renderLoop();
};


#endif //DEMUG_DEBUGGERWINDOW_H
