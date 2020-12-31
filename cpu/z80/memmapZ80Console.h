#pragma once

#include "../../memmap.h"
#include "../../config.h"
#include<memory>

class vdpMS;
class apuMS;

class memmapZ80Console: public memmap {
public:
    memmapZ80Console(std::shared_ptr<config> cfg, std::shared_ptr<vdp> v, std::shared_ptr<apu> a);
    void writeByte(uint32_t addr, uint8_t val) override;
    void writeWord(uint32_t addr, uint16_t val) override;
    void writeLong(uint32_t addr, uint32_t val) override;
    uint8_t& readByte(uint32_t addr) override;
    uint16_t& readWord(uint32_t addr) override;
    uint32_t& readLong(uint32_t addr) override;

    uint8_t readPortByte(uint8_t port);
    void writePortByte(uint8_t port, uint8_t val);

private:
    uint8_t& map(uint32_t addr) override;
    std::shared_ptr<vdp> vdp_dev;
    std::shared_ptr<apu> apu_dev;

    uint8_t dummyByte;
    uint16_t dummyWord;
    uint32_t dummyLong;

    bool valid;

    std::array<uint8_t, 0x2000> ram; // 8KB of RAM, mapped to 0xC000-0xDFFF and mirrored to 0xE000-0xFFFB
    std::array<uint8_t, 0x10'0000> rom; // ROM is only visible in 3 chunks of 16KB each, with most games controlled by a Sega family of mappers. Largest GG/SMS/SG-1000 ROMs are 1MB in size.
    std::array<uint8_t, 0x8000> cart_ram;

    uint8_t map_ctrl, map_slot0, map_slot1, map_slot2;

};
