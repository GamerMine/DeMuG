#include "SharpSM83.h"

Logger *SharpSM83::logger;
bool SharpSM83::PAUSE = false;
bool SharpSM83::NEXT_INSTR = false;
SharpSM83::debugInfo SharpSM83::DEBUG_INFO = {};

SharpSM83::SharpSM83(class Bus *bus) {
    this->mBus = bus;

    PC = 0x0000;
    SP = 0x0000;
    flags.rawFlags = 0x00;
    interruptShouldBeEnabled = false;
    IME = false;

    logger = Logger::getInstance("Cpu");
}

void SharpSM83::reset() {
    PC = 0x0000;
    SP = 0x0000;
    registers.A = 0;
    registers.B = 0;
    registers.C = 0;
    registers.D = 0;
    registers.E = 0;
    registers.H = 0;
    registers.L = 0;
    flags.rawFlags = 0x00;
    interruptShouldBeEnabled = false;
    IME = false;
}

void SharpSM83::operator()() {
    size_t cycles;
    while (!Bus::GLOBAL_HALT) {
        if (!PAUSE || NEXT_INSTR) {
            NEXT_INSTR = false;
            uint8_t instr = mBus->read(PC++);
            {
                DEBUG_INFO.currentInstr = opcodeStr[instr];
                DEBUG_INFO.currentAddr = PC - 1;
                DEBUG_INFO.Z = flags.zero;
                DEBUG_INFO.C = flags.carry;
                DEBUG_INFO.HC = flags.halfCarry;
                DEBUG_INFO.N = flags.negative;
            }
            if (interruptShouldBeEnabled) { IME = true; } else {IME = false;}
            if (ENABLE_DEBUG_PRINTS) logger->log(Logger::DEBUG, "Executing instruction %s at %X", opcodeStr[instr], PC - 1);
            if (PC - 1 == 0x0293 && ENABLE_DEBUG_PRINTS) PAUSE = true;
            cycles += opcodes[instr]();
            if (dmaCycles) {dmaCycles = false; cycles += 160;}
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
                        //ENABLE_DEBUG_PRINTS = true;
                    }
                }
            }
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
    if (addDataToSP) logger->log(Logger::WARNING, "Using an unimplemented function of LD 'addDataToSP'");
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
        *reg1 = *reg2;
        cycles = 2;
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
        registers.A = mBus->read(PC++) ^ registers.A;
        cycles = 2;
    }
    else {
        registers.A = *reg ^ registers.A;
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

uint8_t SharpSM83::JR(const bool *flag, bool invert) {
    uint8_t cycles;
    if (flag == nullptr) {
        auto relAddr = (int8_t)mBus->read(PC++);
        PC = PC + relAddr;
        cycles = 3;
    } else {
        auto relAddr = (int8_t)mBus->read(PC++);
        if (invert) {
            if (!*flag) {
                PC = PC + relAddr;
                cycles = 3;
            } else {
                cycles = 2;
            }
        } else {
            if (*flag) {
                PC = PC + relAddr;
                cycles = 3;
            } else {
                cycles = 2;
            }
        }
    }
    return cycles;
}

uint8_t SharpSM83::INC(uint8_t *reg) {
    *reg = *reg + 1;

    flags.zero = *reg == 0x00;
    flags.negative = 0;
    flags.halfCarry = *reg > 0xF;

    return 1;
}

uint8_t SharpSM83::INC(uint16_t *reg) {
    *reg = *reg + 1;
    return 1;
}

uint8_t SharpSM83::DEC(uint8_t *reg) {
    *reg = *reg - 1;

    flags.zero = *reg == 0x00;
    flags.negative = 1;
    flags.halfCarry = *reg > 0xF;

    return 1;
}

uint8_t SharpSM83::SUB(const uint8_t *reg) {
    uint8_t cycles;
    uint8_t value;
    if (reg == nullptr) {
        value = mBus->read(PC++);
        cycles = 2;
    } else {
        value = *reg;
        cycles = 1;
    }

    flags.zero = (registers.A - value) == 0x00;
    flags.negative = 1;
    flags.halfCarry = (registers.A - value) > 0xF;
    flags.carry = value > registers.A;

    registers.A = registers.A - value;

    return cycles;
}

uint8_t SharpSM83::CALL(const bool *flag, bool invert) {
    uint8_t cycles;
    if (flag == nullptr) {
        uint16_t addr = mBus->read(PC + 1) << 8 | mBus->read(PC);
        PC += 2;
        SP--;
        mBus->write(SP--, PC >> 8);
        mBus->write(SP, PC & 0xFF);
        PC = addr;
        cycles = 6;
    } else {
        uint16_t addr = mBus->read(PC + 1) << 8 | mBus->read(PC);
        PC += 2;
        if (invert) {
            if (!*flag) {
                SP--;
                mBus->write(SP--, PC >> 8);
                mBus->write(SP, PC & 0xFF);
                PC = addr;
                cycles = 6;
            } else {
                cycles = 3;
            }
        } else {
            if (*flag) {
                SP--;
                mBus->write(SP--, PC >> 8);
                mBus->write(SP, PC & 0xFF);
                PC = addr;
                cycles = 6;
            } else {
                cycles = 3;
            }
        }
    }
    return cycles;
}

uint8_t SharpSM83::CP(const uint8_t *reg) {
    uint8_t cycles;
    uint16_t result;
    if (reg == nullptr) {
        result = registers.A - mBus->read(PC++);
        cycles = 2;
    } else {
        result = registers.A - *reg;
        cycles = 1;
    }

    flags.zero = (result & 0xFF) == 0x00;
    flags.negative = 1;
    flags.carry = result > 0xFF;
    flags.halfCarry = result > 0xF;

    return cycles;
}

uint8_t SharpSM83::CP(uint16_t reg) {
    uint16_t result = registers.A - mBus->read(reg);

    flags.zero = (result & 0xFF) == 0x00;
    flags.negative = 1;
    flags.carry = result > 0xFF;
    flags.halfCarry = result > 0xF;

    return 2;
}

uint8_t SharpSM83::PUSH(const uint16_t *reg) {
    if (reg == nullptr) {
        SP--;
        mBus->write(SP--, registers.A);
        mBus->write(SP, flags.rawFlags >> 4);
    } else {
        SP--;
        mBus->write(SP--, *reg >> 8);
        mBus->write(SP, *reg & 0xFF);
    }
    return 4;
}

uint8_t SharpSM83::RLA() {
    uint8_t tmpValue = registers.A;
    registers.A = (registers.A << 1) + flags.carry;

    flags.carry = tmpValue >> 7;
    flags.zero = 0;
    flags.negative = 0;
    flags.halfCarry = 0;

    return 1;
}

uint8_t SharpSM83::POP(uint16_t *reg) {
    if (reg == nullptr) {
        flags.rawFlags = mBus->read(SP++) << 4;
        registers.A = mBus->read(SP++);
    } else {
        *reg = mBus->read(SP + 1) << 8 | mBus->read(SP);
        SP += 2;
    }
    return 3;
}

uint8_t SharpSM83::RET(const bool *flag, bool invert) {
    uint8_t cycles;
    if (flag == nullptr) {
        PC = mBus->read(SP + 1) << 8 | mBus->read(SP);
        SP += 2;
        cycles = 4;
    } else {
        if (invert) {
            if (!*flag) {
                PC = mBus->read(SP + 1) << 8 | mBus->read(SP);
                SP += 2;
                cycles = 5;
            } else {
                cycles = 2;
            }
        } else {
            if (*flag) {
                PC = mBus->read(SP + 1) << 8 | mBus->read(SP);
                SP += 2;
                cycles = 5;
            } else {
                cycles = 2;
            }
        }
    }
    return cycles;
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
        mBus->write(registers.HL, (value << 1) + flags.carry);
        flags.zero = (value << 1) + flags.carry == 0x00;
        flags.carry = value >> 7;
    } else {
        uint8_t value = *reg;
        *reg = (*reg << 1) + flags.carry;
        flags.zero = (*reg << 1) + flags.carry == 0x00;
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
    logger->log(Logger::DEBUG, "Not implemented 2"); return 0;
}

uint8_t SharpSM83::INC(uint16_t reg) {
    uint8_t value = mBus->read(reg);
    value = value + 1;
    mBus->write(reg, value);

    flags.zero = value == 0x00;
    flags.negative = 0;
    flags.halfCarry = value > 0xF;

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

    flags.zero = value == 0x00;
    flags.negative = 1;
    flags.halfCarry = value > 0xF;

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
    uint16_t result;
    uint8_t cycles;
    if (reg == nullptr) {
        result = registers.A + mBus->read(PC++);
        cycles = 2;
    } else {
        result = registers.A + *reg;
        cycles = 1;
    }
    registers.A = result & 0xFF;

    flags.zero = (result & 0xFF) == 0x00;
    flags.negative = 0;
    flags.carry = result > 0xFF;
    flags.halfCarry = result > 0xF;

    return cycles;
}

uint8_t SharpSM83::ADD(uint16_t reg) {
    uint16_t result = registers.A + mBus->read(reg);
    registers.A = result & 0xFF;

    flags.zero = (result & 0xFF) == 0x00;
    flags.negative = 0;
    flags.carry = result > 0xFF;
    flags.halfCarry = result > 0xF;

    return 0;
}

uint8_t SharpSM83::ADD(uint16_t *reg1, const uint16_t *reg2) {
    uint8_t cycles;
    if (reg2 == nullptr) {
        *reg1 = *reg1 + (int8_t)mBus->read(PC++);

        flags.zero = 0;
        flags.negative = 0;
        flags.halfCarry = *reg1 > 0xF;
        flags.carry = *reg1 > 0xFF;
        cycles = 4;
    } else {
        uint32_t value = *reg1 + *reg2;
        *reg1 = *reg1 + *reg2;
        flags.negative = 0;
        flags.halfCarry = (value >> 8) > 0xF;
        flags.carry = (value >> 8) > 0xFF;
        cycles = 2;
    }

    return cycles;
}

uint8_t SharpSM83::ADC(const uint8_t *reg) {
    uint8_t cycles;
    uint16_t result;
    if (reg == nullptr) {
        result = registers.A + mBus->read(PC++) + flags.carry;
        cycles = 2;
    } else {
        result = registers.A + *reg + flags.carry;
        cycles = 1;
    }
    registers.A = (uint8_t)result;

    flags.zero = (result & 0xFF) == 0x00;
    flags.negative = 0;
    flags.halfCarry = result > 0xF;
    flags.carry = result > 0xFF;

    return cycles;
}

uint8_t SharpSM83::ADC(uint16_t reg) {
    uint16_t result = registers.A + mBus->read(reg) + flags.carry;
    registers.A = (uint8_t)result;

    flags.zero = (result & 0xFF) == 0x00;
    flags.negative = 0;
    flags.halfCarry = result > 0xF;
    flags.carry = result > 0xFF;

    return 2;
}

uint8_t SharpSM83::SUB(uint16_t reg) {
    uint8_t value = mBus->read(reg);

    flags.zero = (registers.A - value) == 0x00;
    flags.negative = 1;
    flags.halfCarry = (registers.A - value) > 0xF;
    flags.carry = value > registers.A;

    registers.A = registers.A - value;

    return 2;
}

uint8_t SharpSM83::SBC(uint8_t *reg1, uint8_t *reg2) {
    logger->log(Logger::DEBUG, "Not implemented 10"); return 0;
}

uint8_t SharpSM83::AND(const uint8_t *reg) {
    uint8_t cycles;
    if (reg == nullptr) {
        registers.A = registers.A & mBus->read(PC++);
        cycles = 2;
    } else {
        registers.A = registers.A & *reg;
        cycles = 1;
    }

    flags.zero = registers.A == 0x00;
    flags.negative = 0;
    flags.halfCarry = 1;
    flags.carry = 0;

    return cycles;
}

uint8_t SharpSM83::AND(uint16_t reg) {
    registers.A = registers.A & mBus->read(reg);

    flags.zero = registers.A == 0x00;
    flags.negative = 0;
    flags.halfCarry = 1;
    flags.carry = 0;

    return 2;
}

uint8_t SharpSM83::XOR(uint16_t reg) {
    registers.A = mBus->read(reg) ^ registers.A;

    flags.zero = registers.A == 0x00;
    flags.negative = 0;
    flags.halfCarry = 0;
    flags.carry = 0;

    return 2;
}

uint8_t SharpSM83::OR(const uint8_t *reg) {
    uint8_t cycles;
    if (reg == nullptr) {
        registers.A = registers.A | mBus->read(PC++);
        cycles = 2;
    } else {
        registers.A = registers.A | *reg;
        cycles = 1;
    }

    flags.zero = registers.A == 0x00;
    flags.negative = 0;
    flags.halfCarry = 0;
    flags.carry = 0;

    return cycles;
}

uint8_t SharpSM83::OR(uint16_t reg) {
    registers.A = registers.A | mBus->read(reg);

    flags.zero = registers.A == 0x00;
    flags.negative = 0;
    flags.halfCarry = 0;
    flags.carry = 0;

    return 2;
}

uint8_t SharpSM83::RRCA() {
    flags.carry = registers.A & 0x01;
    registers.A = registers.A >> 1;

    flags.zero = 0;
    flags.negative = 0;
    flags.halfCarry = 0;

    return 0;
}

uint8_t SharpSM83::STOP() { // The STOP instruction seems to be resest after a key press
    //PAUSE = true; // TODO skipping this instruction

    return 0;
}

uint8_t SharpSM83::RRA() {
    flags.carry = registers.A & 0x01;
    registers.A = (registers.A >> 1) + (flags.carry << 7);

    flags.zero = 0;
    flags.negative = 0;
    flags.halfCarry = 0;

    return 1;
}

uint8_t SharpSM83::DAA() {
    if (!flags.negative) {
        if (flags.carry || registers.A > 0x99) {registers.A += 0x60; flags.carry = 1;}
        if (flags.halfCarry || (registers.A & 0x0F) > 0x09) {registers.A += 0x06;}
    } else {
        if (flags.carry) registers.A -= 0x60;
        if (flags.halfCarry) registers.A -= 0x06;
    }

    flags.zero = registers.A == 0x00;
    flags.halfCarry = 0;

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
    /*logger->log(Logger::DEBUG, "Not implemented 21");*/ return 0; // TODO: Implements HALT
}

uint8_t SharpSM83::JP(const bool *flag, bool invert) {
    uint8_t cycles;
    if (flag == nullptr) {
        PC = mBus->read(PC + 1) << 8 | mBus->read(PC);
        cycles = 4;
    } else {
        if (invert) {
            if (!*flag) {
                PC = mBus->read(PC + 1) << 8 | mBus->read(PC);
                cycles = 4;
            } else {
                PC = PC + 2;
                cycles = 3;
            }
        } else {
            if (*flag) {
                PC = mBus->read(PC + 1) << 8 | mBus->read(PC);
                cycles = 4;
            } else {
                PC = PC + 2;
                cycles = 3;
            }
        }
    }

    return cycles;
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
    uint8_t cycles = 0;
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
        value = value >> 1;
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

uint8_t SharpSM83::RR(uint8_t *reg) {
    logger->log(Logger::DEBUG, "Not implemented 30"); return 0;
}

uint8_t SharpSM83::SLA(uint8_t *reg) {
    uint8_t cycles;
    uint8_t value;
    if (reg == nullptr) {
        value = mBus->read(registers.HL);
        flags.carry = value >> 7;
        mBus->write(registers.HL, value << 1);
        cycles = 4;
    } else {
        flags.carry = *reg >> 7;
        *reg = *reg << 1;
        value = *reg;
        cycles = 2;
    }

    flags.zero = *reg == 0x00;
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
        mBus->write(registers.HL, value >> 1);
        cycles = 4;
    } else {
        flags.carry = *reg & 0x01;
        *reg = *reg >> 1;
        value = *reg;
        cycles = 2;
    }

    flags.zero = *reg == 0x00;
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
