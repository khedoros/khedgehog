#pragma once

#include "../vdp.h"
#include<cstdint>
#include<array>

class vdpMS: public vdp {
public:
    vdpMS();
    void writeAddress(uint8_t val);
    void writeData(uint8_t val);
    uint8_t readData();
    uint8_t readStatus();

private:
    std::array<uint8_t, 0x4000> vram;
    std::array<uint8_t, 0x20> pal_ram;

    bool addr_latch;
    uint8_t addr_buffer;
    uint8_t data_buffer;
    unsigned int address:14;
    enum class addr_mode_t {vram_read, vram_write, reg_write, cram_write} addr_mode;

    // Reference: https://www.smspower.org/Development/VDPRegisters
    // Control Registers
    struct mode_1_t { //Register #0: Mode Control #1
        unsigned sync_enable:1;
        unsigned mode_2:1;
        unsigned mode_4:1;
        unsigned shift_sprites:1;
        unsigned line_interrupts:1;
        unsigned hide_left:1;
        unsigned h_scroll_lock:1;
        unsigned v_scroll_lock:1;
    } mode_1;

    struct mode_2_t { //Register #1: Mode Control #2
        unsigned doubled_sprites:1; //stretched
        unsigned large_sprites:1; //tiled
        unsigned unused:1;
        unsigned mode_3:1;
        unsigned mode_1:1;
        unsigned frame_interrupts:1;
        unsigned enable_display:1;
        unsigned unused_2:1;
    } mode_2;

    struct nt_base_t { //Register #2: Name table base address
        unsigned mask_bit:1;
        unsigned base:3;
        unsigned unused:4;
    } nt_base;

    uint8_t color_base; //Register #3: Color table base address

    uint8_t bg_pattern_base; //Register #4: bg tiles base address

    uint8_t sprite_table_base; // Register #5: Sprite metadata table base address


};
