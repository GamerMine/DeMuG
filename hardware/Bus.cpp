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

#include "Bus.h"

bool Bus::GLOBAL_HALT = false;

Bus::Bus() {
    logger = Logger::getInstance("Bus");
    disableBootRom = false;

    ppu = new Ppu(this);
    std::thread ppuThread(std::ref(*ppu));
    apu = new Apu(this);
    cpu = new SharpSM83(this);
    std::thread cpuThread(std::ref(*cpu));
    inputManager = new InputManager(this);
    timer = new Timer(this);
    serial = new SerialIO(this);

    readBootRom();
    cartridge = CartridgeHelper::readGameRom("Guillaume/Nouveau dossier/Kirby's Dream Land.gb");

    ppuThread.join();
    cpuThread.join();

    apu->closeConnection();
}

void Bus::write(uint16_t addr, uint8_t data) {
    if (addr <= 0x00FF && !disableBootRom) logger->log(Logger::WARNING, "Trying to write in an unauthorized area: 0x%X", addr); // We should not write into the boot rom (Read Only)
    else if (addr <= 0x00FF && disableBootRom) cartridge->write(addr, data); // Game cartridge if boot rom is unmapped
    else if (addr >= 0x0100 && addr <= 0x7FFF) cartridge->write(addr, data); // Game cartridge
    else if (addr >= 0x8000 && addr <= 0x9FFF) ppu->write(addr, data); // VRAM
    else if (addr >= 0xA000 && addr <= 0xBFFF) cartridge->write(addr, data); // Game cartridge RAM
    else if (addr >= 0xC000 && addr <= 0xDFFF) ram[addr - 0xC000] = data; // WRAM
    else if (addr >= 0xE000 && addr <= 0xFDFF) ram[addr - 0xE000] = data; // Mirror of WRAM
    else if (addr >= 0xFE00 && addr <= 0xFE9F) ppu->write(addr, data); // PPU OAM
    else if (addr == 0xFF00) JOYP.raw = data; // JOYP Register
    else if (addr == 0xFF01) SerialIO::SB = data; // SB Register (Serial Transfer)
    else if (addr == 0xFF02) {  // SC Register (Serial Transfer)
        if (SerialIO::SC.transferEnable && !(data >> 7)) SharpSM83::IF.serial = 1;
        SerialIO::SC.raw = data;
    }
    else if (addr == 0xFF04) Timer::DIV = 0x00; // DIV Register (Timer)
    else if (addr == 0xFF05) Timer::TIMA = data; // TIMA Register (Timer)
    else if (addr == 0xFF06) Timer::TMA = data; // TMA Register (Timer)
    else if (addr == 0xFF07) Timer::TAC.raw = data & 0b111; // TAC Register (Timer)
    else if (addr == 0xFF0F) SharpSM83::IF.raw = data; // Writing to Interrupt Flags
    else if (addr >= 0xFF10 && addr <= 0xFF3F) apu->write(addr, data); // Writing to APU
    else if (addr >= 0xFF40 && addr <= 0xFF4B) ppu->write(addr, data); // PPU Registers
    else if (addr == 0xFF50 && data != 0x00) {
        logger->log(Logger::DEBUG, "Boot rom disconnected from the bus");
        disableBootRom = true;
    }
    else if (addr >= 0xFF80 && addr <= 0xFFFE) hram[addr - 0xFF80] = data; // HRAM
    else if (addr == 0xFFFF) SharpSM83::IE.raw = data; // Writing to Interrupt Enable
    //else logger->log(Logger::DEBUG, "%sTrying to write at %X, but address is not accessible", Colors::LOG_RED, addr);
}

uint8_t Bus::read(uint16_t addr) {
    uint8_t value = 0xFF;
    if (addr <= 0x00FF && !disableBootRom) value = bootRom[addr]; // DMG BOOT ROM if mapped
    else if (addr <= 0x00FF && disableBootRom) value = cartridge->read(addr); // Game cartridge if boot rom is unmapped
    else if (addr >= 0x0100 && addr <= 0x7FFF) value = cartridge->read(addr); // Game cartridge
    else if (addr >= 0x8000 && addr <= 0x9FFF) value = ppu->read(addr); // VRAM
    else if (addr >= 0xA000 && addr <= 0xBFFF) value = cartridge->read(addr); // Game cartridge RAM
    else if (addr >= 0xC000 && addr <= 0xDFFF) value = ram[addr - 0xC000]; // WRAM
    else if (addr >= 0xE000 && addr <= 0xFDFF) value = ram[addr - 0xE000]; // Mirror of WRAM
    else if (addr >= 0xFE00 && addr <= 0xFE9F) value = ppu->read(addr); // OAM
    else if (addr == 0xFF00) { // Joypad register
        if (!JOYP.selectButtons) value = InputManager::JOY_BTN.raw;
        else if (!JOYP.selectDpad) value = InputManager::JOY_DPAD.raw;
        else value = 0x0F;
    }
    else if (addr == 0xFF01) value = SerialIO::SB; // SB register (Serial Transfer)
    else if (addr == 0xFF02) value = SerialIO::SC.raw; // SC register (Serial Transfer)
    else if (addr == 0xFF04) value = Timer::DIV >> 8; // DIV register (Timer)
    else if (addr == 0xFF05) value = Timer::TIMA; // TIMA Register (Timer)
    else if (addr == 0xFF06) value = Timer::TMA; // TMA Register (Timer)
    else if (addr == 0xFF07) value = Timer::TAC.raw & 0x07; // TAC Register (Timer)
    else if (addr == 0xFF0F) value = SharpSM83::IF.raw; // Interrupt Flag
    else if (addr >= 0xFF10 && addr <= 0xFF3F) value = apu->read(addr); // APU
    else if (addr >= 0xFF40 && addr <= 0xFF4B) value = ppu->read(addr); // PPU
    else if (addr >= 0xFF80 && addr <= 0xFFFE) value = hram[addr - 0xFF80]; // HRAM
    else if (addr == 0xFFFF) value = SharpSM83::IE.raw;
    //else logger->log(Logger::DEBUG, "Trying to read at %X, but address is not accessible", addr);

    return value;
}

void Bus::tickApu() {
    apu->tick();
}

void Bus::tick(uint8_t mCycle) {
    timer->tick(mCycle);
    ppu->tick(mCycle);
    inputManager->tick();
    serial->tick();
}

void Bus::readBootRom() {
    uint16_t i;
    logger->log(Logger::DEBUG, "Reading boot rom...");
    std::ifstream file(BOOT_ROM_LOCATION, std::ios::binary);

    if (!file.good()) {
        logger->log(Logger::CRITICAL, "File not found: %s", BOOT_ROM_LOCATION);
        exit(1);
    }

    char byte;

    while (!file.eof()) {
        file.read(&byte, 1);
        bootRom[i++] = (uint8_t)byte;
    }

    file.close();
}

void Bus::reset() {
    using namespace std::chrono_literals;
    ppu->reset();
    std::this_thread::sleep_for(30ms);
    cpu->reset();
}
