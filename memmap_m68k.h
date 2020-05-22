#pragma once

#include<cstdint>

class memmap_m68k {
public:
    memmap_m68k();
    uint8_t readByte(uint32_t addr);
    uint16_t readWord(uint32_t addr);
    uint32_t readLong(uint32_t addr);
    void writeByte(uint32_t addr, uint8_t val);
    void writeWord(uint32_t addr, uint16_t val);
    void writeLong(uint32_t addr, uint32_t val);
private:

};
