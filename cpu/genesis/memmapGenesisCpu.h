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
    uint8_t& readByte(uint32_t addr) override;
    uint16_t& readWord(uint32_t addr) override;
    uint32_t& readLong(uint32_t addr) override;
    void writeByte(uint32_t addr, uint8_t val) override;
    void writeWord(uint32_t addr, uint16_t val) override;
    void writeLong(uint32_t addr, uint32_t val) override;
    uint32_t dummyVal = 0xdeadbeef;

protected:
    uint8_t& map(uint32_t addr) override;

private:
    std::array<uint8_t, 0x400000> rom;
    std::array<uint8_t, 0x10000>  ram;
    std::shared_ptr<memmapGenesisApu>   apu;
    std::shared_ptr<memmapGenesisVdp>   vdp;
    std::shared_ptr<inputMapperGenesis> io;
    bool valid;
};
