#ifndef EMU_GAMEBOY_BUS_H
#define EMU_GAMEBOY_BUS_H

#define RAM_SIZE 8192
#define BOOT_ROM_SIZE 0xFF
#define HRAM_SIZE 12
#define BOOT_ROM_LOCATION "resources/boot/DMG_ROM.bin"

#include <fstream>

#include "Screen.h"
#include "SharpSM83.h"

class Bus {
public:
    Bus();

    void write(uint16_t addr, uint8_t data);
    uint8_t read(uint16_t addr);

private:
    Screen *screen;
    class SharpSM83 *cpu;
    Logger *logger;

    uint8_t bootRom[BOOT_ROM_SIZE];
    uint8_t ram[RAM_SIZE];
    uint8_t vram[RAM_SIZE];
    uint8_t hram[HRAM_SIZE];

    void readBootRom();
};


#endif //EMU_GAMEBOY_BUS_H
