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
    romName = nullptr;
    disableBootRom = true;

    readBootRom();
    readGameRom("ie_push.gb");

    ppu = new Ppu(this);
    std::thread ppuThread(std::ref(*ppu));
    cpu = new SharpSM83(this);
    std::thread cpuThread(std::ref(*cpu));
    inputManager = new InputManager(this);
    std::thread inputsThread(std::ref(*inputManager));
    timer = new Timer(this);
    std::thread timerThread(std::ref(*timer));

    cpuThread.join();
    ppuThread.join();
    inputsThread.join();
    timerThread.join();
}

void Bus::write(uint16_t addr, uint8_t data) {
    if (addr <= 0x00FF && !disableBootRom) { // DMG BOOT ROM
        // We should not write into the boot rom (Read Only)
        logger->log(Logger::WARNING, "Trying to write in an unauthorized area: 0x%X", addr);
    }
    if (addr >= 0x8000 && addr <= 0x9FFF) ppu->write(addr, data); // VRAM
    if (addr >= 0xC000 && addr <= 0xDFFF) ram[addr - 0xC000] = data; // WRAM
    if (addr >= 0xE000 && addr <= 0xFDFF) ram[addr - 0xE000] = data; // Mirror of WRAM
    if (addr >= 0xFE00 && addr <= 0xFE9F) ppu->write(addr, data); // PPU OAM
    if (addr == 0xFF00) JOYP.raw = data; // JOYP Register
    if (addr == 0xFF04) Timer::DIV = 0x00; // DIV Register (Timer)
    if (addr == 0xFF05) Timer::TIMA = data; // TIMA Register (Timer)
    if (addr == 0xFF06) Timer::TMA = data; // TMA Register (Timer)
    if (addr == 0xFF07) {
        Timer::TAC.raw = data & 0b111; // TAC Register (Timer)
        //if (data == 0x05) SharpSM83::PAUSE = true;
    }
    if (addr == 0xFF0F) SharpSM83::IF.raw = data; // Writing to Interrupt Flags
    if (addr >= 0xFF40 && addr <= 0xFF49) ppu->write(addr, data); // PPU Registers
    if (addr == 0xFF50 && data != 0x00) {
        logger->log(Logger::DEBUG, "Boot rom disconnected from the bus");
        disableBootRom = true;
    }
    if (addr >= 0xFF80 && addr <= 0xFFFE) hram[addr - 0xFF80] = data; // HRAM
    if (addr == 0xFFFF) SharpSM83::IE.raw = data; // Writing to Interrupt Enable
}

uint8_t Bus::read(uint16_t addr) {
    uint8_t value = 0xFF;
    if (addr <= 0x00FF && !disableBootRom) value = bootRom[addr]; // DMG BOOT ROM if mapped
    if (addr <= 0x00FF && disableBootRom) value = gameRom[addr]; // Game cartridge if boot rom is unmapped
    if (addr >= 0x0100 && addr <= 0x7FFF && romName != nullptr) value = gameRom[addr]; // Game cartridge
    if (addr >= 0x8000 && addr <= 0x9FFF) value = ppu->read(addr); // VRAM
    if (addr >= 0xC000 && addr <= 0xDFFF) value = ram[addr - 0xC000]; // WRAM
    if (addr >= 0xE000 && addr <= 0xFDFF) value = ram[addr - 0xE000]; // Mirror of WRAM
    if (addr >= 0xFE00 && addr <= 0xFE9F) value = ppu->read(addr); // OAM
    if (addr == 0xFF00) {
        if (!JOYP.selectButtons) value = InputManager::JOY_BTN.raw;
        else if (!JOYP.selectDpad) value = InputManager::JOY_DPAD.raw;
        else value = 0x0F;
    } // Joypad register
    if (addr == 0xFF04) value = Timer::DIV; // DIV register (Timer)
    if (addr == 0xFF05) value = Timer::TIMA; // TIMA Register (Timer)
    if (addr == 0xFF06) value = Timer::TMA; // TMA Register (Timer)
    if (addr == 0xFF07) value = Timer::TAC.raw & 0x03; // TAC Register (Timer)
    if (addr == 0xFF0F) value = SharpSM83::IF.raw; // Interrupt Flag
    if (addr >= 0xFF40 && addr <= 0xFF45) value = ppu->read(addr); // PPU
    if (addr >= 0xFF80 && addr <= 0xFFFE) value = hram[addr - 0xFF80]; // HRAM
    if (addr == 0xFFFF) value = SharpSM83::IE.raw;

    return value;
}

void Bus::readGameRom(const char *filename) {
    logger->log(Logger::DEBUG, "Reading game : %s", filename);
    std::ifstream file(filename, std::ios::binary);

    if (!file.good()) {
        logger->log(Logger::CRITICAL, "File not found: %s", filename);
        return;
    }
    romName = filename;

    file.read(reinterpret_cast<char *>(gameRom.data()), sizeof(uint8_t) * gameRom.size());
    file.close();
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
