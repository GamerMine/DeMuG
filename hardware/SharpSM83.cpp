/*
 *  ____
 * /\  _`\                                       /'\_/`\  __
 * \ \ \L\_\     __      ___ ___      __   _ __ /\      \/\_\    ___      __
 *  \ \ \L_L   /'__`\  /' __` __`\  /'__`\/\`'__\ \ \__\ \/\ \ /' _ `\  /'__`\
 *   \ \ \/, \/\ \L\.\_/\ \/\ \/\ \/\  __/\ \ \/ \ \ \_/\ \ \ \/\ \/\ \/\  __/
 *    \ \____/\ \__/.\_\ \_\ \_\ \_\ \____\\ \_\  \ \_\\ \_\ \_\ \_\ \_\ \____\
 *     \/___/  \/__/\/_/\/_/\/_/\/_/\/____/ \/_/   \/_/ \/_/\/_/\/_/\/_/\/____/
 *
 * Copyright (c) 2023-2023 GamerMine <maxime-sav@outlook.fr>
 *
 * This Source Code From is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/ .
 */

#include "SharpSM83.h"

Logger *SharpSM83::logger;
bool SharpSM83::PAUSE = false;
bool SharpSM83::NEXT_INSTR = false;
SharpSM83::debugInfo SharpSM83::DEBUG_INFO = {};

SharpSM83::SharpSM83(class Bus *bus) {
    this->mBus = bus;

    PC = 0x0100;
    SP = 0xFFFE;
    registers.A = 0x01;
    registers.B = 0xFF;
    registers.C = 0x13;
    registers.D = 0x00;
    registers.E = 0xC1;
    registers.H = 0x84;
    registers.L = 0x03;
    flags.rawFlags = 0x00;
    interruptShouldBeEnabled = false;
    IF.raw = 0xE1;
    IME = false;

    logger = Logger::getInstance("Cpu");
}

void SharpSM83::reset() {
    PC = 0x0100;
    SP = 0xFFFE;
    registers.A = 0x01;
    registers.B = 0xFF;
    registers.C = 0x13;
    registers.D = 0x00;
    registers.E = 0xC1;
    registers.H = 0x84;
    registers.L = 0x03;
    flags.rawFlags = 0x00;
    interruptShouldBeEnabled = false;
    IF.raw = 0xE1;
    IME = false;
}

void SharpSM83::operator()() {
    PAUSE = true;
    while (!Bus::GLOBAL_HALT) {
        if (!PAUSE || NEXT_INSTR) {
            if (interruptShouldBeEnabled) { IME = true; } else {IME = false;}

            uint8_t instr = mBus->read(PC++);
            {
                DEBUG_INFO.currentInstr = opcodeStr[instr];
                DEBUG_INFO.currentAddr = PC - 1;
                DEBUG_INFO.Z = flags.zero;
                DEBUG_INFO.C = flags.carry;
                DEBUG_INFO.HC = flags.halfCarry;
                DEBUG_INFO.N = flags.negative;
                DEBUG_INFO.regA = registers.A;
                DEBUG_INFO.regB = registers.B;
                DEBUG_INFO.regC = registers.C;
                DEBUG_INFO.regD = registers.D;
                DEBUG_INFO.regE = registers.E;
                DEBUG_INFO.regH = registers.H;
                DEBUG_INFO.regL = registers.L;
                DEBUG_INFO.regSP = SP;
            }
            opcodes[instr]();


            {
                if (IME) {
                    if (IE.vblank && IF.vblank) {
                        IME = 0;
                        interruptShouldBeEnabled = false;
                        IF.vblank = 0;
                        SP--;
                        mBus->write(SP--, PC >> 8);
                        mBus->write(SP, PC & 0xFF);
                        PC = 0x0040;
                    } else if (IE.lcd && IF.lcd) {
                        IME = 0;
                        interruptShouldBeEnabled = false;
                        IF.lcd = 0;
                        SP--;
                        mBus->write(SP--, PC >> 8);
                        mBus->write(SP, PC & 0xFF);
                        PC = 0x0048;
                    } else if (IE.joypad && IF.joypad) {
                        IME = 0;
                        interruptShouldBeEnabled = false;
                        IF.joypad = 0;
                        SP--;
                        mBus->write(SP--, PC >> 8);
                        mBus->write(SP, PC & 0xFF);
                        PC = 0x0060;
                    }
                }
            }
            if (NEXT_INSTR) { using namespace std::chrono_literals; std::this_thread::sleep_for(100ms); }
        }
    }
}

uint8_t SharpSM83::NOP() {
    return 1;
}

uint8_t SharpSM83::LD(uint8_t *reg) {
    if (reg == nullptr) {
        mBus->write(mBus->read(PC + 1) << 8 | mBus->read(PC), registers.A);
        PC += 2;
    } else {
        *reg = mBus->read(mBus->read(PC + 1) << 8 | mBus->read(PC));
        PC += 2;
    }
    return 4;
}

uint8_t SharpSM83::LD(uint16_t *reg1, const uint16_t *reg2, bool addDataToSP) { // TODO: Support for addDataToSP
    uint8_t cycles;
    if (reg1 == nullptr) {
        uint16_t addr = mBus->read(PC + 1) << 8 | mBus->read(PC);
        mBus->write(addr, SP & 0xFF);
        mBus->write(addr + 1, SP >> 8);
        PC += 2;
        cycles = 5;
    }
    else if (reg2 == nullptr) {
        *reg1 = mBus->read(PC + 1) << 8 | mBus->read(PC);
        PC = PC + 2;
        cycles = 3;
    }
    else {
        if (addDataToSP) {
            auto value = (int8_t)mBus->read(PC++);
            int result = static_cast<int>(SP + value);

            logger->log(Logger::DEBUG, "%sBEFORE: HL = %X\tSP = %X\te8 = %i", Colors::LOG_ORANGE, registers.HL, SP, value);

            flags.zero = 0;
            flags.negative = 0;
            flags.halfCarry = ((SP ^ value ^ (result & 0xFFFF)) & 0x10) == 0x10;
            flags.carry = ((SP ^ value ^ (result & 0xFFFF)) & 0x100) == 0x100;

            registers.HL = static_cast<uint16_t>(result);

            logger->log(Logger::DEBUG, "%sAFTER: HL = %X\t carry = %b\t halfcarry = %b", Colors::LOG_DARK_GREEN, registers.HL, flags.carry, flags.halfCarry);
        } else {
            *reg1 = *reg2;
            cycles = 2;
        }
    }
    return cycles;
}

uint8_t SharpSM83::LD(uint16_t reg1, const uint8_t *reg2, bool incrementReg1, bool decrementReg1) {
    uint8_t cycles;
    if (reg2 == nullptr) {
        mBus->write(reg1, mBus->read(PC++));
        cycles = 3;
    }
    else {
        mBus->write(reg1, *reg2);
        if (incrementReg1) registers.HL++;
        if (decrementReg1) registers.HL--;
        cycles = 2;
    }

    return cycles;
}

uint8_t SharpSM83::LD(uint8_t *reg1, const uint8_t *reg2) {
    uint8_t cycles;
    if (reg2 == nullptr) {
        *reg1 = mBus->read(PC++);
        cycles = 2;
    } else {
        *reg1 = *reg2;
        cycles = 1;
    }
    return cycles;
}

uint8_t SharpSM83::LD(uint8_t reg1, const uint8_t *reg2) {
    mBus->write(0xFF00 + reg1, *reg2);
    return 2;
}

uint8_t SharpSM83::LD(uint8_t *reg1, uint16_t reg2, bool incrementReg2, bool decrementReg2) {
    *reg1 = mBus->read(reg2);
    if (incrementReg2) registers.HL++;
    if (decrementReg2) registers.HL--;
    return 2;
}

uint8_t SharpSM83::LDH(const uint8_t *reg) {
    if (reg == nullptr) {
        mBus->write(0xFF00 + mBus->read(PC++), registers.A);
    } else {
        registers.A = mBus->read(0xFF00 + mBus->read(PC++));
    }
    return 3;
}

uint8_t SharpSM83::XOR(const uint8_t *reg) {
    uint8_t cycles;
    if (reg == nullptr) {
        registers.A ^= mBus->read(PC++);
        cycles = 2;
    }
    else {
        registers.A ^= *reg;
        cycles = 1;
    }

    flags.zero = registers.A == 0x00;
    flags.negative = 0;
    flags.halfCarry = 0;
    flags.carry = 0;

    return cycles;
}

uint8_t SharpSM83::PREFIX() {
    return pfxOpcodes[mBus->read(PC++)]();
}

uint8_t SharpSM83::JR(const bool *flag) {
    if (flag == nullptr) {
        auto relAddr = static_cast<int8_t>(mBus->read(PC++));
        PC = PC + relAddr;
    } else {
        auto relAddr = static_cast<int8_t>(mBus->read(PC++));
        if (*flag) {
            PC = PC + relAddr;
        }
    }

    return 1;
}

uint8_t SharpSM83::INC(uint8_t *reg) {
    *reg = *reg + 1;

    flags.halfCarry = (*reg & 0x0F) == 0x00;
    flags.zero = *reg == 0x00;
    flags.negative = 0;

    return 1;
}

uint8_t SharpSM83::INC(uint16_t *reg) {
    *reg = *reg + 1;
    return 1;
}

uint8_t SharpSM83::DEC(uint8_t *reg) {
    *reg = *reg - 1;

    flags.halfCarry = (*reg & 0x0F) == 0x0F;
    flags.zero = *reg == 0x00;
    flags.negative = 1;

    return 1;
}

uint8_t SharpSM83::SUB(const uint8_t *reg) {
    uint8_t cycles;
    uint8_t value;
    uint16_t result;
    if (reg == nullptr) {
        value = mBus->read(PC++);
        cycles = 2;
    } else {
        value = *reg;
        cycles = 1;
    }

    result = registers.A - value;

    flags.zero = (result & 0xFF) == 0x00;
    flags.negative = 1;
    flags.halfCarry = ((registers.A & 0x0F) - (value & 0x0F)) < 0x00;
    flags.carry = registers.A < value;

    registers.A = result & 0xFF;

    return cycles;
}

uint8_t SharpSM83::CALL(const bool *flag) {
    if (flag == nullptr) {
        uint16_t addr = mBus->read(PC + 1) << 8 | mBus->read(PC);
        PC += 2;
        SP--;
        mBus->write(SP--, PC >> 8);
        mBus->write(SP, PC & 0xFF);
        PC = addr;
    } else {
        uint16_t addr = mBus->read(PC + 1) << 8 | mBus->read(PC);
        PC += 2;
        if (*flag) {
            SP--;
            mBus->write(SP--, PC >> 8);
            mBus->write(SP, PC & 0xFF);
            PC = addr;
        }
    }

    return 1;
}

uint8_t SharpSM83::CP(const uint8_t *reg) {
    uint8_t cycles;
    uint8_t value;
    int result;
    if (reg == nullptr) {
        value = mBus->read(PC++);
        cycles = 2;
    } else {
        value = *reg;
        cycles = 1;
    }
    result = registers.A - value;

    flags.zero = result == 0;
    flags.negative = 1;
    flags.carry = registers.A < value;
    flags.halfCarry = ((registers.A & 0x0F) - (value & 0x0F)) < 0;

    return cycles;
}

uint8_t SharpSM83::CP(uint16_t reg) {
    uint8_t value = mBus->read(reg);
    int result = registers.A - value;

    flags.zero = result == 0;
    flags.negative = 1;
    flags.carry = registers.A < value;
    flags.halfCarry = ((registers.A & 0x0F) - (value & 0x0F)) < 0x00;

    return 2;
}

uint8_t SharpSM83::PUSH(const uint16_t *reg) {
    if (reg == nullptr) {
        SP--;
        mBus->write(SP--, registers.A);
        mBus->write(SP, flags.rawFlags);
    } else {
        SP--;
        mBus->write(SP--, *reg >> 8);
        mBus->write(SP, *reg & 0xFF);
    }
    return 4;
}

uint8_t SharpSM83::RLA() {
    uint8_t tmpValue = registers.A;
    registers.A = (registers.A << 1) | flags.carry;

    flags.carry = tmpValue >> 7;
    flags.zero = 0;
    flags.negative = 0;
    flags.halfCarry = 0;

    return 1;
}

uint8_t SharpSM83::POP(uint16_t *reg) {
    if (reg == nullptr) {
        uint8_t data = mBus->read(SP++);
        flags.rawFlags = data & 0xF0;
        registers.A = mBus->read(SP++);
    } else {
        *reg = mBus->read(SP + 1) << 8 | mBus->read(SP);
        SP += 2;
    }
    return 3;
}

uint8_t SharpSM83::RET(const bool *flag) {
    if (flag == nullptr) {
        PC = mBus->read(SP + 1) << 8 | mBus->read(SP);
        SP += 2;
    } else {
        if (*flag) {
            PC = mBus->read(SP + 1) << 8 | mBus->read(SP);
            SP += 2;
        }
    }
    return 1;
}

uint8_t SharpSM83::BIT(uint8_t bit, const uint8_t *reg) {
    uint8_t cycles;
    if (reg == nullptr) {
        flags.zero = (mBus->read(registers.HL) & (1 << bit)) == 0;
        cycles = 3;
    } else {
        flags.zero = (*reg & (1 << bit)) == 0;
        cycles = 2;
    }

    flags.negative = 0;
    flags.halfCarry = 1;

    return cycles;
}

uint8_t SharpSM83::RL(uint8_t *reg) {
    if (reg == nullptr) {
        uint8_t value = mBus->read(registers.HL);
        mBus->write(registers.HL, (value << 1) | flags.carry);
        flags.zero = ((value << 1) | flags.carry) == 0x00;
        flags.carry = value >> 7;
    } else {
        uint8_t value = *reg;
        *reg = (*reg << 1) | flags.carry;
        flags.zero = ((value << 1) | flags.carry) == 0x00;
        flags.carry = value >> 7;
    }

    flags.negative = 0;
    flags.halfCarry = 0;

    return 0;
}

uint8_t SharpSM83::NIMP() {
    logger->log(Logger::WARNING, "Not implemented following %s at %X", opcodeStr[mBus->read(PC - 2)], PC - 2); return 0;
}

uint8_t SharpSM83::LD(uint8_t *reg1, uint8_t reg2) {
    *reg1 = mBus->read(0xFF00 + reg2);

    return 2;
}

uint8_t SharpSM83::INC(uint16_t reg) {
    uint8_t value = mBus->read(reg);
    value = value + 1;
    mBus->write(reg, value);

    flags.halfCarry = (value & 0x0F) == 0x00;
    flags.zero = value == 0x00;
    flags.negative = 0;

    return 3;
}

uint8_t SharpSM83::DEC(uint16_t *reg) {
    *reg = *reg - 1;

    return 2;
}

uint8_t SharpSM83::DEC(uint16_t reg) {
    uint8_t value = mBus->read(reg);
    value = value - 1;
    mBus->write(reg, value);

    flags.halfCarry = (value & 0xF) == 0x0F;
    flags.zero = value == 0x00;
    flags.negative = 1;

    return 3;
}

uint8_t SharpSM83::RLCA() {
    flags.carry = registers.A >> 7;
    registers.A = (registers.A << 1) + flags.carry;

    flags.zero = 0;
    flags.negative = 0;
    flags.halfCarry = 0;

    return 1;
}

uint8_t SharpSM83::ADD(const uint8_t *reg) {
    uint result;
    uint8_t cycles;
    if (reg == nullptr) {
        uint8_t value = mBus->read(PC++);
        result = registers.A + value;
        flags.halfCarry = ((registers.A & 0xF) + (value & 0xF)) > 0x0F;
        cycles = 2;
    } else {
        uint8_t value = *reg;
        result = registers.A + value;
        flags.halfCarry = ((registers.A & 0xF) + (value & 0xF)) > 0x0F;
        cycles = 1;
    }
    registers.A = result & 0xFF;

    flags.zero = (result & 0xFF) == 0x00;
    flags.negative = 0;
    flags.carry = (result & 0x100) != 0;

    return cycles;
}

uint8_t SharpSM83::ADD(uint16_t reg) {
    uint8_t value = mBus->read(reg);
    uint result = registers.A + value;
    flags.halfCarry = ((registers.A & 0xF) + (value & 0xF)) > 0x0F;
    registers.A = static_cast<uint8_t>(result);

    flags.zero = (result & 0xFF) == 0x00;
    flags.negative = 0;
    flags.carry = (result & 0x100) != 0;

    return 0;
}

uint8_t SharpSM83::ADD(uint16_t *reg1, const uint16_t *reg2) {
    uint8_t cycles;
    if (reg2 == nullptr) {
        auto value = (int8_t)mBus->read(PC++);
        int result = static_cast<int>(SP + value);

        flags.zero = 0;
        flags.negative = 0;
        flags.halfCarry = ((SP ^ value ^ (result & 0xFFFF)) & 0x10) == 0x10;
        flags.carry = ((SP ^ value ^ (result & 0xFFFF)) & 0x100) == 0x100;

        SP = static_cast<uint16_t>(result);

        cycles = 4;
    } else {
        uint result = *reg1 + *reg2;

        flags.halfCarry = ((*reg1 & 0xFFF) + (*reg2 & 0xFFF)) > 0x0FFF;
        flags.carry = (result & 0x10000) != 0;
        flags.negative = 0;

        *reg1 = static_cast<uint16_t>(result);
        cycles = 2;
    }

    return cycles;
}

uint8_t SharpSM83::ADC(const uint8_t *reg) {
    uint8_t cycles;
    uint16_t result;
    uint8_t value;
    if (reg == nullptr) {
        value = mBus->read(PC++);
        result = registers.A + value + flags.carry;
        cycles = 2;
    } else {
        value = *reg;
        result = registers.A + value + flags.carry;
        cycles = 1;
    }
    flags.halfCarry = ((registers.A & 0xF) + (value & 0xF) + flags.carry) > 0x0F;
    registers.A = result & 0xFF;

    flags.zero = (result & 0xFF) == 0x00;
    flags.negative = 0;
    flags.carry = result > 0xFF;

    return cycles;
}

uint8_t SharpSM83::ADC(uint16_t reg) {
    uint8_t value = mBus->read(reg);
    uint16_t result = registers.A + value + flags.carry;
    flags.halfCarry = ((registers.A & 0xF) + (value & 0xF) + flags.carry) > 0x0F;
    registers.A = result & 0xFF;

    flags.zero = (result & 0xFF) == 0x00;
    flags.negative = 0;
    flags.carry = result > 0xFF;

    return 2;
}

uint8_t SharpSM83::SUB(uint16_t reg) {
    uint8_t value = mBus->read(reg);
    uint16_t result = registers.A - value;

    flags.zero = (result & 0xFF) == 0x00;
    flags.negative = 1;
    flags.halfCarry = ((registers.A & 0x0F) - (value & 0x0F)) < 0x00;
    flags.carry = registers.A < value;

    registers.A = result & 0xFF;

    return 2;
}

uint8_t SharpSM83::SBC(const uint8_t *reg) {
    uint8_t cycles;
    uint8_t value;
    uint16_t result;
    if (reg == nullptr) {
        value = mBus->read(PC++);
        cycles = 2;
    } else {
        value = *reg;
        cycles = 1;
    }

    result = registers.A - value - flags.carry;
    int result_full = registers.A - value - flags.carry;

    flags.zero = (result & 0xFF) == 0x00;
    flags.negative = 1;
    flags.halfCarry = ((registers.A & 0x0F) - (value & 0x0F) - flags.carry) < 0x00;
    flags.carry = result_full < 0x00;

    registers.A = result & 0xFF;

    return cycles;
}

uint8_t SharpSM83::SBC(uint16_t reg) {
    uint8_t value = mBus->read(reg);
    uint16_t result = registers.A - value - flags.carry;
    int result_full = registers.A - value - flags.carry;

    flags.zero = (result & 0xFF) == 0x00;
    flags.negative = 1;
    flags.halfCarry = ((registers.A & 0x0F) - (value & 0x0F) - flags.carry) < 0x00;
    flags.carry = result_full < 0x00;

    registers.A = result & 0xFF;

    return 2;
}

uint8_t SharpSM83::AND(const uint8_t *reg) {
    uint8_t cycles;
    if (reg == nullptr) {
        registers.A &= mBus->read(PC++);
        cycles = 2;
    } else {
        registers.A &= *reg;
        cycles = 1;
    }

    flags.zero = registers.A == 0x00;
    flags.negative = 0;
    flags.halfCarry = 1;
    flags.carry = 0;

    return cycles;
}

uint8_t SharpSM83::AND(uint16_t reg) {
    registers.A &= mBus->read(reg);

    flags.zero = registers.A == 0x00;
    flags.negative = 0;
    flags.halfCarry = 1;
    flags.carry = 0;

    return 2;
}

uint8_t SharpSM83::XOR(uint16_t reg) {
    registers.A ^= mBus->read(reg);

    flags.zero = registers.A == 0x00;
    flags.negative = 0;
    flags.halfCarry = 0;
    flags.carry = 0;

    return 2;
}

uint8_t SharpSM83::OR(const uint8_t *reg) {
    uint8_t cycles;
    if (reg == nullptr) {
        registers.A |= mBus->read(PC++);
        cycles = 2;
    } else {
        registers.A |= *reg;
        cycles = 1;
    }

    flags.zero = registers.A == 0x00;
    flags.negative = 0;
    flags.halfCarry = 0;
    flags.carry = 0;

    return cycles;
}

uint8_t SharpSM83::OR(uint16_t reg) {
    registers.A |= mBus->read(reg);

    flags.zero = registers.A == 0x00;
    flags.negative = 0;
    flags.halfCarry = 0;
    flags.carry = 0;

    return 2;
}

uint8_t SharpSM83::RRCA() {
    flags.carry &= 0x01;
    registers.A = registers.A >> 1 | flags.carry << 7;

    flags.zero = 0;
    flags.negative = 0;
    flags.halfCarry = 0;

    return 0;
}

uint8_t SharpSM83::STOP() { // The STOP instruction seems to be resest after a key press
    logger->log(Logger::CRITICAL, "Not Implemented STOP");
    return 0;
}

uint8_t SharpSM83::RRA() {
    uint8_t value = registers.A;
    bool carry = flags.carry;
    bool willCarry = value & 0x01;
    flags.carry = willCarry;

    uint8_t result = value >> 1;
    result |= (carry << 7);
    flags.zero = result == 0x00;

    registers.A = result;

    flags.zero = 0;
    flags.negative = 0;
    flags.halfCarry = 0;

    return 1;
}

uint8_t SharpSM83::DAA() {
    uint8_t reg = registers.A;

    uint16_t correction = flags.carry
                     ? 0x60
                     : 0x00;

    if (flags.halfCarry || (!flags.negative && ((reg & 0x0F) > 9))) {
        correction |= 0x06;
    }

    if (flags.carry || (!flags.negative && (reg > 0x99))) {
        correction |= 0x60;
    }

    if (flags.negative) {
        reg = static_cast<uint8_t>(reg - correction);
    } else {
        reg = static_cast<uint8_t>(reg + correction);
    }

    if (((correction << 2) & 0x100) != 0) {
        flags.carry = 1;
    }

    flags.halfCarry = 0;
    flags.zero = (reg == 0);

    registers.A = static_cast<uint8_t>(reg);

    return 1;
}

uint8_t SharpSM83::CPL() {
    registers.A = ~registers.A;

    flags.negative = 1;
    flags.halfCarry = 1;

    return 0;
}

uint8_t SharpSM83::SCF() {
    logger->log(Logger::DEBUG, "Not implemented 19"); return 0;
}

uint8_t SharpSM83::CCF() {
    logger->log(Logger::DEBUG, "Not implemented 20"); return 0;
}

uint8_t SharpSM83::HALT() {
    logger->log(Logger::DEBUG, "Not implemented 21"); return 0; // TODO: Implements HALT
}

uint8_t SharpSM83::JP(const bool *flag, bool unused) {
    if (flag == nullptr) {
        PC = mBus->read(PC + 1) << 8 | mBus->read(PC);
    } else {
        if (*flag) {
            PC = mBus->read(PC + 1) << 8 | mBus->read(PC);
        } else {
            PC = PC + 2;
        }
    }

    return 1;
}

uint8_t SharpSM83::JP(const uint16_t *reg) {
    PC = *reg;

    return 1;
}

uint8_t SharpSM83::RST(uint16_t addr) {
    SP--;
    mBus->write(SP--, PC >> 8);
    mBus->write(SP, PC & 0xFF);

    PC = addr;

    return 4;
}

uint8_t SharpSM83::RETI() {
    PC = mBus->read(SP + 1) << 8 | mBus->read(SP);
    SP += 2;
    interruptShouldBeEnabled = true;
    IME = true;

    return 4;
}

uint8_t SharpSM83::DI() {
    interruptShouldBeEnabled = false;

    return 1;
}

uint8_t SharpSM83::EI() {
    interruptShouldBeEnabled = true;

    return 1;
}

uint8_t SharpSM83::RLC(uint8_t *reg) {
    uint8_t cycles;
    uint8_t value;
    if (reg == nullptr) {
        value = mBus->read(registers.HL);
        flags.carry = value >> 7;
        value = (value << 1) + flags.carry;
        mBus->write(registers.HL, value);
        cycles = 4;
    } else {
        value = *reg;
        flags.carry = value >> 7;
        value = (value << 1) + flags.carry;
        *reg = value;
        cycles = 2;
    }

    flags.zero = (value == 0x00);
    flags.negative = 0;
    flags.halfCarry = 0;

    return cycles;
}

uint8_t SharpSM83::RRC(uint8_t *reg) {
    uint8_t cycles;
    uint8_t value;
    if (reg == nullptr) {
        value = mBus->read(registers.HL);
        flags.carry = value & 0x01;
        value = (value >> 1) | (flags.carry << 7);
        mBus->write(registers.HL, value);
        cycles = 4;
    } else {
        flags.carry = *reg & 0x01;
        *reg = (*reg >> 1) | (flags.carry << 7);
        value = *reg;
        cycles = 2;
    }

    flags.zero = value == 0x00;
    flags.negative = 0;
    flags.halfCarry = 0;

    return cycles;
}

uint8_t SharpSM83::RR(uint8_t *reg) {
    if (reg == nullptr) {
        uint8_t value = mBus->read(registers.HL);
        bool carry = flags.carry;
        bool willCarry = value & 0x01;
        flags.carry = willCarry;

        uint8_t result = value >> 1;
        result |= (carry << 7);
        flags.zero = result == 0x00;

        mBus->write(registers.HL, result);

    } else {
        uint8_t value = *reg;
        bool carry = flags.carry;
        bool willCarry = value & 0x01;
        flags.carry = willCarry;

        uint8_t result = value >> 1;
        result |= (carry << 7);
        flags.zero = result == 0x00;

        *reg = result;
    }

    flags.negative = 0;
    flags.halfCarry = 0;

    return 0;
}

uint8_t SharpSM83::SLA(uint8_t *reg) {
    uint8_t cycles;
    uint8_t value;
    if (reg == nullptr) {
        value = mBus->read(registers.HL);
        flags.carry = value >> 7;
        value = value << 1;
        mBus->write(registers.HL, value);
        cycles = 4;
    } else {
        flags.carry = *reg >> 7;
        *reg = *reg << 1;
        value = *reg;
        cycles = 2;
    }

    flags.zero = value == 0x00;
    flags.negative = 0;
    flags.halfCarry = 0;

    return cycles;
}

uint8_t SharpSM83::SRA(uint8_t *reg) {
    logger->log(Logger::DEBUG, "Not implemented 32"); return 0;
}

uint8_t SharpSM83::SWAP(uint8_t *reg) {
    uint8_t cycles;
    uint8_t value;
    if (reg == nullptr) {
        value = mBus->read(registers.HL);
        value = value << 4 | value >> 4;
        mBus->write(registers.HL, value);
        cycles = 4;
    } else {
        *reg = *reg << 4 | *reg >> 4;
        value = *reg;
        cycles = 2;
    }

    flags.zero = value == 0x00;
    flags.negative = 0;
    flags.halfCarry = 0;
    flags.carry = 0;

    return cycles;
}

uint8_t SharpSM83::SRL(uint8_t *reg) {
    uint8_t cycles;
    uint8_t value;
    if (reg == nullptr) {
        value = mBus->read(registers.HL);

        flags.carry = value & 0x01;

        value >>= 1;

        mBus->write(registers.HL, value);
        cycles = 4;
    } else {
        flags.carry = *reg & 0x01;
        *reg = *reg >> 1;
        value = *reg;
        cycles = 2;
    }

    flags.zero = value == 0x00;
    flags.negative = 0;
    flags.halfCarry = 0;

    return cycles;
}

uint8_t SharpSM83::RES(uint8_t bit, uint8_t *reg) {
    uint8_t cycles;
    if (reg == nullptr) {
        uint8_t value = mBus->read(registers.HL);
        value = value & ~(1 << bit);
        mBus->write(registers.HL, value);
        cycles = 4;
    } else {
        *reg = *reg & ~(1 << bit);
        cycles = 2;
    }

    return cycles;
}

uint8_t SharpSM83::SET(uint8_t bit, uint8_t *reg) {
    uint8_t cycles;
    if (reg == nullptr) {
        uint8_t value = mBus->read(registers.HL);
        value |= (0x1 << bit);
        mBus->write(registers.HL, value);
        cycles = 4;
    } else {
        *reg = *reg | (0x1 << bit);
        cycles = 2;
    }

    return cycles;
}

SharpSM83::debugInfo::~debugInfo() = default;
