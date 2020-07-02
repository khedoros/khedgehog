#pragma once

#include<cstdint>
#include<array>
#include<memory>

#include "../m68k/memmapM68k.h"
#include "../../config.h"
#include "../../apu/genesis/memmapGenesisApu.h"
#include "../../vdp/genesis/memmapGenesisVdp.h"
#include "../../io/genesis/inputMapperGenesis.h"

class memmapGenesisCpu : public memmapM68k {
public:
    memmapGenesisCpu(std::shared_ptr<config> cfg);
    uint8_t& readByte(uint32_t addr);
    uint16_t& readWord(uint32_t addr);
    uint32_t& readLong(uint32_t addr);
    void writeByte(uint32_t addr, uint8_t val);
    void writeWord(uint32_t addr, uint16_t val);
    void writeLong(uint32_t addr, uint32_t val);
    uint8_t dummyByte;
    uint16_t dummyWord;
    uint32_t dummyLong;
private:
    std::array<uint8_t, 0x400000> rom;
    std::array<uint8_t, 0x10000>  ram;
    std::shared_ptr<memmapGenesisApu>   apu;
    std::shared_ptr<memmapGenesisVdp>   vdp;
    std::shared_ptr<inputMapperGenesis> io;
    bool valid;
};
