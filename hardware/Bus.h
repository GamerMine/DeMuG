#ifndef EMU_GAMEBOY_BUS_H
#define EMU_GAMEBOY_BUS_H

#define RAM_SIZE 8192

#include "Screen.h"

class Bus {
public:
    Bus();

    void write(uint16_t addr, uint8_t data);
    uint8_t read(uint16_t addr);

private:
    Screen *screen;
    uint8_t ram[RAM_SIZE];
    uint8_t vram[RAM_SIZE];
};


#endif //EMU_GAMEBOY_BUS_H
