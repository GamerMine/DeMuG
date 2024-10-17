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

    struct dbgBusStatus {
        char currentGamePath[4096];
    };

    inline static std::vector<uint8_t> *romsData;
    inline static SharedMemoryReader *smr_cpu;
    inline static SharedMemoryReader *smr_bus;
    inline static dbgCpuStatus *v_dbgCpuStatus;
    inline static dbgBusStatus *v_dbgBusStatus;

    inline static std::vector<uint16_t> breakpoints;

    static void createCodeViewTable();

private:
    enum addressing {
        NONE,
        IMMEDIATE_8,
        IMMEDIATE_16,
    };

    struct instr_t {
        uint8_t nbRomSkip;
        char finalString[0xFF];
    };

    static CodeViewer::instr_t getInstr(const char *fmt, CodeViewer::addressing addrMode, uint16_t currAddr);

    using opcodeFunc = std::function<instr_t(uint16_t currAddr)>;
    inline static opcodeFunc opcodes[256] = {
            [](uint16_t currAddr) { return getInstr("NOP", addressing::NONE, currAddr); },
            [](uint16_t currAddr) { return getInstr("LD BC, 0x%04X", addressing::IMMEDIATE_16, currAddr); },
            [](uint16_t currAddr) { return getInstr("LD [BC], A", addressing::NONE, currAddr); },
            [](uint16_t currAddr) { return getInstr("INC BC", addressing::NONE, currAddr); },
            [](uint16_t currAddr) { return getInstr("INC B", addressing::NONE, currAddr); },
            [](uint16_t currAddr) { return getInstr("DEC B", addressing::NONE, currAddr); },
            [](uint16_t currAddr) { return getInstr("LD B, 0x%02X", addressing::IMMEDIATE_8, currAddr); },
            [](uint16_t currAddr) { return getInstr("RLCA", addressing::NONE, currAddr); },
            [](uint16_t currAddr) { return getInstr("LD [0x%04X], SP", addressing::IMMEDIATE_16, currAddr); },
            [](uint16_t currAddr) { return getInstr("ADD HL, BC", addressing::NONE, currAddr); },
            [](uint16_t currAddr) { return getInstr("LD A, [BC]", addressing::NONE, currAddr); },
            [](uint16_t currAddr) { return getInstr("DEC BC", addressing::NONE, currAddr); },
            [](uint16_t currAddr) { return getInstr("INC C", addressing::NONE, currAddr); },
            [](uint16_t currAddr) { return getInstr("DEC C", addressing::NONE, currAddr); },
            [](uint16_t currAddr) { return getInstr("LD C, 0x%02X", addressing::IMMEDIATE_8, currAddr); },
            [](uint16_t currAddr) { return getInstr("RRCA", addressing::NONE, currAddr); },
            [](uint16_t currAddr) { return getInstr("STOP 0x%02X", addressing::IMMEDIATE_8, currAddr); },
            [](uint16_t currAddr) { return getInstr("LD DE, 0x%04X", addressing::IMMEDIATE_16, currAddr); },
            [](uint16_t currAddr) { return getInstr("LD [DE], A", addressing::NONE, currAddr); },
            [](uint16_t currAddr) { return getInstr("INC DE", addressing::NONE, currAddr); },
            [](uint16_t currAddr) { return getInstr("INC D", addressing::NONE, currAddr); },
            [](uint16_t currAddr) { return getInstr("DEC D", addressing::NONE, currAddr); },
            [](uint16_t currAddr) { return getInstr("LD D, 0x%02X", addressing::IMMEDIATE_8, currAddr); },
            [](uint16_t currAddr) { return getInstr("RLA", addressing::NONE, currAddr); },
            [](uint16_t currAddr) { return getInstr("JR e8", addressing::NONE, currAddr); },
            [](uint16_t currAddr) { return getInstr("ADD HL, DE", addressing::NONE, currAddr); },
            [](uint16_t currAddr) { return getInstr("LD A, [DE]", addressing::NONE, currAddr); },
            [](uint16_t currAddr) { return getInstr("DEC DE", addressing::NONE, currAddr); },
            [](uint16_t currAddr) { return getInstr("INC E", addressing::NONE, currAddr); },
            [](uint16_t currAddr) { return getInstr("DEC E", addressing::NONE, currAddr); },
            [](uint16_t currAddr) { return getInstr("LD E, 0x%02X", addressing::IMMEDIATE_8, currAddr); },
            [](uint16_t currAddr) { return getInstr("RRA", addressing::NONE, currAddr); },
            [](uint16_t currAddr) { return getInstr("JR NZ, e8", addressing::NONE, currAddr); },
            [](uint16_t currAddr) { return getInstr("LD HL, 0x%04X", addressing::IMMEDIATE_16, currAddr); },
            [](uint16_t currAddr) { return getInstr("LD [HL+], A", addressing::NONE, currAddr); },
            [](uint16_t currAddr) { return getInstr("INC HL", addressing::NONE, currAddr); },
            [](uint16_t currAddr) { return getInstr("INC H", addressing::NONE, currAddr); },
            [](uint16_t currAddr) { return getInstr("DEC H", addressing::NONE, currAddr); },
            [](uint16_t currAddr) { return getInstr("LD H, 0x%02X", addressing::IMMEDIATE_8, currAddr); },
            [](uint16_t currAddr) { return getInstr("DAA", addressing::NONE, currAddr); },
            [](uint16_t currAddr) { return getInstr("JR Z, e8", addressing::NONE, currAddr); },
            [](uint16_t currAddr) { return getInstr("ADD HL, HL", addressing::NONE, currAddr); },
            [](uint16_t currAddr) { return getInstr("LD A, [HL+]", addressing::NONE, currAddr); },
            [](uint16_t currAddr) { return getInstr("DEC HL", addressing::NONE, currAddr); },
            [](uint16_t currAddr) { return getInstr("INC L", addressing::NONE, currAddr); },
            [](uint16_t currAddr) { return getInstr("DEC L", addressing::NONE, currAddr); },
            [](uint16_t currAddr) { return getInstr("LD L, 0x%02X", addressing::IMMEDIATE_8, currAddr); },
            [](uint16_t currAddr) { return getInstr("CPL", addressing::NONE, currAddr); },
            [](uint16_t currAddr) { return getInstr("JR NC, e8", addressing::NONE, currAddr); },
            [](uint16_t currAddr) { return getInstr("LD SP, 0x%04X", addressing::IMMEDIATE_16, currAddr); },
            [](uint16_t currAddr) { return getInstr("LD [HL-], A", addressing::NONE, currAddr); },
            [](uint16_t currAddr) { return getInstr("INC SP", addressing::NONE, currAddr); },
            [](uint16_t currAddr) { return getInstr("INC [HL]", addressing::NONE, currAddr); },
            [](uint16_t currAddr) { return getInstr("DEC [HL]", addressing::NONE, currAddr); },
            [](uint16_t currAddr) { return getInstr("LD [HL], 0x%02X", addressing::IMMEDIATE_8, currAddr); },
            [](uint16_t currAddr) { return getInstr("SCF", addressing::NONE, currAddr); },
            [](uint16_t currAddr) { return getInstr("JR C, e8", addressing::NONE, currAddr); },
            [](uint16_t currAddr) { return getInstr("ADD HL, SP", addressing::NONE, currAddr); },
            [](uint16_t currAddr) { return getInstr("LD A, [HL-]", addressing::NONE, currAddr); },
            [](uint16_t currAddr) { return getInstr("DEC SP", addressing::NONE, currAddr); },
            [](uint16_t currAddr) { return getInstr("INC A", addressing::NONE, currAddr); },
            [](uint16_t currAddr) { return getInstr("DEC A", addressing::NONE, currAddr); },
            [](uint16_t currAddr) { return getInstr("LD A, 0x%02X", addressing::IMMEDIATE_8, currAddr); },
            [](uint16_t currAddr) { return getInstr("CCF", addressing::NONE, currAddr); },
            [](uint16_t currAddr) { return getInstr("LD B, B", addressing::NONE, currAddr); },
            [](uint16_t currAddr) { return getInstr("LD B, C", addressing::NONE, currAddr); },
            [](uint16_t currAddr) { return getInstr("LD B, D", addressing::NONE, currAddr); },
            [](uint16_t currAddr) { return getInstr("LD B, E", addressing::NONE, currAddr); },
            [](uint16_t currAddr) { return getInstr("LD B, H", addressing::NONE, currAddr); },
            [](uint16_t currAddr) { return getInstr("LD B, L", addressing::NONE, currAddr); },
            [](uint16_t currAddr) { return getInstr("LD B, [HL]", addressing::NONE, currAddr); },
            [](uint16_t currAddr) { return getInstr("LD B, A", addressing::NONE, currAddr); },
            [](uint16_t currAddr) { return getInstr("LD C, B", addressing::NONE, currAddr); },
            [](uint16_t currAddr) { return getInstr("LD C, C", addressing::NONE, currAddr); },
            [](uint16_t currAddr) { return getInstr("LD C, D", addressing::NONE, currAddr); },
            [](uint16_t currAddr) { return getInstr("LD C, E", addressing::NONE, currAddr); },
            [](uint16_t currAddr) { return getInstr("LD C, H", addressing::NONE, currAddr); },
            [](uint16_t currAddr) { return getInstr("LD C, L", addressing::NONE, currAddr); },
            [](uint16_t currAddr) { return getInstr("LD C, [HL]", addressing::NONE, currAddr); },
            [](uint16_t currAddr) { return getInstr("LD C, A", addressing::NONE, currAddr); },
            [](uint16_t currAddr) { return getInstr("LD D, B", addressing::NONE, currAddr); },
            [](uint16_t currAddr) { return getInstr("LD D, C", addressing::NONE, currAddr); },
            [](uint16_t currAddr) { return getInstr("LD D, D", addressing::NONE, currAddr); },
            [](uint16_t currAddr) { return getInstr("LD D, E", addressing::NONE, currAddr); },
            [](uint16_t currAddr) { return getInstr("LD D, H", addressing::NONE, currAddr); },
            [](uint16_t currAddr) { return getInstr("LD D, L", addressing::NONE, currAddr); },
            [](uint16_t currAddr) { return getInstr("LD D, [HL]", addressing::NONE, currAddr); },
            [](uint16_t currAddr) { return getInstr("LD D, A", addressing::NONE, currAddr); },
            [](uint16_t currAddr) { return getInstr("LD E, B", addressing::NONE, currAddr); },
            [](uint16_t currAddr) { return getInstr("LD E, C", addressing::NONE, currAddr); },
            [](uint16_t currAddr) { return getInstr("LD E, D", addressing::NONE, currAddr); },
            [](uint16_t currAddr) { return getInstr("LD E, E", addressing::NONE, currAddr); },
            [](uint16_t currAddr) { return getInstr("LD E, H", addressing::NONE, currAddr); },
            [](uint16_t currAddr) { return getInstr("LD E, L", addressing::NONE, currAddr); },
            [](uint16_t currAddr) { return getInstr("LD E, [HL]", addressing::NONE, currAddr); },
            [](uint16_t currAddr) { return getInstr("LD E, A", addressing::NONE, currAddr); },
            [](uint16_t currAddr) { return getInstr("LD H, B", addressing::NONE, currAddr); },
            [](uint16_t currAddr) { return getInstr("LD H, C", addressing::NONE, currAddr); },
            [](uint16_t currAddr) { return getInstr("LD H, D", addressing::NONE, currAddr); },
            [](uint16_t currAddr) { return getInstr("LD H, E", addressing::NONE, currAddr); },
            [](uint16_t currAddr) { return getInstr("LD H, H", addressing::NONE, currAddr); },
            [](uint16_t currAddr) { return getInstr("LD H, L", addressing::NONE, currAddr); },
            [](uint16_t currAddr) { return getInstr("LD H, [HL]", addressing::NONE, currAddr); },
            [](uint16_t currAddr) { return getInstr("LD H, A", addressing::NONE, currAddr); },
            [](uint16_t currAddr) { return getInstr("LD L, C", addressing::NONE, currAddr); },
            [](uint16_t currAddr) { return getInstr("LD L, D", addressing::NONE, currAddr); },
            [](uint16_t currAddr) { return getInstr("LD L, B", addressing::NONE, currAddr); },
            [](uint16_t currAddr) { return getInstr("LD L, E", addressing::NONE, currAddr); },
            [](uint16_t currAddr) { return getInstr("LD L, H", addressing::NONE, currAddr); },
            [](uint16_t currAddr) { return getInstr("LD L, L", addressing::NONE, currAddr); },
            [](uint16_t currAddr) { return getInstr("LD L, [HL]", addressing::NONE, currAddr); },
            [](uint16_t currAddr) { return getInstr("LD L, A", addressing::NONE, currAddr); },
            [](uint16_t currAddr) { return getInstr("LD [HL], B", addressing::NONE, currAddr); },
            [](uint16_t currAddr) { return getInstr("LD [HL], C", addressing::NONE, currAddr); },
            [](uint16_t currAddr) { return getInstr("LD [HL], D", addressing::NONE, currAddr); },
            [](uint16_t currAddr) { return getInstr("LD [HL], E", addressing::NONE, currAddr); },
            [](uint16_t currAddr) { return getInstr("LD [HL], H", addressing::NONE, currAddr); },
            [](uint16_t currAddr) { return getInstr("LD [HL], L", addressing::NONE, currAddr); },
            [](uint16_t currAddr) { return getInstr("HALT", addressing::NONE, currAddr); },
            [](uint16_t currAddr) { return getInstr("LD [HL], A", addressing::NONE, currAddr); },
            [](uint16_t currAddr) { return getInstr("LD A, B", addressing::NONE, currAddr); },
            [](uint16_t currAddr) { return getInstr("LD A, C", addressing::NONE, currAddr); },
            [](uint16_t currAddr) { return getInstr("LD A, D", addressing::NONE, currAddr); },
            [](uint16_t currAddr) { return getInstr("LD A, E", addressing::NONE, currAddr); },
            [](uint16_t currAddr) { return getInstr("LD A, H", addressing::NONE, currAddr); },
            [](uint16_t currAddr) { return getInstr("LD A, L", addressing::NONE, currAddr); },
            [](uint16_t currAddr) { return getInstr("LD A, [HL]", addressing::NONE, currAddr); },
            [](uint16_t currAddr) { return getInstr("LD A, A", addressing::NONE, currAddr); },
            [](uint16_t currAddr) { return getInstr("ADD A, B", addressing::NONE, currAddr); },
            [](uint16_t currAddr) { return getInstr("ADD A, C", addressing::NONE, currAddr); },
            [](uint16_t currAddr) { return getInstr("ADD A, D", addressing::NONE, currAddr); },
            [](uint16_t currAddr) { return getInstr("ADD A, E", addressing::NONE, currAddr); },
            [](uint16_t currAddr) { return getInstr("ADD A, H", addressing::NONE, currAddr); },
            [](uint16_t currAddr) { return getInstr("ADD A, L", addressing::NONE, currAddr); },
            [](uint16_t currAddr) { return getInstr("ADD A, [HL]", addressing::NONE, currAddr); },
            [](uint16_t currAddr) { return getInstr("ADD A, A", addressing::NONE, currAddr); },
            [](uint16_t currAddr) { return getInstr("ADC A, B", addressing::NONE, currAddr); },
            [](uint16_t currAddr) { return getInstr("ADC A, C", addressing::NONE, currAddr); },
            [](uint16_t currAddr) { return getInstr("ADC A, D", addressing::NONE, currAddr); },
            [](uint16_t currAddr) { return getInstr("ADC A, E", addressing::NONE, currAddr); },
            [](uint16_t currAddr) { return getInstr("ADC A, H", addressing::NONE, currAddr); },
            [](uint16_t currAddr) { return getInstr("ADC A, L", addressing::NONE, currAddr); },
            [](uint16_t currAddr) { return getInstr("ADC A, [HL]", addressing::NONE, currAddr); },
            [](uint16_t currAddr) { return getInstr("ADC A, A", addressing::NONE, currAddr); },
            [](uint16_t currAddr) { return getInstr("SUB A, B", addressing::NONE, currAddr); },
            [](uint16_t currAddr) { return getInstr("SUB A, C", addressing::NONE, currAddr); },
            [](uint16_t currAddr) { return getInstr("SUB A, D", addressing::NONE, currAddr); },
            [](uint16_t currAddr) { return getInstr("SUB A, E", addressing::NONE, currAddr); },
            [](uint16_t currAddr) { return getInstr("SUB A, H", addressing::NONE, currAddr); },
            [](uint16_t currAddr) { return getInstr("SUB A, L", addressing::NONE, currAddr); },
            [](uint16_t currAddr) { return getInstr("SUB A, [HL]", addressing::NONE, currAddr); },
            [](uint16_t currAddr) { return getInstr("SUB A, A", addressing::NONE, currAddr); },
            [](uint16_t currAddr) { return getInstr("SBC A, B", addressing::NONE, currAddr); },
            [](uint16_t currAddr) { return getInstr("SBC A, C", addressing::NONE, currAddr); },
            [](uint16_t currAddr) { return getInstr("SBC A, D", addressing::NONE, currAddr); },
            [](uint16_t currAddr) { return getInstr("SBC A, E", addressing::NONE, currAddr); },
            [](uint16_t currAddr) { return getInstr("SBC A, H", addressing::NONE, currAddr); },
            [](uint16_t currAddr) { return getInstr("SBC A, L", addressing::NONE, currAddr); },
            [](uint16_t currAddr) { return getInstr("SBC A, [HL]", addressing::NONE, currAddr); },
            [](uint16_t currAddr) { return getInstr("SBC A, A", addressing::NONE, currAddr); },
            [](uint16_t currAddr) { return getInstr("AND A, B", addressing::NONE, currAddr); },
            [](uint16_t currAddr) { return getInstr("AND A, C", addressing::NONE, currAddr); },
            [](uint16_t currAddr) { return getInstr("AND A, D", addressing::NONE, currAddr); },
            [](uint16_t currAddr) { return getInstr("AND A, E", addressing::NONE, currAddr); },
            [](uint16_t currAddr) { return getInstr("AND A, H", addressing::NONE, currAddr); },
            [](uint16_t currAddr) { return getInstr("AND A, L", addressing::NONE, currAddr); },
            [](uint16_t currAddr) { return getInstr("AND A, [HL]", addressing::NONE, currAddr); },
            [](uint16_t currAddr) { return getInstr("AND A, A", addressing::NONE, currAddr); },
            [](uint16_t currAddr) { return getInstr("XOR A, B", addressing::NONE, currAddr); },
            [](uint16_t currAddr) { return getInstr("XOR A, C", addressing::NONE, currAddr); },
            [](uint16_t currAddr) { return getInstr("XOR A, D", addressing::NONE, currAddr); },
            [](uint16_t currAddr) { return getInstr("XOR A, E", addressing::NONE, currAddr); },
            [](uint16_t currAddr) { return getInstr("XOR A, H", addressing::NONE, currAddr); },
            [](uint16_t currAddr) { return getInstr("XOR A, L", addressing::NONE, currAddr); },
            [](uint16_t currAddr) { return getInstr("XOR A, [HL]", addressing::NONE, currAddr); },
            [](uint16_t currAddr) { return getInstr("XOR A, A", addressing::NONE, currAddr); },
            [](uint16_t currAddr) { return getInstr("OR A, B", addressing::NONE, currAddr); },
            [](uint16_t currAddr) { return getInstr("OR A, C", addressing::NONE, currAddr); },
            [](uint16_t currAddr) { return getInstr("OR A, D", addressing::NONE, currAddr); },
            [](uint16_t currAddr) { return getInstr("OR A, E", addressing::NONE, currAddr); },
            [](uint16_t currAddr) { return getInstr("OR A, H", addressing::NONE, currAddr); },
            [](uint16_t currAddr) { return getInstr("OR A, L", addressing::NONE, currAddr); },
            [](uint16_t currAddr) { return getInstr("OR A, [HL]", addressing::NONE, currAddr); },
            [](uint16_t currAddr) { return getInstr("OR A, A", addressing::NONE, currAddr); },
            [](uint16_t currAddr) { return getInstr("CP A, B", addressing::NONE, currAddr); },
            [](uint16_t currAddr) { return getInstr("CP A, C", addressing::NONE, currAddr); },
            [](uint16_t currAddr) { return getInstr("CP A, D", addressing::NONE, currAddr); },
            [](uint16_t currAddr) { return getInstr("CP A, E", addressing::NONE, currAddr); },
            [](uint16_t currAddr) { return getInstr("CP A, H", addressing::NONE, currAddr); },
            [](uint16_t currAddr) { return getInstr("CP A, L", addressing::NONE, currAddr); },
            [](uint16_t currAddr) { return getInstr("CP A, [HL]", addressing::NONE, currAddr); },
            [](uint16_t currAddr) { return getInstr("CP A, A", addressing::NONE, currAddr); },
            [](uint16_t currAddr) { return getInstr("RET NZ", addressing::NONE, currAddr); },
            [](uint16_t currAddr) { return getInstr("POP BC", addressing::NONE, currAddr); },
            [](uint16_t currAddr) { return getInstr("JP NZ, 0x%04X", addressing::IMMEDIATE_16, currAddr); },
            [](uint16_t currAddr) { return getInstr("JP 0x%04X", addressing::IMMEDIATE_16, currAddr); },
            [](uint16_t currAddr) { return getInstr("CALL NZ, 0x%04X", addressing::IMMEDIATE_16, currAddr); },
            [](uint16_t currAddr) { return getInstr("PUSH BC", addressing::NONE, currAddr); },
            [](uint16_t currAddr) { return getInstr("ADD A, 0x%02X", addressing::IMMEDIATE_8, currAddr); },
            [](uint16_t currAddr) { return getInstr("RST 0x0000", addressing::NONE, currAddr); },
            [](uint16_t currAddr) { return getInstr("RET Z", addressing::NONE, currAddr); },
            [](uint16_t currAddr) { return getInstr("RET", addressing::NONE, currAddr); },
            [](uint16_t currAddr) { return getInstr("JP Z, 0x%04X", addressing::IMMEDIATE_16, currAddr); },
            [](uint16_t currAddr) { return getInstr("PREFIX", addressing::NONE, currAddr); },
            [](uint16_t currAddr) { return getInstr("CALL Z, 0x%04X", addressing::IMMEDIATE_16, currAddr); },
            [](uint16_t currAddr) { return getInstr("CALL 0x%04X", addressing::IMMEDIATE_16, currAddr); },
            [](uint16_t currAddr) { return getInstr("ADC A, 0x%02X", addressing::IMMEDIATE_8, currAddr); },
            [](uint16_t currAddr) { return getInstr("RST 0x0008", addressing::NONE, currAddr); },
            [](uint16_t currAddr) { return getInstr("RET NC", addressing::NONE, currAddr); },
            [](uint16_t currAddr) { return getInstr("POP DE", addressing::NONE, currAddr); },
            [](uint16_t currAddr) { return getInstr("JP NC, 0x%04X", addressing::IMMEDIATE_16, currAddr); },
            [](uint16_t currAddr) { return getInstr("NIMP", addressing::NONE, currAddr); },
            [](uint16_t currAddr) { return getInstr("CALL NC, 0x%04X", addressing::IMMEDIATE_16, currAddr); },
            [](uint16_t currAddr) { return getInstr("PUSH DE", addressing::NONE, currAddr); },
            [](uint16_t currAddr) { return getInstr("SUB A, 0x%02X", addressing::IMMEDIATE_8, currAddr); },
            [](uint16_t currAddr) { return getInstr("RST 0x0010", addressing::NONE, currAddr); },
            [](uint16_t currAddr) { return getInstr("RET C", addressing::NONE, currAddr); },
            [](uint16_t currAddr) { return getInstr("RETI", addressing::NONE, currAddr); },
            [](uint16_t currAddr) { return getInstr("JP C, 0x%04X", addressing::IMMEDIATE_16, currAddr); },
            [](uint16_t currAddr) { return getInstr("NIMP", addressing::NONE, currAddr); },
            [](uint16_t currAddr) { return getInstr("CALL C, 0x%04X", addressing::IMMEDIATE_16, currAddr); },
            [](uint16_t currAddr) { return getInstr("NIMP", addressing::NONE, currAddr); },
            [](uint16_t currAddr) { return getInstr("SBC A, 0x%02X", addressing::IMMEDIATE_8, currAddr); },
            [](uint16_t currAddr) { return getInstr("RST 0x0018", addressing::NONE, currAddr); },
            [](uint16_t currAddr) { return getInstr("LDH [a8], A", addressing::NONE, currAddr); },
            [](uint16_t currAddr) { return getInstr("POP HL", addressing::NONE, currAddr); },
            [](uint16_t currAddr) { return getInstr("LD [C], A", addressing::NONE, currAddr); },
            [](uint16_t currAddr) { return getInstr("NIMP", addressing::NONE, currAddr); },
            [](uint16_t currAddr) { return getInstr("NIMP", addressing::NONE, currAddr); },
            [](uint16_t currAddr) { return getInstr("PUSH HL", addressing::NONE, currAddr); },
            [](uint16_t currAddr) { return getInstr("AND A, 0x%02X", addressing::IMMEDIATE_8, currAddr); },
            [](uint16_t currAddr) { return getInstr("RST 0x0020", addressing::NONE, currAddr); },
            [](uint16_t currAddr) { return getInstr("ADD SP, e8", addressing::NONE, currAddr); },
            [](uint16_t currAddr) { return getInstr("JP HL", addressing::NONE, currAddr); },
            [](uint16_t currAddr) { return getInstr("LD [0x%04X], A", addressing::IMMEDIATE_16, currAddr); },
            [](uint16_t currAddr) { return getInstr("NIMP", addressing::NONE, currAddr); },
            [](uint16_t currAddr) { return getInstr("NIMP", addressing::NONE, currAddr); },
            [](uint16_t currAddr) { return getInstr("NIMP", addressing::NONE, currAddr); },
            [](uint16_t currAddr) { return getInstr("XOR A, 0x%02X", addressing::IMMEDIATE_8, currAddr); },
            [](uint16_t currAddr) { return getInstr("RST 0x0028", addressing::NONE, currAddr); },
            [](uint16_t currAddr) { return getInstr("LDH A, [a8]", addressing::NONE, currAddr); },
            [](uint16_t currAddr) { return getInstr("POP AF", addressing::NONE, currAddr); },
            [](uint16_t currAddr) { return getInstr("LD A, [C]", addressing::NONE, currAddr); },
            [](uint16_t currAddr) { return getInstr("DI", addressing::NONE, currAddr); },
            [](uint16_t currAddr) { return getInstr("NIMP", addressing::NONE, currAddr); },
            [](uint16_t currAddr) { return getInstr("PUSH AF", addressing::NONE, currAddr); },
            [](uint16_t currAddr) { return getInstr("OR A, 0x%02X", addressing::IMMEDIATE_8, currAddr); },
            [](uint16_t currAddr) { return getInstr("RST 0x0030", addressing::NONE, currAddr); },
            [](uint16_t currAddr) { return getInstr("LD HL, SP + e8", addressing::NONE, currAddr); },
            [](uint16_t currAddr) { return getInstr("LD SP, HL", addressing::NONE, currAddr); },
            [](uint16_t currAddr) { return getInstr("LD A, [0x%04X]", addressing::IMMEDIATE_16, currAddr); },
            [](uint16_t currAddr) { return getInstr("EI", addressing::NONE, currAddr); },
            [](uint16_t currAddr) { return getInstr("NIMP", addressing::NONE, currAddr); },
            [](uint16_t currAddr) { return getInstr("NIMP", addressing::NONE, currAddr); },
            [](uint16_t currAddr) { return getInstr("CP A, 0x%02X", addressing::IMMEDIATE_8, currAddr); },
            [](uint16_t currAddr) { return getInstr("RST 0x0038", addressing::NONE, currAddr); }
    };
};


#endif //DEMUG_CODEVIEWER_H
