#pragma once

#include "../vdp.h"
#include<cstdint>
#include<array>

class vdpMS: public vdp {
public:
    vdpMS();
    void writeByte(uint8_t address, uint8_t val) override;
    uint8_t readByte(uint8_t address) override;
    std::vector<std::vector<uint8_t>> getPartialRender() override; // Render a composited view of the current VDP memory state
    std::vector<std::vector<uint8_t>> getSpritePartialRender() override; // Render the sprite layer(s) of the current VDP memory state
    std::vector<std::vector<uint8_t>> getBgPartialRender() override; // Redner the background layer(s) of the current VDP memory state
    uint64_t calc(uint64_t) override;

private:
    void writeAddress(uint8_t val);
    void writeData(uint8_t val);
    uint8_t readData();
    uint8_t readStatus();
    std::array<uint8_t, 0x4000> vram;
    std::array<uint8_t, 0x20> pal_ram;

    bool addr_latch;
    uint8_t addr_buffer;
    uint8_t data_buffer;
    unsigned int address:14;
    unsigned int pal_address:5;
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
        unsigned doubled_sprites:1; //stretched (1 tile doubled to 16x16?)
        unsigned large_sprites:1; //tiled (16x16, made of 4 tiles?)
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

    // Each tile in the background table is stored as 2 bytes, in a 32x28x2 = 1792-byte table.
    struct tile_info_t {
        union {
            struct {
                unsigned tile_num:9;
                unsigned hflip:1;
                unsigned vflip:1;
                unsigned palnum:1;
                unsigned priority:1;
                unsigned unused:3;
            };
            struct {
                uint8_t byte1;
                uint8_t byte2;
            };
            uint16_t tile;
        };
    };

    // Tile format:
    //   first byte has bit 0's of first row.
    //   second byte has bit 1's of first row.
    //   fifth byte has bit 0's of second row, etc
    //   So each 8x8 tile is 32 bytes, up to 448 tiles defined, in most cases

    uint8_t color_base; //Register #3: Color table base address

    uint8_t bg_pattern_base; //Register #4: bg tiles base address

    uint8_t sprite_table_base; // Register #5: Sprite metadata table base address

    static constexpr std::array<uint8_t, 16 * 3> tms_palette { // The set TMS9918 palette
        0x00, 0x00, 0x00,   0x00, 0x00, 0x00,   0x21, 0xc8, 0x42,   0x5e, 0xdc, 0x78,
        0x54, 0x55, 0xed,   0x7d, 0x76, 0xfc,   0xd4, 0x52, 0x4d,   0x42, 0xeb, 0xf5,
        0xfc, 0x55, 0x54,   0xff, 0x79, 0x78,   0xd4, 0xc1, 0x54,   0xe6, 0xce, 0x80,
        0x21, 0xb0, 0x3b,   0xc9, 0x5b, 0xba,   0xcc, 0xcc, 0xcc,   0xff, 0xff, 0xff
    };

    static constexpr std::array<uint8_t, 4> sms_pal_component { 0x00, 0x55, 0xaa, 0xff }; // 2-bit SMS RGB color components
    static constexpr std::array<uint8_t, 3> sms_pal_mask { 0x30, 0x0c, 0x03 }; // masks for the color components
    static constexpr std::array<uint8_t, 3> sms_pal_shift { 0x04, 0x02, 0x00 }; // bit shifts for the color components
    static constexpr std::array<uint8_t, 16> sms_tms_pal_index {               // SMS equivs to TMS9918 pal entries
        0x00, 0x00, 0x08, 0x0c, 0x10, 0x30, 0x01, 0x3c, 0x02, 0x03, 0x05, 0x0f, 0x04, 0x33, 0x15, 0x3f
    };

    static constexpr std::array<uint8_t, 16> gg_pal_component {
        0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff
    };
    static constexpr std::array<uint8_t, 3> gg_pal_mask { 0x0f, 0xf0, 0x0f };
    static constexpr std::array<uint8_t, 3> gg_pal_shift { 0, 4, 0 };
};
