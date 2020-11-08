#pragma once

#include<cstdint>

class memmap {
public:
    virtual uint8_t& readByte(uint32_t addr) = 0;
    virtual uint16_t& readWord(uint32_t addr) = 0;
    virtual uint32_t& readLong(uint32_t addr) = 0;
    virtual void writeByte(uint32_t addr, uint8_t val) = 0;
    virtual void writeWord(uint32_t addr, uint16_t val) = 0;
    virtual void writeLong(uint32_t addr, uint32_t val) = 0;
protected:
    virtual uint8_t& map(uint32_t addr) = 0;
};
