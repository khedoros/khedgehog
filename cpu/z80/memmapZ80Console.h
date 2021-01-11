#pragma once

#include "../../memmap.h"
#include "../../config.h"
#include "../../io/ioEvent.h"
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
    void sendEvent(ioEvent e) override;

    uint8_t readPortByte(uint8_t port, uint64_t cycle);
    void writePortByte(uint8_t port, uint8_t val, uint64_t cycle);

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
    std::size_t romsize;
    std::array<uint8_t, 0x8000> cart_ram;

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

    enum class button_state_t {
        pressed,
        open
    };

    struct io_port_ab_t {
        button_state_t port_a_up;
        button_state_t port_a_down;
        button_state_t port_a_left;
        button_state_t port_a_right;
        button_state_t port_a_tl;
        button_state_t port_a_tr;
        button_state_t port_b_up;
        button_state_t port_b_down;
    } io_port_ab;

    struct io_port_b_misc_t {
        button_state_t port_b_left;
        button_state_t port_b_right;
        button_state_t port_b_tl;
        button_state_t port_b_tr;
        button_state_t reset;
        unsigned unused:1; // always returns 1
        button_state_t port_a_th;
        button_state_t port_b_th;
    } io_port_b_misc;
};
