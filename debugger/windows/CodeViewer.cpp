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

#include "CodeViewer.h"

void CodeViewer::InitCodeViewer() {
    smr_cpu = new SharedMemoryReader("/demug_cpu", sizeof(dbgCpuStatus));
    v_dbgCpuStatus = (dbgCpuStatus*) smr_cpu->shm_ptr;

    smr_bus = new SharedMemoryReader("/demug_bus", sizeof(dbgBusStatus));
    v_dbgBusStatus = (dbgBusStatus*) smr_bus->shm_ptr;
}

void CodeViewer::ShowCodeViewer() {
    RomReader::loadRom(v_dbgBusStatus->currentGamePath);

    ImGui::Begin("Code Viewer");

    createCodeViewTable();

    ImGui::End();
}

void CodeViewer::createCodeViewTable() {
    if (!RomReader::isLoaded()) return;
    ImGui::BeginTable("codeViewer", 2, ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersInnerH | ImGuiTableFlags_BordersInnerV);
    ImGui::TableSetupColumn("address", ImGuiTableColumnFlags_WidthFixed, 50.0f);
    uint16_t startingAddress = 0x0000;
    uint8_t currentRowNumber = 0x0000;
    char addr[8];

    for (uint16_t data = 0x0000; data < RomReader::romData.size(); data++) {
        ImGui::TableNextRow();
        if (v_dbgCpuStatus->PC == data) {
            ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg0, ImColor(0.0f, 1.0f, 0.0f, 0.30f));
            ImGui::SetScrollHereY();
        }

        for (uint16_t breakpoint : breakpoints) {
            if (breakpoint == currentRowNumber) ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg0, ImColor(1.0f, 0.0f, 0.0f, 0.30f));
        }

        ImGui::TableSetColumnIndex(0);
        sprintf(addr, "0x%04X", startingAddress);
        //ImGui::Text("0x%X", startingAddress);
        if (ImGui::Selectable(addr)) {
            auto it = std::find(breakpoints.begin(), breakpoints.end(), currentRowNumber);
            if (it != breakpoints.end()) breakpoints.erase(std::remove(breakpoints.begin(), breakpoints.end(), currentRowNumber), breakpoints.end());
            else breakpoints.push_back(currentRowNumber);
        }

        ImGui::TableSetColumnIndex(1);
        instr_t instr = opcodes[RomReader::romData[data]](data);
        ImGui::Text("%s", instr.finalString);

        data += instr.nbRomSkip;
        startingAddress += instr.nbRomSkip > 0 ? instr.nbRomSkip + 1 : 1;
        currentRowNumber++;
    }

    ImGui::EndTable();
}

CodeViewer::instr_t CodeViewer::getInstr(const char *fmt, CodeViewer::addressing addrMode, uint16_t currAddr) {
    instr_t instr{};
    switch (addrMode) {
        case CodeViewer::addressing::NONE: {
            instr.nbRomSkip = 0;
            strcpy(instr.finalString, fmt);
            break;
        }
        case CodeViewer::addressing::IMMEDIATE_8: {
            char fStr[0xFF];
            uint8_t value = RomReader::romData[currAddr + 1];
            sprintf(fStr, fmt, value);
            instr.nbRomSkip = 1;
            strcpy(instr.finalString, fStr);
            break;
        }
        case CodeViewer::addressing::IMMEDIATE_16: {
            char fStr[0xFF];
            uint16_t value = (RomReader::romData[currAddr + 2] << 8) | RomReader::romData[currAddr + 1];
            sprintf(fStr, fmt, value);
            instr.nbRomSkip = 2;
            strcpy(instr.finalString, fStr);
            break;
        }
    }

    return instr;
}
