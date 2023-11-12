#ifndef EMU_GAMEBOY_BUS_H
#define EMU_GAMEBOY_BUS_H

#define RAM_SIZE 8192
#define BOOT_ROM_SIZE 0xFF
#define HRAM_SIZE 12
#define BOOT_ROM_LOCATION "boot/DMG_ROM.bin"

#include <array>
#include <fstream>

#include "InputManager.h"
#include "Ppu.h"
#include "Screen.h"
#include "SharpSM83.h"

class Bus {
public:
    Bus();

    static bool GLOBAL_HALT;

    void write(uint16_t addr, uint8_t data);
    uint8_t read(uint16_t addr);

    void reset();

    inline static union {
        struct {
            bool aOrRight : 1;
            bool bOrLeft : 1;
            bool selectOrUp : 1;
            bool startOrDown : 1;
            bool selectDpad : 1;
            bool selectButtons : 1;
            bool unused : 2;
        };
        uint8_t raw;
    } JOYP;
private:
    class SharpSM83 *cpu;
    class Ppu *ppu;
    class InputManager *inputManager;
    Logger *logger;

    const char *romName;
    uint8_t disableBootRom;

    uint8_t bootRom[BOOT_ROM_SIZE]{};
    uint8_t ram[RAM_SIZE]{};
    uint8_t hram[HRAM_SIZE]{};
    inline static std::array<uint8_t, 32*1024> gameRom;

    void readGameRom(const char *filename);
    void readBootRom();
};


#endif //EMU_GAMEBOY_BUS_H
