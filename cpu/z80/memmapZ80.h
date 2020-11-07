#pragma once

#include "../memmapCpu.h"
#include<memory>

class memmapZ80: public memmapCpu {
    //uint8_t& map(uint32_t addr) override;
    uint8_t& readByte(uint32_t addr) override;
    uint16_t& readWord(uint32_t addr) override;
    uint32_t& readLong(uint32_t addr) override;
    void writeByte(uint32_t addr, uint8_t val) override;
    void writeWord(uint32_t addr, uint16_t val) override;
    void writeLong(uint32_t addr, uint32_t val) override;

    uint8_t dummy8;
    uint16_t dummy16;
    uint32_t dummy32;
};
