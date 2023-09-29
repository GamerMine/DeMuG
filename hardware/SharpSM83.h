#ifndef EMU_GAMEBOY_SHARPSM83_H
#define EMU_GAMEBOY_SHARPSM83_H

#include <cstdint>

class SharpSM83 {
public:
    SharpSM83();

private:
    struct flags {
        bool zero;
        bool negative;
        bool halfCarry;
        bool carry;
    };

    uint16_t PC, SP;
    uint8_t A;
    uint8_t B, C, D, E, H, L;
};


#endif //EMU_GAMEBOY_SHARPSM83_H
