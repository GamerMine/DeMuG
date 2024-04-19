/*
 *  ____
 * /\  _`\                                       /'\_/`\  __
 * \ \ \L\_\     __      ___ ___      __   _ __ /\      \/\_\    ___      __
 *  \ \ \L_L   /'__`\  /' __` __`\  /'__`\/\`'__\ \ \__\ \/\ \ /' _ `\  /'__`\
 *   \ \ \/, \/\ \L\.\_/\ \/\ \/\ \/\  __/\ \ \/ \ \ \_/\ \ \ \/\ \/\ \/\  __/
 *    \ \____/\ \__/.\_\ \_\ \_\ \_\ \____\\ \_\  \ \_\\ \_\ \_\ \_\ \_\ \____\
 *     \/___/  \/__/\/_/\/_/\/_/\/_/\/____/ \/_/   \/_/ \/_/\/_/\/_/\/_/\/____/
 *
 * Copyright (c) 2024-2024 GamerMine <maxime-sav@outlook.fr>
 *
 * This Source Code From is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/ .
 */

#include "Apu.h"
#include "audio/SC2Pulse.h"
#include "audio/SC3Wave.h"
#include "audio/SC4Noise.h"

Apu::Apu(class Bus *bus) {
    mBus = bus;
    logger = Logger::getInstance("Apu");
    rate = 0;

    InitAudioDevice();
    SetMasterVolume(DEFAULT_MASTER_VOLUME);

    pulseSweep = new SC1PulseSweep();
    pulse = new SC2Pulse();
    wave = new SC3Wave();
    noise = new SC4Noise();

    NR50.raw = 0x77;
    NR51.raw = 0xF3;
    NR52.raw = 0xF1;
}

void Apu::write(uint16_t addr, uint8_t data) {
    //logger->log(Logger::DEBUG, "Writing %X to %X", data, addr);

    if (addr == 0xFF10 && NR52.audioOnOff)      SC1PulseSweep::NR10.raw = data;
    else if (addr == 0xFF11) {
        if (NR52.audioOnOff) SC1PulseSweep::NR11.raw = data;
        else SC1PulseSweep::NR11.raw = data & 0x3F;
    }
    else if (addr == 0xFF12 && NR52.audioOnOff) {
        SC1PulseSweep::NR12.raw = data;
        if ((SC1PulseSweep::NR12.raw & 0xF8) != 0x00) SC1PulseSweep::DAC = true;
        else { SC1PulseSweep::DAC = false; NR52.ch1On = 0; StopAudioStream(pulseSweep->audioStream); }
    }
    else if (addr == 0xFF13 && NR52.audioOnOff) SC1PulseSweep::NR13 = data;
    else if (addr == 0xFF14 && NR52.audioOnOff) {
        SC1PulseSweep::NR14.raw = data;
        if (data >> 7 && SC1PulseSweep::DAC) {
            NR52.ch1On = 1;
            SC1PulseSweep::sweepDone = true;
            SC1PulseSweep::sweepTick = 0;
            SC1PulseSweep::NR12Temp.raw = SC1PulseSweep::NR12.raw;
            SC1PulseSweep::currentVolume = static_cast<int8_t>(SC1PulseSweep::NR12Temp.initialVolume);
            SetAudioStreamVolume(pulseSweep->audioStream, SC1PulseSweep::NR12Temp.initialVolume * (1.0f / 15.0f));
            PlayAudioStream(pulseSweep->audioStream);
        }
    }
    else if (addr == 0xFF16) {
        if (NR52.audioOnOff) SC2Pulse::NR21.raw = data;
        else SC2Pulse::NR21.raw = data & 0x3F;
    }
    else if (addr == 0xFF17 && NR52.audioOnOff) {
        SC2Pulse::NR22.raw = data;
        if ((SC2Pulse::NR22.raw & 0xF8) != 0x00) SC2Pulse::DAC = true;
        else { SC2Pulse::DAC = false; NR52.ch2On = 0; StopAudioStream(pulse->audioStream); }
    }
    else if (addr == 0xFF18 && NR52.audioOnOff) SC2Pulse::NR23 = data;
    else if (addr == 0xFF19 && NR52.audioOnOff) {
        SC2Pulse::NR24.raw = data;
        if (data >> 7 && SC2Pulse::DAC) {
            NR52.ch2On = 1;
            SC2Pulse::NR22Temp.raw = SC2Pulse::NR22.raw;
            SC2Pulse::currentVolume = static_cast<int8_t>(SC2Pulse::NR22Temp.initialVolume);
            SetAudioStreamVolume(pulse->audioStream, SC2Pulse::NR22Temp.initialVolume * (1.0f / 15.0f));
            PlayAudioStream(pulse->audioStream);
        }
    }
    else if (addr == 0xFF1A && NR52.audioOnOff) {
        SC3Wave::NR30.raw = data;
        if (!SC3Wave::NR30.DACOnOff) {
            NR52.ch3On = 0;
            StopAudioStream(SC3Wave::audioStream);
        }
    }
    else if (addr == 0xFF1B) {
        if (NR52.audioOnOff) SC3Wave::NR31 = data;
        else SC3Wave::NR31 = data & 0x3F;
    }
    else if (addr == 0xFF1C && NR52.audioOnOff) SC3Wave::NR32.raw = data;
    else if (addr == 0xFF1D && NR52.audioOnOff) SC3Wave::NR33 = data;
    else if (addr == 0xFF1E && NR52.audioOnOff) {
        SC3Wave::NR34.raw = data;
        if (data >> 7 && SC3Wave::NR30.DACOnOff) {
            NR52.ch3On = 1;
            PlayAudioStream(SC3Wave::audioStream);
        }
    }
    else if (addr == 0xFF20) {
        if (NR52.audioOnOff) SC4Noise::NR41.raw = data;
        else SC4Noise::NR41.raw = data & 0x3F;
    }
    else if (addr == 0xFF21 && NR52.audioOnOff) {
        SC4Noise::NR42.raw = data;
        if ((SC4Noise::NR42.raw & 0xF8) != 0x00) SC4Noise::DAC = true;
        else { SC4Noise::DAC = false; NR52.ch4On = 0; StopAudioStream(noise->audioStream); }
    }
    else if (addr == 0xFF22 && NR52.audioOnOff) SC4Noise::NR43.raw = data;
    else if (addr == 0xFF23 && NR52.audioOnOff) {
        SC4Noise::NR44.raw = data;
        if (data >> 7 && SC4Noise::DAC) {
            NR52.ch4On = 1;
            PlayAudioStream(noise->audioStream);
        }
    }
    else if (addr == 0xFF24 && NR52.audioOnOff) NR50.raw = data;
    else if (addr == 0xFF25 && NR52.audioOnOff) NR51.raw = data;
    else if (addr == 0xFF26) {
        if (!(data >> 7)) resetRegisters();
        NR52.raw = data & 0xF0;
    }
    else if (addr >= 0xFF30 && addr <= 0xFF3F) SC3Wave::waveRAM[addr - 0xFF30] = data;
}

uint8_t Apu::read(uint16_t addr) {
    uint8_t data = 0xFF;

    if (addr == 0xFF10)      data = SC1PulseSweep::NR10.raw | 0x80;
    else if (addr == 0xFF11) data = SC1PulseSweep::NR11.raw | 0x3F;
    else if (addr == 0xFF12) data = SC1PulseSweep::NR12.raw;
    else if (addr == 0xFF14) data = SC1PulseSweep::NR14.raw | 0xBF;
    else if (addr == 0xFF16) data = SC2Pulse::NR21.raw | 0x3F;
    else if (addr == 0xFF17) data = SC2Pulse::NR22.raw;
    else if (addr == 0xFF19) data = SC2Pulse::NR24.raw | 0xBF;
    else if (addr == 0xFF1A) data = SC3Wave::NR30.raw | 0x7F;
    else if (addr == 0xFF1C) data = SC3Wave::NR32.raw | 0x9F;
    else if (addr == 0xFF1E) data = SC3Wave::NR34.raw | 0xBF;
    else if (addr == 0xFF21) data = SC4Noise::NR42.raw;
    else if (addr == 0xFF22) data = SC4Noise::NR43.raw;
    else if (addr == 0xFF23) data = SC4Noise::NR44.raw | 0xBF;
    else if (addr == 0xFF24) data = NR50.raw;
    else if (addr == 0xFF25) data = NR51.raw;
    else if (addr == 0xFF26) data = NR52.raw | 0x70;
    else if (addr >= 0xFF30 && addr <= 0xFF3F) data = SC3Wave::waveRAM[addr - 0xFF30];

    //logger->log(Logger::DEBUG, "Reading %X from %X", data, addr);

    return data;
}

void Apu::tick() {
    rate++;

    if ((rate % 2) == 0) { // Execute every 2 ticks
        if (SC1PulseSweep::NR14.lengthEnable) {
            uint8_t oldLengthTimer = SC1PulseSweep::NR11.initialLengthTimer;
            if (SC1PulseSweep::NR11.initialLengthTimer < 63) SC1PulseSweep::NR11.initialLengthTimer++;
            if (oldLengthTimer + 1 == 64) {
                NR52.ch1On = 0;
                StopAudioStream(pulseSweep->audioStream);
            }
        }
        if (SC2Pulse::NR24.lengthEnable) {
            uint8_t oldLengthTimer = SC2Pulse::NR21.initialLengthTimer;
            if (SC2Pulse::NR21.initialLengthTimer < 63) SC2Pulse::NR21.initialLengthTimer++;
            if (oldLengthTimer + 1 == 64) {
                NR52.ch2On = 0;
                StopAudioStream(pulse->audioStream);
            }
        }
        if (SC3Wave::NR34.lengthEnable) {
            uint16_t oldLengthTimer = SC3Wave::NR31;
            if (oldLengthTimer + 1 == 256) {
                NR52.ch3On = 0;
                StopAudioStream(SC3Wave::audioStream);
            }
        }
        if (SC4Noise::NR44.lengthEnable) {
            uint8_t oldLengthTimer = SC4Noise::NR41.initialLengthTimer;
            SC4Noise::NR41.initialLengthTimer++;
            if (oldLengthTimer + 1 == 64) {
                NR52.ch4On = 0;
                StopAudioStream(noise->audioStream);
            }
        }
    }

    if ((rate % 4) == 0) {
        if (SC1PulseSweep::NR10.pace != 0 && NR52.ch1On) {
            SC1PulseSweep::sweepTick++;
            if (SC1PulseSweep::sweepDone) {
                SC1PulseSweep::currentSweepPace = SC1PulseSweep::NR10.pace;
                SC1PulseSweep::sweepDone = false;
            }
            if (SC1PulseSweep::sweepTick == SC1PulseSweep::currentSweepPace) {
                SC1PulseSweep::sweepTick = 0;

                uint16_t currentPeriod = ((SC1PulseSweep::NR14.periodHigh << 8) | SC1PulseSweep::NR13);
                uint16_t resultingPeriod;

                if (SC1PulseSweep::NR10.direction) resultingPeriod = currentPeriod - (currentPeriod >> SC1PulseSweep::NR10.individualStep);
                else resultingPeriod = currentPeriod + (currentPeriod >> SC1PulseSweep::NR10.individualStep);

                if (resultingPeriod > 0x7FF || currentPeriod > resultingPeriod) {
                    NR52.ch1On = 0;
                    StopAudioStream(pulseSweep->audioStream);
                } else {
                    SC1PulseSweep::NR13 = resultingPeriod & 0x00FF;
                    SC1PulseSweep::NR14.periodHigh = resultingPeriod >> 8;
                }

                SC1PulseSweep::sweepDone = true;
            }
        } else {
            SC1PulseSweep::sweepDone = true;
        }
    }

    if ((rate % 8) == 0) {
        SC1PulseSweep::enveloppeTick++;
        if (SC1PulseSweep::NR12Temp.sweepPace != 0x0 && NR52.ch1On) {
            if (SC1PulseSweep::enveloppeTick % SC1PulseSweep::NR12Temp.sweepPace == 0) {
                if (SC1PulseSweep::NR12Temp.nvDirection) SC1PulseSweep::currentVolume++;
                else SC1PulseSweep::currentVolume--;
                if (SC1PulseSweep::currentVolume >= 0xF) SC1PulseSweep::currentVolume = 0xF;
                else if (SC1PulseSweep::currentVolume <= 0) SC1PulseSweep::currentVolume = 0;
                SetAudioStreamVolume(pulseSweep->audioStream, SC1PulseSweep::currentVolume * (1.0f / 15.0f));
            }
        }

        SC2Pulse::enveloppeTick++;
        if (SC2Pulse::NR22Temp.sweepPace != 0x0 && NR52.ch2On) {
            if (SC2Pulse::enveloppeTick % SC2Pulse::NR22Temp.sweepPace == 0) {
                if (SC2Pulse::NR22Temp.nvDirection) SC2Pulse::currentVolume++;
                else SC2Pulse::currentVolume--;
                if (SC2Pulse::currentVolume >= 0xF) SC2Pulse::currentVolume = 0xF;
                else if (SC2Pulse::currentVolume <= 0) SC2Pulse::currentVolume = 0;
                SetAudioStreamVolume(pulse->audioStream, SC2Pulse::currentVolume * (1.0f / 15.0f));
            }
        }
    }

    if (rate == 8) {
        // Do things
        rate = 0;
    }
}

void Apu::resetRegisters() {
    SC1PulseSweep::NR10.raw = 0x00;
    SC1PulseSweep::NR11.raw &= 0x3F;
    SC1PulseSweep::NR12.raw = 0x00;
    SC1PulseSweep::NR13 = 0x00;
    SC1PulseSweep::NR14.raw = 0x00;
    SC2Pulse::NR21.raw &= 0x3F;
    SC2Pulse::NR22.raw = 0x00;
    SC2Pulse::NR23 = 0x00;
    SC2Pulse::NR24.raw = 0x00;
    SC3Wave::NR30.raw = 0x00;
    SC3Wave::NR32.raw = 0x00;
    SC3Wave::NR33 = 0x00;
    SC3Wave::NR34.raw = 0x00;
    SC4Noise::NR42.raw = 0x00;
    SC4Noise::NR43.raw = 0x00;
    SC4Noise::NR44.raw = 0x00;
    NR50.raw = 0x00;
    NR51.raw = 0x00;
    NR52.raw = 0x00;
}

void Apu::closeConnection() {
    CloseAudioDevice();
}
