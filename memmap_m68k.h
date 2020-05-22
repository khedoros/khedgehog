#pragma once

#include<cstdint>
#include<array>
#include<memory>

class memmap_m68k {
public:
    memmap_m68k(std::shared_ptr<config> config);
    uint8_t readByte(uint32_t addr);
    uint16_t readWord(uint32_t addr);
    uint32_t readLong(uint32_t addr);
    void writeByte(uint32_t addr, uint8_t val);
    void writeWord(uint32_t addr, uint16_t val);
    void writeLong(uint32_t addr, uint32_t val);
private:
    std::array<uint8_t, 0x400000> rom;
    std::array<uint8_t, 0x10000>  ram;
    std::shared_ptr<memmap_apu>   apu;
    std::shared_ptr<memmap_vdp>   vdp;
    std::shared_ptr<input_mapper> io;
};
