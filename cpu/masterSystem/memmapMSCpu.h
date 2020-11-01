#pragma once
#include "../z80/memmapZ80.h"
#include "../../apu/masterSystem/memmapMSApu.h"
#include "../../vdp/masterSystem/memmapMSVdp.h"
#include "../../io/masterSystem/inputMapperMS.h"
#include "../../config.h"
#include<memory>

class memmapMSCpu: public memmapZ80 {
public:
    memmapMSCpu(std::shared_ptr<config> cfg);
    uint8_t& readByte(uint32_t addr) override;
    uint16_t& readWord(uint32_t addr) override;
    uint32_t& readLong(uint32_t addr) override;
    void writeByte(uint32_t addr, uint8_t val) override;
    void writeWord(uint32_t addr, uint16_t val) override;
    void writeLong(uint32_t addr, uint32_t val) override;
    uint32_t dummyVal = 0xdeadbeef;

protected:
    virtual uint8_t& map(uint32_t addr) override;

private:
    std::array<uint8_t, 0xc000> rom;
    std::array<uint8_t, 0x2000>  ram;
    std::shared_ptr<memmapMSApu>   apu;
    std::shared_ptr<memmapMSVdp>   vdp;
    std::shared_ptr<inputMapperMS> io;
    bool valid;
/*
$0000-$03ff	ROM (unpaged)
$0400-$3fff	ROM mapper slot 0
$4000-$7fff	ROM mapper slot 1
$8000-$bfff	ROM/RAM mapper slot 2
$c000-$dfff	System RAM
$e000-$ffff	System RAM (mirror)
$fff8	3D glasses control
$fff9-$fffb	3D glasses control (mirrors)
$fffc	Cartridge RAM mapper control
$fffd	Mapper slot 0 control
$fffe	Mapper slot 1 control
$ffff	Mapper slot 2 control
*/
};
