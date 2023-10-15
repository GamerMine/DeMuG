#ifndef EMU_GAMEBOY_BUS_H
#define EMU_GAMEBOY_BUS_H

#define RAM_SIZE 8192
#define BOOT_ROM_SIZE 0xFF
#define HRAM_SIZE 12
#define BOOT_ROM_LOCATION "boot/DMG_ROM.bin"

#include <fstream>

#include "InputManager.h"
#include "Ppu.h"
#include "Screen.h"
#include "SharpSM83.h"

class Bus {
public:
    Bus();

    static bool GLOBAL_HALT;

    void sendPpuWorkSignal();
    void write(uint16_t addr, uint8_t data);
    uint8_t read(uint16_t addr);

    void reset();

    union {
        struct {
            bool unused : 3;
            bool joypad : 1;
            bool serial : 1;
            bool timer  : 1;
            bool lcd    : 1;
            bool vblank : 1;
        };
        uint8_t raw;
    } IE {}; // Interrupt Enable controls handlers calls

    union {
        struct {
            bool unused : 3;
            bool joypad : 1;
            bool serial : 1;
            bool timer  : 1;
            bool lcd    : 1;
            bool vblank : 1;
        };
        uint8_t raw;
    } IF {}; // Interrupt flag controls handlers request

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
    std::array<uint8_t, 32*1024> gameRom;

    void readGameRom(const char *filename);
    void readBootRom();
};


#endif //EMU_GAMEBOY_BUS_H
