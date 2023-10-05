#include "SharpSM83.h"

bool enableSleep = false;
Logger *SharpSM83::logger;

SharpSM83::SharpSM83(class Bus *bus) {
    this->mBus = bus;

    PC = 0x0000;
    SP = 0x0000;
    flags.rawFlags = 0x00;

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
}

[[noreturn]] void SharpSM83::operator()() {
    using namespace std::chrono_literals;
    while (true) {
        //if (PC == 0x0034) enableSleep = true;
        //if (enableSleep) std::this_thread::sleep_for(200ms);
        uint8_t instr = mBus->read(PC++);
        //logger->log(Logger::DEBUG, "%sExecuting instruction: %X at %X", Colors::BLUE, instr, PC-1);
        opcodes[instr]();
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

uint8_t SharpSM83::LD(uint16_t *reg1, uint16_t *reg2, bool addDataToSP) { // TODO: Support for addDataToSP
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

uint8_t SharpSM83::LD(uint16_t reg1, uint8_t *reg2, bool incrementReg1, bool decrementReg1) {
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

uint8_t SharpSM83::LD(uint8_t *reg1, uint8_t *reg2) {
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

uint8_t SharpSM83::LD(uint8_t reg1, uint8_t *reg2) {
    mBus->write(0xFF00 + reg1, *reg2);
    return 2;
}

uint8_t SharpSM83::LD(uint8_t *reg1, uint16_t reg2, bool incrementReg2, bool decrementReg2) {
    *reg1 = mBus->read(reg2);
    if (incrementReg2) registers.HL++;
    if (decrementReg2) registers.HL--;
    return 2;
}

uint8_t SharpSM83::LDH(uint8_t *reg) {
    if (reg == nullptr) {
        mBus->write(0xFF00 + mBus->read(PC++), registers.A);
    } else {
        registers.A = mBus->read(0xFF00 + mBus->read(PC++));
    }
    return 3;
}

uint8_t SharpSM83::XOR(uint8_t *reg) {
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

uint8_t SharpSM83::JR(bool *flag, bool invert) {
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

uint8_t SharpSM83::SUB(uint8_t *reg) {
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

uint8_t SharpSM83::CALL(bool *flag, bool invert) {
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
                mBus->write(SP--, PC & 0xFF);
                mBus->write(SP, PC >> 8);
                PC = addr;
                cycles = 6;
            } else {
                cycles = 3;
            }
        } else {
            if (*flag) {
                SP--;
                mBus->write(SP--, PC & 0xFF);
                mBus->write(SP, PC >> 8);
                PC = addr;
                cycles = 6;
            } else {
                cycles = 3;
            }
        }
    }
    return cycles;
}

uint8_t SharpSM83::CP(uint8_t *reg) {
    uint8_t cycles;
    uint16_t result;
    if (reg == nullptr) {
        result = registers.A - mBus->read(PC++);
        cycles = 2;
    } else {
        result = registers.A - *reg;
        cycles = 1;
    }

    flags.zero = (uint8_t)result == 0x00;
    flags.negative = 1;
    flags.carry = result > 0xFF;
    flags.halfCarry = result > 0xF;

    return cycles;
}

uint8_t SharpSM83::CP(uint8_t reg) {
    uint16_t result = registers.A - mBus->read(reg);

    flags.zero = result == 0x00;
    flags.negative = 1;
    flags.carry = result > 0xFF;
    flags.halfCarry = result > 0xF;

    return 2;
}

uint8_t SharpSM83::PUSH(uint16_t *reg) {
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

uint8_t SharpSM83::RET(bool *flag, bool invert) {
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

uint8_t SharpSM83::BIT(uint8_t bit, uint8_t *reg) {
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
    logger->log(Logger::DEBUG, "Not implemented"); return 0;
}

uint8_t SharpSM83::LD(uint8_t *reg1, uint8_t reg2) {
    logger->log(Logger::DEBUG, "Not implemented"); return 0;
}

uint8_t SharpSM83::INC(uint16_t reg) {
    logger->log(Logger::DEBUG, "Not implemented"); return 0;
}

uint8_t SharpSM83::DEC(uint16_t *reg) {
    logger->log(Logger::DEBUG, "Not implemented"); return 0;
}

uint8_t SharpSM83::DEC(uint16_t reg) {
    logger->log(Logger::DEBUG, "Not implemented"); return 0;
}

uint8_t SharpSM83::RLCA() {
    logger->log(Logger::DEBUG, "Not implemented"); return 0;
}

uint8_t SharpSM83::ADD(uint8_t *reg1, uint8_t *reg2) {
    logger->log(Logger::DEBUG, "Not implemented"); return 0;
}

uint8_t SharpSM83::ADC(uint8_t *reg1, uint8_t *reg2) {
    logger->log(Logger::DEBUG, "Not implemented"); return 0;
}

uint8_t SharpSM83::SUB(uint8_t reg) {
    logger->log(Logger::DEBUG, "Not implemented"); return 0;
}

uint8_t SharpSM83::SBC(uint8_t *reg1, uint8_t *reg2) {
    logger->log(Logger::DEBUG, "Not implemented"); return 0;
}

uint8_t SharpSM83::AND(uint8_t *reg1, uint8_t *reg2) {
    logger->log(Logger::DEBUG, "Not implemented"); return 0;
}

uint8_t SharpSM83::XOR(uint16_t reg) {
    logger->log(Logger::DEBUG, "Not implemented"); return 0;
}

uint8_t SharpSM83::OR(uint8_t *reg1, uint8_t *reg2) {
    logger->log(Logger::DEBUG, "Not implemented"); return 0;
}

uint8_t SharpSM83::ADD(uint16_t *reg1, uint16_t *reg2) {
    logger->log(Logger::DEBUG, "Not implemented"); return 0;
}

uint8_t SharpSM83::RRCA() {
    logger->log(Logger::DEBUG, "Not implemented"); return 0;
}

uint8_t SharpSM83::STOP() {
    logger->log(Logger::DEBUG, "Not implemented"); return 0;
}

uint8_t SharpSM83::RRA() {
    logger->log(Logger::DEBUG, "Not implemented"); return 0;
}

uint8_t SharpSM83::DAA() {
    logger->log(Logger::DEBUG, "Not implemented"); return 0;
}

uint8_t SharpSM83::CPL() {
    logger->log(Logger::DEBUG, "Not implemented"); return 0;
}

uint8_t SharpSM83::SCF() {
    logger->log(Logger::DEBUG, "Not implemented"); return 0;
}

uint8_t SharpSM83::CCF() {
    logger->log(Logger::DEBUG, "Not implemented"); return 0;
}

uint8_t SharpSM83::HALT() {
    logger->log(Logger::DEBUG, "Not implemented"); return 0;
}

uint8_t SharpSM83::JP(bool *flag, bool invert) {
    logger->log(Logger::DEBUG, "Not implemented"); return 0;
}

uint8_t SharpSM83::JP(uint16_t *reg) {
    logger->log(Logger::DEBUG, "Not implemented"); return 0;
}

uint8_t SharpSM83::RST(uint16_t addr) {
    logger->log(Logger::DEBUG, "Not implemented"); return 0;
}

uint8_t SharpSM83::RETI() {
    logger->log(Logger::DEBUG, "Not implemented"); return 0;
}

uint8_t SharpSM83::DI() {
    logger->log(Logger::DEBUG, "Not implemented"); return 0;
}

uint8_t SharpSM83::EI() {
    logger->log(Logger::DEBUG, "Not implemented"); return 0;
}

uint8_t SharpSM83::RLC(uint8_t *reg) {
    logger->log(Logger::DEBUG, "Not implemented"); return 0;
}

uint8_t SharpSM83::RRC(uint8_t *reg) {
    logger->log(Logger::DEBUG, "Not implemented"); return 0;
}

uint8_t SharpSM83::RR(uint8_t *reg) {
    logger->log(Logger::DEBUG, "Not implemented"); return 0;
}

uint8_t SharpSM83::SLA(uint8_t *reg) {
    logger->log(Logger::DEBUG, "Not implemented"); return 0;
}

uint8_t SharpSM83::SRA(uint8_t *reg) {
    logger->log(Logger::DEBUG, "Not implemented"); return 0;
}

uint8_t SharpSM83::SWAP(uint8_t *reg) {
    logger->log(Logger::DEBUG, "Not implemented"); return 0;
}

uint8_t SharpSM83::SRL(uint8_t *reg) {
    logger->log(Logger::DEBUG, "Not implemented"); return 0;
}

uint8_t SharpSM83::RES(uint8_t bit, uint8_t *reg) {
    logger->log(Logger::DEBUG, "Not implemented"); return 0;
}

uint8_t SharpSM83::SET(uint8_t bit, uint8_t *reg) {
    logger->log(Logger::DEBUG, "Not implemented"); return 0;
}
