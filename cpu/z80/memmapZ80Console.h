#pragma once

#include "../../memmap.h"
#include "../../config.h"
#include "../../io/ioEvent.h"
#include<memory>

class vdpMS;
class TiPsg;
//class apuMS;

class memmapZ80Console: public memmap {
public:
    memmapZ80Console(std::shared_ptr<config> conf, std::shared_ptr<vdp> v, std::shared_ptr<TiPsg> a);
    void writeByte(uint32_t addr, uint8_t val) override;
    void writeWord(uint32_t addr, uint16_t val) override;
    void writeLong(uint32_t addr, uint32_t val) override;
    uint8_t& readByte(uint32_t addr) override;
    uint16_t& readWord(uint32_t addr) override;
    uint32_t& readLong(uint32_t addr) override;
    void sendEvent(ioEvent e) override;

    uint8_t readPortByte(uint8_t port, uint64_t cycle);
    void writePortByte(uint8_t port, uint8_t val, uint64_t cycle);

private:
    uint8_t& map(uint32_t addr) override;
    std::shared_ptr<vdp> vdp_dev;
    std::shared_ptr<TiPsg> apu_dev;
    std::shared_ptr<config> cfg;

    uint8_t dummyByte;
    uint16_t dummyWord;
    uint32_t dummyLong;

    bool valid;

    std::array<uint8_t, 0x2000> ram; // 8KB of RAM, mapped to 0xC000-0xDFFF and mirrored to 0xE000-0xFFFB
    std::array<uint8_t, 0x10'0000> rom; // ROM is only visible in 3 chunks of 16KB each, with most games controlled by a Sega family of mappers. Largest GG/SMS/SG-1000 ROMs are 1MB in size.
    std::size_t romsize;
    std::array<uint8_t, 0x8000> cartRam;

    bool slot2RamActive;
    uint8_t slot2RamPage;

    uint8_t map_ctrl, map_slot0, map_slot1, map_slot2;
    uint32_t map_slot0_offset, map_slot1_offset, map_slot2_offset;


    // Port 3E: Memory control
    struct mem_ctrl_t {
        unsigned unused:2;
        bool io_chip_disabled;
        bool bios_rom_disabled;
        bool wram_disabled;
        bool card_slot_disabled;
        bool cart_slot_disabled;
        bool exp_slot_disabled;
    } mem_ctrl;

    enum class io_dir_t {
        output,
        input
    };

    enum class io_level_t {
        low,
        high
    };

    // Port 3F: I/O Port control
    struct io_port_ctrl_t {
        io_dir_t port_a_tr_dir;
        io_dir_t port_a_th_dir;
        io_dir_t port_b_tr_dir;
        io_dir_t port_b_th_dir;
        io_level_t port_a_tr_lev;
        io_level_t port_a_th_lev;
        io_level_t port_b_tr_lev;
        io_level_t port_b_th_lev;
    } io_port_ctrl;

    enum joya_buttons {
        p1_up = 1<<0,
        p1_down = 1<<1,
        p1_left = 1<<2,
        p1_right = 1<<3,
        p1_b1 = 1<<4,
        p1_b2 = 1<<5,
        p2_up = 1<<6,
        p2_down = 1<<7,
    };

    enum joyb_buttons {
        p2_left = 1<<0,
        p2_right = 1<<1,
        p2_b1 = 1<<2,
        p2_b2 = 1<<3,
        reset = 1<<4,
        //bit 5 unused
        p1_b3 = 1<<6,
        p2_b3 = 1<<7
    };

    struct io_port_ab_t {
        bool port_a_up;
        bool port_a_down;
        bool port_a_left;
        bool port_a_right;
        bool port_a_tl;
        bool port_a_tr;
        bool port_b_up;
        bool port_b_down;
    } io_port_ab;

    struct io_port_b_misc_t {
        bool port_b_left;
        bool port_b_right;
        bool port_b_tl;
        bool port_b_tr;
        bool reset;
        bool unused; // always returns 1
        bool port_a_th;
        bool port_b_th;
    } io_port_b_misc;

    struct gg_port_0_t {
        bool start;
    } gg_port_0;
};
