#include "SharpSM83.h"

SharpSM83::SharpSM83(Bus *bus) {
    mBus = bus;

    PC = 0x0000;
    SP = 0x0000;
    flags.rawFlags = 0x00;
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

uint8_t SharpSM83::NOP() {
    return 1;
}

uint8_t SharpSM83::LD(uint16_t *reg1, uint16_t *reg2, bool addDataToSP) { // TODO: Support for addDataToSP
    uint8_t cycles;
    if (reg1 == nullptr) {
        uint16_t addr = mBus->read(PC + 1) << 8 | mBus->read(PC);
        mBus->write(addr, SP & 0xFF);
        mBus->write(addr + 1, SP >> 8);
        cycles = 5;
    }
    else if (reg2 == nullptr) {
        *reg1 = mBus->read(PC + 1) << 8 | mBus->read(PC);
        PC += 2;
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
        int8_t relAddr = mBus->read(PC++);
        PC = PC + relAddr;
        cycles = 3;
    } else {
        int8_t relAddr = mBus->read(PC++);
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
    return 1;
}

uint8_t SharpSM83::BIT(uint8_t bit, uint8_t *reg) {
    uint8_t cycles;
    if (reg == nullptr) {
        flags.zero = (mBus->read(registers.HL) & ((bit + 1) * 8)) == 0;
        cycles = 3;
    } else {
        flags.zero = (*reg & ((bit + 1) * 8)) == 0;
        cycles = 2;
    }

    flags.negative = 0;
    flags.halfCarry = 1;

    return cycles;
}
