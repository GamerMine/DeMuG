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

#include "Ppu.h"

Ppu::Ppu(Bus *bus) {
    mBus = bus;
    LCDC.raw = 0x91;
    STAT.raw = 0x81;
    SCY = 0x00;
    SCX = 0x00;
    WX = 0x00;
    WY = 0x00;
    LY = 0x91;
    LYC = 0x00;
    BGP.raw = 0xFC;
    OBP0.raw = 0x00;
    OBP1.raw = 0x00;
    DMA = 0x00;

    for (uint8_t &i : vram) i = 0x00;
    for (uint8_t &i : OAM)  i = 0x00;

    screen = new Screen(this);
}

void Ppu::reset() {
    screen->reset();
    LCDC.raw = 0x91;
    STAT.raw = 0x81;
    SCY = 0x00;
    SCX = 0x00;
    WX = 0x00;
    WY = 0x00;
    LY = 0x91;
    LYC = 0x00;
    BGP.raw = 0xFC;
    OBP0.raw = 0x00;
    OBP1.raw = 0x00;

    for (uint8_t &i : vram) i = 0x00;
    for (uint8_t &i : OAM)  i = 0x00;
}

uint8_t Ppu::read(uint16_t addr) const {
    uint8_t value = 0xFF;
    if (addr >= 0x8000 && addr <= 0x9FFF) value = vram[addr - 0x8000];
    if (addr >= 0xFE00 && addr <= 0xFE9F) value = OAM[addr - 0xFE00];
    if (addr == 0xFF40) value = LCDC.raw;
    if (addr == 0xFF41) { value = STAT.raw; /*Logger::getInstance("PPU")->log(Logger::DEBUG, "READ STAT : %X", STAT.raw);*/ }
    if (addr == 0xFF42) value = SCY;
    if (addr == 0xFF43) value = SCX;
    if (addr == 0xFF44) value = LY;
    if (addr == 0xFF45) value = LYC;
    if (addr == 0xFF46) value = DMA;
    if (addr == 0xFF47) value = BGP.raw;
    if (addr == 0xFF48) value = OBP0.raw;
    if (addr == 0xFF49) value = OBP1.raw;
    if (addr == 0xFF4A) value = WY;
    if (addr == 0xFF4B) value = WX;

    return value;
}

void Ppu::write(uint16_t addr, uint8_t data) {
    if (addr >= 0x8000 && addr <= 0x9FFF) vram[addr - 0x8000] = data;
    if (addr >= 0xFE00 && addr <= 0xFE9F) OAM[addr - 0xFE00] = data;
    if (addr == 0xFF40) LCDC.raw = data;
    if (addr == 0xFF41) STAT.raw = data;
    if (addr == 0xFF42) SCY = data;
    if (addr == 0xFF43) SCX = data;
    if (addr == 0xFF44) Logger::getInstance("PPU")->log(Logger::WARNING, "LY register is read-only");
    if (addr == 0xFF45) LYC = data;
    if (addr == 0xFF46) {
        DMA = data;
        startTransfer();
    }
    if (addr == 0xFF47) BGP.raw = data;
    if (addr == 0xFF48) OBP0.raw = data;
    if (addr == 0xFF49) OBP1.raw = data;
    if (addr == 0xFF4A) WY = data;
    if (addr == 0xFF4B) WX = data;
}

void Ppu::startTransfer() {
    uint16_t page = DMA;

    for (uint8_t lAddr = 0x00; lAddr < 0xA0; lAddr++) {
        OAM[lAddr] = mBus->read((page << 8) + lAddr);
    }
    //mBus->tick(160);
}

void Ppu::tick(uint8_t mCycle) const {
    screen->tick(mCycle);
}

void Ppu::runPpu() const {
    screen->run();
}

void Ppu::closeConnection() {
    screen->close();
}
