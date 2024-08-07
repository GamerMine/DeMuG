/*
 *  ____
 * /\  _`\                                       /'\_/`\  __
 * \ \ \L\_\     __      ___ ___      __   _ __ /\      \/\_\    ___      __
 *  \ \ \L_L   /'__`\  /' __` __`\  /'__`\/\`'__\ \ \__\ \/\ \ /' _ `\  /'__`\
 *   \ \ \/, \/\ \L\.\_/\ \/\ \/\ \/\  __/\ \ \/ \ \ \_/\ \ \ \/\ \/\ \/\  __/
 *    \ \____/\ \__/.\_\ \_\ \_\ \_\ \____\\ \_\  \ \_\\ \_\ \_\ \_\ \_\ \____\
 *     \/___/  \/__/\/_/\/_/\/_/\/_/\/____/ \/_/   \/_/ \/_/\/_/\/_/\/_/\/____/
 *
 * Copyright (c) 2023-2024 GamerMine <maxime-sav@outlook.fr>
 *
 * This Source Code From is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/ .
 */

#ifndef EMU_GAMEBOY_SHARPSM83_H
#define EMU_GAMEBOY_SHARPSM83_H

#include <cstdint>
#include <cstdio>
#include <thread>
#include <vector>
#include <string>

#include "Bus.h"
#include "../logging/Logger.h"

class SharpSM83 {
public:
    explicit SharpSM83(class Bus *bus);

    void reset();
    void runCpu();

    inline static union {
        struct {
            bool vblank : 1;
            bool lcd    : 1;
            bool timer  : 1;
            bool serial : 1;
            bool joypad : 1;
            bool unused : 3;
        };
        uint8_t raw;
    } IE {}; // Interrupt Enable controls handlers calls

    inline static union {
        struct {
            bool vblank : 1;
            bool lcd    : 1;
            bool timer  : 1;
            bool serial : 1;
            bool joypad : 1;
            bool unused : 3;
        };
        uint8_t raw;
    } IF {}; // Interrupt flag controls handlers request

public:
    inline static union {
        struct {
            bool unused1;   // Bit 0
            bool unused2;   // Bit 1
            bool unused3;   // Bit 2
            bool unused4;   // Bit 3
            bool carry;     // Bit 4
            bool halfCarry; // Bit 5
            bool negative;  // Bit 6
            bool zero;      // Bit 7
        };
        uint8_t rawFlags;
    } flags {};

    inline static union {
        union {
            struct {
                uint8_t C;
                uint8_t B;
                uint8_t E;
                uint8_t D;
                uint8_t L;
                uint8_t H;
                uint8_t unused;
                uint8_t A;
            };
            struct {
                uint16_t BC;
                uint16_t DE;
                uint16_t HL;
                uint16_t unused2;
            };
        };
        uint8_t rawRegisters[8];
    } registers{};

    inline static std::atomic<uint16_t> PC;
    inline static uint16_t SP;
    inline static bool IME; // Interrupt master enable flag
    inline static uint8_t interruptShouldBeEnabled;
    Bus *mBus;
    static Logger *logger;
    bool haltBug;
    uint8_t haltInstr{0x00};
    int executedCycles{0};

    bool checkInterrupts(bool executeHandler = true);

private:
    inline static std::chrono::time_point start = std::chrono::high_resolution_clock::now();
    using opcodeFunc = std::function<uint8_t()>;
    opcodeFunc opcodes[256] = {
            /*00*/[this]() {return NOP();},                               // OK
            /*01*/[this]() {return LD(&registers.BC, nullptr);},          // OK
            /*02*/[this]() {return LD(registers.BC, &registers.A);},      // OK
            /*03*/[this]() {return INC(&registers.BC);},                  // OK
            /*04*/[this]() {return INC(&registers.B);},
            /*05*/[this]() {return DEC(&registers.B);},
            /*06*/[this]() {return LD(&registers.B, nullptr);},           // OK
            /*07*/[this]() {return RLCA();},
            /*08*/[this]() {return LD(nullptr, &SP);},                    // OK
            /*09*/[this]() {return ADD(&registers.HL, &registers.BC);},
            /*0A*/[this]() {return LD(&registers.A, registers.BC);},      // OK
            /*0B*/[this]() {return DEC(&registers.BC);},                  // OK
            /*0C*/[this]() {return INC(&registers.C);},
            /*0D*/[this]() {return DEC(&registers.C);},
            /*0E*/[this]() {return LD(&registers.C, nullptr);},           // OK
            /*0F*/[this]() {return RRCA();},
            /*10*/[this]() {return STOP();},
            /*11*/[this]() {return LD(&registers.DE, nullptr);},          // OK
            /*12*/[this]() {return LD(registers.DE, &registers.A);},      // OK
            /*13*/[this]() {return INC(&registers.DE);},                  // OK
            /*14*/[this]() {return INC(&registers.D);},
            /*15*/[this]() {return DEC(&registers.D);},
            /*16*/[this]() {return LD(&registers.D, nullptr);},           // OK
            /*17*/[this]() {return RLA();},
            /*18*/[this]() {return JR(nullptr);},                         // OK
            /*19*/[this]() {return ADD(&registers.HL, &registers.DE);},
            /*1A*/[this]() {return LD(&registers.A, registers.DE);},      // OK
            /*1B*/[this]() {return DEC(&registers.DE);},                  // OK
            /*1C*/[this]() {return INC(&registers.E);},
            /*1D*/[this]() {return DEC(&registers.E);},
            /*1E*/[this]() {return LD(&registers.E, nullptr);},           // OK
            /*1F*/[this]() {return RRA();},
            /*20*/[this]() {bool inverted = !flags.zero; return JR(&inverted);},
            /*21*/[this]() {return LD(&registers.HL, nullptr);},          // OK
            /*22*/[this]() {return LD(registers.HL, &registers.A, true, false);}, // OK
            /*23*/[this]() {return INC(&registers.HL);},                  // OK
            /*24*/[this]() {return INC(&registers.H);},
            /*25*/[this]() {return DEC(&registers.H);},
            /*26*/[this]() {return LD(&registers.H, nullptr);},           // OK
            /*27*/[this]() {return DAA();},
            /*28*/[this]() {return JR(&flags.zero);},
            /*29*/[this]() {return ADD(&registers.HL, &registers.HL);},
            /*2A*/[this]() {return LD(&registers.A, registers.HL, true, false);}, // OK
            /*2B*/[this]() {return DEC(&registers.HL);},                  // OK
            /*2C*/[this]() {return INC(&registers.L);},
            /*2D*/[this]() {return DEC(&registers.L);},
            /*2E*/[this]() {return LD(&registers.L, nullptr);},           // OK
            /*2F*/[this]() {return CPL();},
            /*30*/[this]() {bool inverted = !flags.carry; return JR(&inverted);},
            /*31*/[this]() {return LD(&SP, nullptr);},                     // OK
            /*32*/[this]() {return LD(registers.HL, &registers.A, false, true);}, // OK
            /*33*/[this]() {return INC(&SP);},                             // OK
            /*34*/[this]() {return INC(registers.HL);},
            /*35*/[this]() {return DEC(registers.HL);},
            /*36*/[this]() {return LD(registers.HL, nullptr);},            // OK
            /*37*/[this]() {return SCF();},
            /*38*/[this]() {return JR(&flags.carry);},
            /*39*/[this]() {return ADD(&registers.HL, &SP);},
            /*3A*/[this]() {return LD(&registers.A, registers.HL, false, true);}, // OK
            /*3B*/[this]() {return DEC(&SP);},                            // OK
            /*3C*/[this]() {return INC(&registers.A);},
            /*3D*/[this]() {return DEC(&registers.A);},
            /*3E*/[this]() {return LD(&registers.A, nullptr);},           // OK
            /*3F*/[this]() {return CCF();},
            /*40*/[this]() {return LD(&registers.B, &registers.B);},      // OK
            /*41*/[this]() {return LD(&registers.B, &registers.C);},      // OK
            /*42*/[this]() {return LD(&registers.B, &registers.D);},      // OK
            /*43*/[this]() {return LD(&registers.B, &registers.E);},      // OK
            /*44*/[this]() {return LD(&registers.B, &registers.H);},      // OK
            /*45*/[this]() {return LD(&registers.B, &registers.L);},      // OK
            /*46*/[this]() {return LD(&registers.B, registers.HL);},      // OK
            /*47*/[this]() {return LD(&registers.B, &registers.A);},      // OK
            /*48*/[this]() {return LD(&registers.C, &registers.B);},      // OK
            /*49*/[this]() {return LD(&registers.C, &registers.C);},      // OK
            /*4A*/[this]() {return LD(&registers.C, &registers.D);},      // OK
            /*4B*/[this]() {return LD(&registers.C, &registers.E);},      // OK
            /*4C*/[this]() {return LD(&registers.C, &registers.H);},      // OK
            /*4D*/[this]() {return LD(&registers.C, &registers.L);},      // OK
            /*4E*/[this]() {return LD(&registers.C, registers.HL);},      // OK
            /*4F*/[this]() {return LD(&registers.C, &registers.A);},      // OK
            /*50*/[this]() {return LD(&registers.D, &registers.B);},      // OK
            /*51*/[this]() {return LD(&registers.D, &registers.C);},      // OK
            /*52*/[this]() {return LD(&registers.D, &registers.D);},      // OK
            /*53*/[this]() {return LD(&registers.D, &registers.E);},      // OK
            /*54*/[this]() {return LD(&registers.D, &registers.H);},      // OK
            /*55*/[this]() {return LD(&registers.D, &registers.L);},      // OK
            /*56*/[this]() {return LD(&registers.D, registers.HL);},      // OK
            /*57*/[this]() {return LD(&registers.D, &registers.A);},      // OK
            /*58*/[this]() {return LD(&registers.E, &registers.B);},      // OK
            /*59*/[this]() {return LD(&registers.E, &registers.C);},      // OK
            /*5A*/[this]() {return LD(&registers.E, &registers.D);},      // OK
            /*5B*/[this]() {return LD(&registers.E, &registers.E);},      // OK
            /*5C*/[this]() {return LD(&registers.E, &registers.H);},      // OK
            /*5D*/[this]() {return LD(&registers.E, &registers.L);},      // OK
            /*5E*/[this]() {return LD(&registers.E, registers.HL);},      // OK
            /*5F*/[this]() {return LD(&registers.E, &registers.A);},      // OK
            /*60*/[this]() {return LD(&registers.H, &registers.B);},      // OK
            /*61*/[this]() {return LD(&registers.H, &registers.C);},      // OK
            /*62*/[this]() {return LD(&registers.H, &registers.D);},      // OK
            /*63*/[this]() {return LD(&registers.H, &registers.E);},      // OK
            /*64*/[this]() {return LD(&registers.H, &registers.H);},      // OK
            /*65*/[this]() {return LD(&registers.H, &registers.L);},      // OK
            /*66*/[this]() {return LD(&registers.H, registers.HL);},      // OK
            /*67*/[this]() {return LD(&registers.H, &registers.A);},      // OK
            /*68*/[this]() {return LD(&registers.L, &registers.B);},      // OK
            /*69*/[this]() {return LD(&registers.L, &registers.C);},      // OK
            /*6A*/[this]() {return LD(&registers.L, &registers.D);},      // OK
            /*6B*/[this]() {return LD(&registers.L, &registers.E);},      // OK
            /*6C*/[this]() {return LD(&registers.L, &registers.H);},      // OK
            /*6D*/[this]() {return LD(&registers.L, &registers.L);},      // OK
            /*6E*/[this]() {return LD(&registers.L, registers.HL);},      // OK
            /*6F*/[this]() {return LD(&registers.L, &registers.A);},      // OK
            /*70*/[this]() {return LD(registers.HL, &registers.B);},      // OK
            /*71*/[this]() {return LD(registers.HL, &registers.C);},      // OK
            /*72*/[this]() {return LD(registers.HL, &registers.D);},      // OK
            /*73*/[this]() {return LD(registers.HL, &registers.E);},      // OK
            /*74*/[this]() {return LD(registers.HL, &registers.H);},      // OK
            /*75*/[this]() {return LD(registers.HL, &registers.L);},      // OK
            /*76*/[this]() {return HALT();},
            /*77*/[this]() {return LD(registers.HL, &registers.A);},      // OK
            /*78*/[this]() {return LD(&registers.A, &registers.B);},      // OK
            /*79*/[this]() {return LD(&registers.A, &registers.C);},      // OK
            /*7A*/[this]() {return LD(&registers.A, &registers.D);},      // OK
            /*7B*/[this]() {return LD(&registers.A, &registers.E);},      // OK
            /*7C*/[this]() {return LD(&registers.A, &registers.H);},      // OK
            /*7D*/[this]() {return LD(&registers.A, &registers.L);},      // OK
            /*7E*/[this]() {return LD(&registers.A, registers.HL);},      // OK
            /*7F*/[this]() {return LD(&registers.A, &registers.A);},      // OK
            /*80*/[this]() {return ADD(&registers.B);},
            /*81*/[this]() {return ADD(&registers.C);},
            /*82*/[this]() {return ADD(&registers.D);},
            /*83*/[this]() {return ADD(&registers.E);},
            /*84*/[this]() {return ADD(&registers.H);},
            /*85*/[this]() {return ADD(&registers.L);},
            /*86*/[this]() {return ADD(registers.HL);},
            /*87*/[this]() {return ADD(&registers.A);},
            /*88*/[this]() {return ADC(&registers.B);},
            /*89*/[this]() {return ADC(&registers.C);},
            /*8A*/[this]() {return ADC(&registers.D);},
            /*8B*/[this]() {return ADC(&registers.E);},
            /*8C*/[this]() {return ADC(&registers.H);},
            /*8D*/[this]() {return ADC(&registers.L);},
            /*8E*/[this]() {return ADC(registers.HL);},
            /*8F*/[this]() {return ADC(&registers.A);},
            /*90*/[this]() {return SUB(&registers.B);},
            /*91*/[this]() {return SUB(&registers.C);},
            /*92*/[this]() {return SUB(&registers.D);},
            /*93*/[this]() {return SUB(&registers.E);},
            /*94*/[this]() {return SUB(&registers.H);},
            /*95*/[this]() {return SUB(&registers.L);},
            /*96*/[this]() {return SUB(registers.HL);},
            /*97*/[this]() {return SUB(&registers.A);},
            /*98*/[this]() {return SBC(&registers.B);},
            /*99*/[this]() {return SBC(&registers.C);},
            /*9A*/[this]() {return SBC(&registers.D);},
            /*9B*/[this]() {return SBC(&registers.E);},
            /*9C*/[this]() {return SBC(&registers.H);},
            /*9D*/[this]() {return SBC(&registers.L);},
            /*9E*/[this]() {return SBC(registers.HL);},
            /*9F*/[this]() {return SBC(&registers.A);},
            /*A0*/[this]() {return AND(&registers.B);},
            /*A1*/[this]() {return AND(&registers.C);},
            /*A2*/[this]() {return AND(&registers.D);},
            /*A3*/[this]() {return AND(&registers.E);},
            /*A4*/[this]() {return AND(&registers.H);},
            /*A5*/[this]() {return AND(&registers.L);},
            /*A6*/[this]() {return AND();},
            /*A7*/[this]() {return AND(&registers.A);},
            /*A8*/[this]() {return XOR(&registers.B);},
            /*A9*/[this]() {return XOR(&registers.C);},
            /*AA*/[this]() {return XOR(&registers.D);},
            /*AB*/[this]() {return XOR(&registers.E);},
            /*AC*/[this]() {return XOR(&registers.H);},
            /*AD*/[this]() {return XOR(&registers.L);},
            /*AE*/[this]() {return XOR();},
            /*AF*/[this]() {return XOR(&registers.A);},
            /*B0*/[this]() {return OR(&registers.B);},
            /*B1*/[this]() {return OR(&registers.C);},
            /*B2*/[this]() {return OR(&registers.D);},
            /*B3*/[this]() {return OR(&registers.E);},
            /*B4*/[this]() {return OR(&registers.H);},
            /*B5*/[this]() {return OR(&registers.L);},
            /*B6*/[this]() {return OR();},
            /*B7*/[this]() {return OR(&registers.A);},
            /*B8*/[this]() {return CP(&registers.B);},
            /*B9*/[this]() {return CP(&registers.C);},
            /*BA*/[this]() {return CP(&registers.D);},
            /*BB*/[this]() {return CP(&registers.E);},
            /*BC*/[this]() {return CP(&registers.H);},
            /*BD*/[this]() {return CP(&registers.L);},
            /*BE*/[this]() {return CP(registers.HL);},
            /*BF*/[this]() {return CP(&registers.A);},
            /*C0*/[this]() {bool inverted = !flags.zero; return RET(&inverted);},
            /*C1*/[this]() {return POP(&registers.BC);},                  // OK
            /*C2*/[this]() {bool inverted = !flags.zero; return JP(&inverted);},
            /*C3*/[this]() {return JP(nullptr, false);},                  // OK
            /*C4*/[this]() {bool inverted = !flags.zero; return CALL(&inverted);},
            /*C5*/[this]() {return PUSH(&registers.BC);},                 // OK
            /*C6*/[this]() {return ADD(nullptr);},
            /*C7*/[this]() {return RST(0x0000);},                         // OK
            /*C8*/[this]() {return RET(&flags.zero);},
            /*C9*/[this]() {return RET(nullptr);},                        // OK
            /*CA*/[this]() {return JP(&flags.zero);},
            /*CB*/[this]() {return PREFIX();},
            /*CC*/[this]() {return CALL(&flags.zero);},
            /*CD*/[this]() {return CALL(nullptr);},                       // OK
            /*CE*/[this]() {return ADC(nullptr);},
            /*CF*/[this]() {return RST(0x0008);},                         // OK
            /*D0*/[this]() {bool inverted = !flags.carry; return RET(&inverted);},
            /*D1*/[this]() {return POP(&registers.DE);},                  // OK
            /*D2*/[this]() {bool inverted = !flags.carry; return JP(&inverted);},
            /*D3*/[this]() {return NIMP(0xD3);},
            /*D4*/[this]() {bool inverted = !flags.carry; return CALL(&inverted);},
            /*D5*/[this]() {return PUSH(&registers.DE);},                 // OK
            /*D6*/[this]() {return SUB(nullptr);},
            /*D7*/[this]() {return RST(0x0010);},                         // OK
            /*D8*/[this]() {return RET(&flags.carry);},
            /*D9*/[this]() {return RETI();},                              // OK
            /*DA*/[this]() {return JP(&flags.carry);},
            /*DB*/[this]() {return NIMP(0xDB);},
            /*DC*/[this]() {return CALL(&flags.carry);},
            /*DD*/[this]() {return NIMP(0xDD);},
            /*DE*/[this]() {return SBC(nullptr);},
            /*DF*/[this]() {return RST(0x0018);},                         // OK
            /*E0*/[this]() {return LDH(nullptr);},                        // OK
            /*E1*/[this]() {return POP(&registers.HL);},                  // OK
            /*E2*/[this]() {return LD(registers.C, &registers.A);},       // OK
            /*E3*/[this]() {return NIMP(0xE3);},
            /*E4*/[this]() {return NIMP(0xE4);},
            /*E5*/[this]() {return PUSH(&registers.HL);},                 // OK
            /*E6*/[this]() {return AND(nullptr);},
            /*E7*/[this]() {return RST(0x0020);},                         // OK
            /*E8*/[this]() {return ADD(&SP, nullptr);},
            /*E9*/[this]() {return JP(&registers.HL);},
            /*EA*/[this]() {return LD(nullptr);},                         // OK
            /*EB*/[this]() {return NIMP(0xEB);},
            /*EC*/[this]() {return NIMP(0xEC);},
            /*ED*/[this]() {return NIMP(0xED);},
            /*EE*/[this]() {return XOR(nullptr);},
            /*EF*/[this]() {return RST(0x0028);},                         // OK
            /*F0*/[this]() {return LDH(&registers.A);},                   // OK
            /*F1*/[this]() {return POP(nullptr);},                        // OK
            /*F2*/[this]() {return LD(&registers.A, registers.C);},       // OK
            /*F3*/[this]() {return DI();},
            /*F4*/[this]() {return NIMP(0xF4);},
            /*F5*/[this]() {return PUSH(nullptr);},                       // OK
            /*F6*/[this]() {return OR(nullptr);},
            /*F7*/[this]() {return RST(0x0030);},                         // OK
            /*F8*/[this]() {return LD(&registers.HL, &SP, true);},
            /*F9*/[this]() {return LD(&SP, &registers.HL);},              // OK
            /*FA*/[this]() {return LD(&registers.A);},                    // OK
            /*FB*/[this]() {return EI();},
            /*FC*/[this]() {return NIMP(0xFC);},
            /*FD*/[this]() {return NIMP(0xFD);},
            /*FE*/[this]() {return CP(nullptr);},
            /*FF*/[this]() {return RST(0x0038);},                         // OK
    };

    opcodeFunc pfxOpcodes[256] = {
        [this]() {return RLC(&registers.B);},
        [this]() {return RLC(&registers.C);},
        [this]() {return RLC(&registers.D);},
        [this]() {return RLC(&registers.E);},
        [this]() {return RLC(&registers.H);},
        [this]() {return RLC(&registers.L);},
        [this]() {return RLC(nullptr);},
        [this]() {return RLC(&registers.A);},
        [this]() {return RRC(&registers.B);},
        [this]() {return RRC(&registers.C);},
        [this]() {return RRC(&registers.D);},
        [this]() {return RRC(&registers.E);},
        [this]() {return RRC(&registers.H);},
        [this]() {return RRC(&registers.L);},
        [this]() {return RRC(nullptr);},
        [this]() {return RRC(&registers.A);},
        [this]() {return RL(&registers.B);},
        [this]() {return RL(&registers.C);},
        [this]() {return RL(&registers.D);},
        [this]() {return RL(&registers.E);},
        [this]() {return RL(&registers.H);},
        [this]() {return RL(&registers.L);},
        [this]() {return RL(nullptr);},
        [this]() {return RL(&registers.A);},
        [this]() {return RR(&registers.B);},
        [this]() {return RR(&registers.C);},
        [this]() {return RR(&registers.D);},
        [this]() {return RR(&registers.E);},
        [this]() {return RR(&registers.H);},
        [this]() {return RR(&registers.L);},
        [this]() {return RR(nullptr);},
        [this]() {return RR(&registers.A);},
        [this]() {return SLA(&registers.B);},
        [this]() {return SLA(&registers.C);},
        [this]() {return SLA(&registers.D);},
        [this]() {return SLA(&registers.E);},
        [this]() {return SLA(&registers.H);},
        [this]() {return SLA(&registers.L);},
        [this]() {return SLA(nullptr);},
        [this]() {return SLA(&registers.A);},
        [this]() {return SRA(&registers.B);},
        [this]() {return SRA(&registers.C);},
        [this]() {return SRA(&registers.D);},
        [this]() {return SRA(&registers.E);},
        [this]() {return SRA(&registers.H);},
        [this]() {return SRA(&registers.L);},
        [this]() {return SRA(nullptr);},
        [this]() {return SRA(&registers.A);},
        [this]() {return SWAP(&registers.B);},
        [this]() {return SWAP(&registers.C);},
        [this]() {return SWAP(&registers.D);},
        [this]() {return SWAP(&registers.E);},
        [this]() {return SWAP(&registers.H);},
        [this]() {return SWAP(&registers.L);},
        [this]() {return SWAP(nullptr);},
        [this]() {return SWAP(&registers.A);},
        [this]() {return SRL(&registers.B);},
        [this]() {return SRL(&registers.C);},
        [this]() {return SRL(&registers.D);},
        [this]() {return SRL(&registers.E);},
        [this]() {return SRL(&registers.H);},
        [this]() {return SRL(&registers.L);},
        [this]() {return SRL(nullptr);},
        [this]() {return SRL(&registers.A);},
        [this]() {return BIT(0, &registers.B);},
        [this]() {return BIT(0, &registers.C);},
        [this]() {return BIT(0, &registers.D);},
        [this]() {return BIT(0, &registers.E);},
        [this]() {return BIT(0, &registers.H);},
        [this]() {return BIT(0, &registers.L);},
        [this]() {return BIT(0, nullptr);},
        [this]() {return BIT(0, &registers.A);},
        [this]() {return BIT(1, &registers.B);},
        [this]() {return BIT(1, &registers.C);},
        [this]() {return BIT(1, &registers.D);},
        [this]() {return BIT(1, &registers.E);},
        [this]() {return BIT(1, &registers.H);},
        [this]() {return BIT(1, &registers.L);},
        [this]() {return BIT(1, nullptr);},
        [this]() {return BIT(1, &registers.A);},
        [this]() {return BIT(2, &registers.B);},
        [this]() {return BIT(2, &registers.C);},
        [this]() {return BIT(2, &registers.D);},
        [this]() {return BIT(2, &registers.E);},
        [this]() {return BIT(2, &registers.H);},
        [this]() {return BIT(2, &registers.L);},
        [this]() {return BIT(2, nullptr);},
        [this]() {return BIT(2, &registers.A);},
        [this]() {return BIT(3, &registers.B);},
        [this]() {return BIT(3, &registers.C);},
        [this]() {return BIT(3, &registers.D);},
        [this]() {return BIT(3, &registers.E);},
        [this]() {return BIT(3, &registers.H);},
        [this]() {return BIT(3, &registers.L);},
        [this]() {return BIT(3, nullptr);},
        [this]() {return BIT(3, &registers.A);},
        [this]() {return BIT(4, &registers.B);},
        [this]() {return BIT(4, &registers.C);},
        [this]() {return BIT(4, &registers.D);},
        [this]() {return BIT(4, &registers.E);},
        [this]() {return BIT(4, &registers.H);},
        [this]() {return BIT(4, &registers.L);},
        [this]() {return BIT(4, nullptr);},
        [this]() {return BIT(4, &registers.A);},
        [this]() {return BIT(5, &registers.B);},
        [this]() {return BIT(5, &registers.C);},
        [this]() {return BIT(5, &registers.D);},
        [this]() {return BIT(5, &registers.E);},
        [this]() {return BIT(5, &registers.H);},
        [this]() {return BIT(5, &registers.L);},
        [this]() {return BIT(5, nullptr);},
        [this]() {return BIT(5, &registers.A);},
        [this]() {return BIT(6, &registers.B);},
        [this]() {return BIT(6, &registers.C);},
        [this]() {return BIT(6, &registers.D);},
        [this]() {return BIT(6, &registers.E);},
        [this]() {return BIT(6, &registers.H);},
        [this]() {return BIT(6, &registers.L);},
        [this]() {return BIT(6, nullptr);},
        [this]() {return BIT(6, &registers.A);},
        [this]() {return BIT(7, &registers.B);},
        [this]() {return BIT(7, &registers.C);},
        [this]() {return BIT(7, &registers.D);},
        [this]() {return BIT(7, &registers.E);},
        [this]() {return BIT(7, &registers.H);},
        [this]() {return BIT(7, &registers.L);},
        [this]() {return BIT(7, nullptr);},
        [this]() {return BIT(7, &registers.A);},
        [this]() {return RES(0, &registers.B);},
        [this]() {return RES(0, &registers.C);},
        [this]() {return RES(0, &registers.D);},
        [this]() {return RES(0, &registers.E);},
        [this]() {return RES(0, &registers.H);},
        [this]() {return RES(0, &registers.L);},
        [this]() {return RES(0, nullptr);},
        [this]() {return RES(0, &registers.A);},
        [this]() {return RES(1, &registers.B);},
        [this]() {return RES(1, &registers.C);},
        [this]() {return RES(1, &registers.D);},
        [this]() {return RES(1, &registers.E);},
        [this]() {return RES(1, &registers.H);},
        [this]() {return RES(1, &registers.L);},
        [this]() {return RES(1, nullptr);},
        [this]() {return RES(1, &registers.A);},
        [this]() {return RES(2, &registers.B);},
        [this]() {return RES(2, &registers.C);},
        [this]() {return RES(2, &registers.D);},
        [this]() {return RES(2, &registers.E);},
        [this]() {return RES(2, &registers.H);},
        [this]() {return RES(2, &registers.L);},
        [this]() {return RES(2, nullptr);},
        [this]() {return RES(2, &registers.A);},
        [this]() {return RES(3, &registers.B);},
        [this]() {return RES(3, &registers.C);},
        [this]() {return RES(3, &registers.D);},
        [this]() {return RES(3, &registers.E);},
        [this]() {return RES(3, &registers.H);},
        [this]() {return RES(3, &registers.L);},
        [this]() {return RES(3, nullptr);},
        [this]() {return RES(3, &registers.A);},
        [this]() {return RES(4, &registers.B);},
        [this]() {return RES(4, &registers.C);},
        [this]() {return RES(4, &registers.D);},
        [this]() {return RES(4, &registers.E);},
        [this]() {return RES(4, &registers.H);},
        [this]() {return RES(4, &registers.L);},
        [this]() {return RES(4, nullptr);},
        [this]() {return RES(4, &registers.A);},
        [this]() {return RES(5, &registers.B);},
        [this]() {return RES(5, &registers.C);},
        [this]() {return RES(5, &registers.D);},
        [this]() {return RES(5, &registers.E);},
        [this]() {return RES(5, &registers.H);},
        [this]() {return RES(5, &registers.L);},
        [this]() {return RES(5, nullptr);},
        [this]() {return RES(5, &registers.A);},
        [this]() {return RES(6, &registers.B);},
        [this]() {return RES(6, &registers.C);},
        [this]() {return RES(6, &registers.D);},
        [this]() {return RES(6, &registers.E);},
        [this]() {return RES(6, &registers.H);},
        [this]() {return RES(6, &registers.L);},
        [this]() {return RES(6, nullptr);},
        [this]() {return RES(6, &registers.A);},
        [this]() {return RES(7, &registers.B);},
        [this]() {return RES(7, &registers.C);},
        [this]() {return RES(7, &registers.D);},
        [this]() {return RES(7, &registers.E);},
        [this]() {return RES(7, &registers.H);},
        [this]() {return RES(7, &registers.L);},
        [this]() {return RES(7, nullptr);},
        [this]() {return RES(7, &registers.A);},
        [this]() {return SET(0, &registers.B);},
        [this]() {return SET(0, &registers.C);},
        [this]() {return SET(0, &registers.D);},
        [this]() {return SET(0, &registers.E);},
        [this]() {return SET(0, &registers.H);},
        [this]() {return SET(0, &registers.L);},
        [this]() {return SET(0, nullptr);},
        [this]() {return SET(0, &registers.A);},
        [this]() {return SET(1, &registers.B);},
        [this]() {return SET(1, &registers.C);},
        [this]() {return SET(1, &registers.D);},
        [this]() {return SET(1, &registers.E);},
        [this]() {return SET(1, &registers.H);},
        [this]() {return SET(1, &registers.L);},
        [this]() {return SET(1, nullptr);},
        [this]() {return SET(1, &registers.A);},
        [this]() {return SET(2, &registers.B);},
        [this]() {return SET(2, &registers.C);},
        [this]() {return SET(2, &registers.D);},
        [this]() {return SET(2, &registers.E);},
        [this]() {return SET(2, &registers.H);},
        [this]() {return SET(2, &registers.L);},
        [this]() {return SET(2, nullptr);},
        [this]() {return SET(2, &registers.A);},
        [this]() {return SET(3, &registers.B);},
        [this]() {return SET(3, &registers.C);},
        [this]() {return SET(3, &registers.D);},
        [this]() {return SET(3, &registers.E);},
        [this]() {return SET(3, &registers.H);},
        [this]() {return SET(3, &registers.L);},
        [this]() {return SET(3, nullptr);},
        [this]() {return SET(3, &registers.A);},
        [this]() {return SET(4, &registers.B);},
        [this]() {return SET(4, &registers.C);},
        [this]() {return SET(4, &registers.D);},
        [this]() {return SET(4, &registers.E);},
        [this]() {return SET(4, &registers.H);},
        [this]() {return SET(4, &registers.L);},
        [this]() {return SET(4, nullptr);},
        [this]() {return SET(4, &registers.A);},
        [this]() {return SET(5, &registers.B);},
        [this]() {return SET(5, &registers.C);},
        [this]() {return SET(5, &registers.D);},
        [this]() {return SET(5, &registers.E);},
        [this]() {return SET(5, &registers.H);},
        [this]() {return SET(5, &registers.L);},
        [this]() {return SET(5, nullptr);},
        [this]() {return SET(5, &registers.A);},
        [this]() {return SET(6, &registers.B);},
        [this]() {return SET(6, &registers.C);},
        [this]() {return SET(6, &registers.D);},
        [this]() {return SET(6, &registers.E);},
        [this]() {return SET(6, &registers.H);},
        [this]() {return SET(6, &registers.L);},
        [this]() {return SET(6, nullptr);},
        [this]() {return SET(6, &registers.A);},
        [this]() {return SET(7, &registers.B);},
        [this]() {return SET(7, &registers.C);},
        [this]() {return SET(7, &registers.D);},
        [this]() {return SET(7, &registers.E);},
        [this]() {return SET(7, &registers.H);},
        [this]() {return SET(7, &registers.L);},
        [this]() {return SET(7, nullptr);},
        [this]() {return SET(7, &registers.A);},

    };

public:
    inline static const char *opcodeStr[256] = {
            "NOP",
            "LD BC, n16",
            "LD [BC], A",
            "INC BC",
            "INC B",
            "DEC B",
            "LD B, n8",
            "RLCA",
            "LD [a16], SP",
            "ADD HL, BC",
            "LD A, [BC]",
            "DEC BC",
            "INC C",
            "DEC C",
            "LD C, n8",
            "RRCA",
            "STOP n8",
            "LD DE, n16",
            "LD [DE], A",
            "INC DE",
            "INC D",
            "DEC D",
            "LD D, n8",
            "RLA",
            "JR e8",
            "ADD HL, DE",
            "LD A, [DE]",
            "DEC DE",
            "INC E",
            "DEC E",
            "LD E, n8",
            "RRA",
            "JR NZ, e8",
            "LD HL, n16",
            "LD [HL+], A",
            "INC HL",
            "INC H",
            "DEC H",
            "LD H, n8",
            "DAA",
            "JR Z, e8",
            "ADD HL, HL",
            "LD A, [HL+]",
            "DEC HL",
            "INC L",
            "DEC L",
            "LD L, n8",
            "CPL",
            "JR NC, e8",
            "LD SP, n16",
            "LD [HL-], A",
            "INC SP",
            "INC [HL]",
            "DEC [HL]",
            "LD [HL], n8",
            "SCF",
            "JR C, e8",
            "ADD HL, SP",
            "LD A, [HL-]",
            "DEC SP",
            "INC A",
            "DEC A",
            "LD A, n8",
            "CCF",
            "LD B, B",
            "LD B, C",
            "LD B, D",
            "LD B, E",
            "LD B, H",
            "LD B, L",
            "LD B, [HL]",
            "LD B, A",
            "LD C, B",
            "LD C, C",
            "LD C, D",
            "LD C, E",
            "LD C, H",
            "LD C, L",
            "LD C, [HL]",
            "LD C, A",
            "LD D, B",
            "LD D, C",
            "LD D, D",
            "LD D, E",
            "LD D, H",
            "LD D, L",
            "LD D, [HL]",
            "LD D, A",
            "LD E, B",
            "LD E, C",
            "LD E, D",
            "LD E, E",
            "LD E, H",
            "LD E, L",
            "LD E, [HL]",
            "LD E, A",
            "LD H, B",
            "LD H, C",
            "LD H, D",
            "LD H, E",
            "LD H, H",
            "LD H, L",
            "LD H, [HL]",
            "LD H, A",
            "LD L, C",
            "LD L, D",
            "LD L, B",
            "LD L, E",
            "LD L, H",
            "LD L, L",
            "LD L, [HL]",
            "LD L, A",
            "LD [HL], B",
            "LD [HL], C",
            "LD [HL], D",
            "LD [HL], E",
            "LD [HL], H",
            "LD [HL], L",
            "HALT",
            "LD [HL], A",
            "LD A, B",
            "LD A, C",
            "LD A, D",
            "LD A, E",
            "LD A, H",
            "LD A, L",
            "LD A, [HL]",
            "LD A, A",
            "ADD A, B",
            "ADD A, C",
            "ADD A, D",
            "ADD A, E",
            "ADD A, H",
            "ADD A, L",
            "ADD A, [HL]",
            "ADD A, A",
            "ADC A, B",
            "ADC A, C",
            "ADC A, D",
            "ADC A, E",
            "ADC A, H",
            "ADC A, L",
            "ADC A, [HL]",
            "ADC A, A",
            "SUB A, B",
            "SUB A, C",
            "SUB A, D",
            "SUB A, E",
            "SUB A, H",
            "SUB A, L",
            "SUB A, [HL]",
            "SUB A, A",
            "SBC A, B",
            "SBC A, C",
            "SBC A, D",
            "SBC A, E",
            "SBC A, H",
            "SBC A, L",
            "SBC A, [HL]",
            "SBC A, A",
            "AND A, B",
            "AND A, C",
            "AND A, D",
            "AND A, E",
            "AND A, H",
            "AND A, L",
            "AND A, [HL]",
            "AND A, A",
            "XOR A, B",
            "XOR A, C",
            "XOR A, D",
            "XOR A, E",
            "XOR A, H",
            "XOR A, L",
            "XOR A, [HL]",
            "XOR A, A",
            "OR A, B",
            "OR A, C",
            "OR A, D",
            "OR A, E",
            "OR A, H",
            "OR A, L",
            "OR A, [HL]",
            "OR A, A",
            "CP A, B",
            "CP A, C",
            "CP A, D",
            "CP A, E",
            "CP A, H",
            "CP A, L",
            "CP A, [HL]",
            "CP A, A",
            "RET NZ",
            "POP BC",
            "JP NZ, a16",
            "JP a16",
            "CALL NZ, a16",
            "PUSH BC",
            "ADD A, n8",
            "RST 0x0000",
            "RET Z",
            "RET",
            "JP Z, a16",
            "PREFIX",
            "CALL Z, a16",
            "CALL a16",
            "ADC A, n8",
            "RST 0x0008",
            "RET NC",
            "POP DE",
            "JP NC, a16",
            "NIMP",
            "CALL NC, a16",
            "PUSH DE",
            "SUB A, n8",
            "RST 0x0010",
            "RET C",
            "RETI",
            "JP C, a16",
            "NIMP",
            "CALL C, a16",
            "NIMP",
            "SBC A, n8",
            "RST 0x0018",
            "LDH [a8], A",
            "POP HL",
            "LD [C], A",
            "NIMP",
            "NIMP",
            "PUSH HL",
            "AND A, n8",
            "RST 0x0020",
            "ADD SP, e8",
            "JP HL",
            "LD [a16], A",
            "NIMP",
            "NIMP",
            "NIMP",
            "XOR A, n8",
            "RST 0x0028",
            "LDH A, [a8]",
            "POP AF",
            "LD A, [C]",
            "DI",
            "NIMP",
            "PUSH AF",
            "OR A, n8",
            "RST 0x0030",
            "LD HL, SP + e8",
            "LD SP, HL",
            "LD A, [a16]",
            "EI",
            "NIMP",
            "NIMP",
            "CP A, n8",
            "RST 0x0038",
    };

private:
    // Normal instructions
    uint8_t NIMP(uint8_t opcode);
    uint8_t NOP();
    uint8_t LD(uint8_t *reg);
    uint8_t LD(uint8_t *reg1, const uint8_t *reg2);
    uint8_t LD(uint8_t *reg1, uint8_t reg2);
    uint8_t LD(uint8_t reg1, const uint8_t *reg2);
    uint8_t LD(uint16_t *reg1, const uint16_t *reg2, bool addDataToSP = false);
    uint8_t LD(uint16_t reg1, const uint8_t *reg2, bool incrementReg1 = false, bool decrementReg1 = false);
    uint8_t LD(uint8_t *reg1, uint16_t reg2, bool incrementReg2 = false, bool decrementReg2 = false);
    uint8_t INC(uint8_t *reg);
    uint8_t INC(uint16_t *reg);
    uint8_t INC(uint16_t reg);
    uint8_t DEC(uint8_t *reg);
    uint8_t DEC(uint16_t *reg);
    uint8_t DEC(uint16_t reg);
    uint8_t RLCA();
    uint8_t ADD(const uint8_t *reg);
    uint8_t ADD(uint16_t reg);
    uint8_t ADD(uint16_t *reg1, const uint16_t *reg2);
    uint8_t ADC(const uint8_t *reg);
    uint8_t ADC(uint16_t reg);
    uint8_t SUB(const uint8_t *reg);
    uint8_t SUB(uint16_t reg);
    uint8_t SBC(const uint8_t *reg);
    uint8_t SBC(uint16_t reg);
    uint8_t AND(const uint8_t *reg);
    uint8_t AND();
    uint8_t XOR(const uint8_t *reg);
    uint8_t XOR();
    uint8_t OR(const uint8_t *reg);
    uint8_t OR();
    uint8_t CP(const uint8_t *reg);
    uint8_t CP(uint16_t reg);
    uint8_t RRCA();
    uint8_t STOP();
    uint8_t RLA();
    uint8_t JR(const bool *flag);
    uint8_t RRA();
    uint8_t DAA();
    uint8_t CPL();
    uint8_t SCF();
    uint8_t CCF();
    uint8_t HALT();
    uint8_t RET(const bool *flag);
    uint8_t POP(uint16_t *reg);
    uint8_t JP(const bool *flag, bool unused = false);
    uint8_t JP(const uint16_t *reg);
    uint8_t CALL(const bool *flag);
    uint8_t PUSH(const uint16_t *reg);
    uint8_t RST(uint16_t addr);
    uint8_t PREFIX();
    uint8_t RETI();
    uint8_t LDH(const uint8_t *reg);
    uint8_t DI();
    uint8_t EI();

    // Prefixed instruction (using 0xCB PREFIX instruction)
    uint8_t RLC(uint8_t *reg);
    uint8_t RRC(uint8_t *reg);
    uint8_t RL(uint8_t *reg);
    uint8_t RR(uint8_t *reg);
    uint8_t SLA(uint8_t *reg);
    uint8_t SRA(uint8_t *reg);
    uint8_t SWAP(uint8_t *reg);
    uint8_t SRL(uint8_t *reg);
    uint8_t BIT(uint8_t bit, const uint8_t *reg);
    uint8_t RES(uint8_t bit, uint8_t *reg);
    uint8_t SET(uint8_t bit, uint8_t *reg);
};

#endif //EMU_GAMEBOY_SHARPSM83_H
