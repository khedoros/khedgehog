#pragma once

#include "../vdp.h"
#include "../../config.h"
#include<cstdint>
#include<array>
#include<vector>

class vdpMS: public vdp {
public:
    vdpMS(systemType t, systemRegion r);
    void writeByte(uint8_t address, uint8_t val, uint64_t cycle) override;
    uint8_t readByte(uint8_t address, uint64_t cycle) override;
    std::vector<std::vector<uint8_t>> getPartialRender() override; // Render a composited view of the current VDP memory state
    std::vector<std::vector<uint8_t>> getDebugRender() override; // Render VDP memory in a rawer format
    std::vector<std::vector<uint8_t>>& getFrameBuffer() override; // Get current framebuffer state (e.g. after completing the frame)
    uint16_t name_tab_base();
    uint16_t col_tab_base();
    uint16_t bg_tile_base();
    uint16_t sprite_attr_tab_base();
    uint16_t sprite_tile_base();
    uint64_t calc(uint64_t) override;
    bool lineInterrupt() override;
    bool frameInterrupt() override;
    void endLine(uint64_t lineNum) override;

private:
    void writeAddress(uint8_t val);
    void writeData(uint8_t val);
    uint8_t readData();
    uint8_t readStatus(uint64_t cycle);
    uint8_t readVCounter(uint64_t cycle);
    uint8_t readHCounter(uint64_t cycle);

    enum class graphicsMode_t {text, graphics1, graphics2, multicolor, mode4, mode4_224, mode4_240, unknown};
    graphicsMode_t curMode;

    graphicsMode_t getMode();
    void setPixelSG(std::vector<std::vector<uint8_t>>& buffer, int x, int y, int index);
    void setPixelGG(std::vector<std::vector<uint8_t>>& buffer, int x, int y, int index);
    void setPixelSMS(std::vector<std::vector<uint8_t>>& buffer, int x, int y, int index);
    void renderGraphic1(unsigned int line, std::vector<std::vector<uint8_t>>&);
    void renderGraphic2(unsigned int line, std::vector<std::vector<uint8_t>>&);
    void renderText(unsigned int line, std::vector<std::vector<uint8_t>>&);
    void renderMulticolor(unsigned int line, std::vector<std::vector<uint8_t>>&);
    void renderMode4(unsigned int line, std::vector<std::vector<uint8_t>>&);
	void renderLine(unsigned int line, std::vector<std::vector<uint8_t>>& renderBuffer);
    std::vector<std::vector<uint8_t>> getDBG2Render();
    std::vector<std::vector<uint8_t>> getDBM4Render();
    std::array<uint8_t, 8> getG2TileLine(uint16_t tileAddr, uint8_t row);
    std::array<uint8_t, 8> getM4TileLine(uint16_t tileAddr, uint8_t row);
    void resizeBuffer(unsigned int x, unsigned int y);

    std::array<uint8_t, 0x4000> vram;
    std::vector<uint8_t> pal_ram;

    systemType vdpMode;
    systemRegion vdpRegion;
    uint8_t count = 0;
    unsigned int curXRes;
    unsigned int curYRes;
    std::vector<std::vector<uint8_t>> buffer;

    bool addr_latch;
    uint8_t addr_buffer;
    uint8_t data_buffer;
    unsigned int address:14;
    enum class addr_mode_t {vram_read, vram_write, reg_write, cram_write} addr_mode;

    // Reference: https://www.smspower.org/Development/VDPRegisters
    // Control Registers
    struct ctrl_1_t { //Register #0: Mode Control #1
        union {
            struct {
                unsigned sync_enable:1; //enable/disable external VDP input, for overlay. Not used.
                unsigned mode_2:1; // M3 (mode bit 3) called M2 in Coleco docs
                unsigned mode_4:1; // not in TMS9928a, guess it's for SMS VDP?
                unsigned shift_sprites:1; // ditto for the remaining items
                unsigned line_interrupts:1;
                unsigned hide_left:1;
                unsigned h_scroll_lock:1;
                unsigned v_scroll_lock:1;
            } fields;
            uint8_t val;
        };
    } ctrl_1;

    struct ctrl_2_t { //Register #1: Mode Control #2
        union {
            struct {
                unsigned doubled_sprites:1; //stretched (1 tile doubled to 16x16?)             Mode table M1 M2 M3
                unsigned large_sprites:1; //tiled (16x16, made of 4 tiles?)                                0  0  0   Graphics I mode
                unsigned unused:1; // reserved                                                             0  1  0   Graphics II mode
                unsigned mode_3:1; //                                                                      0  0  1   Multicolor mode
                unsigned mode_1:1; //                                                                      1  0  0   Text mode
                unsigned frame_interrupts:1; // Enable/Disable NMI interrupts at end of active raster
                unsigned enable_display:1; // Blanking (==0) shows the border color
                unsigned unused_2:1; // Selects amount of RAM apparently. I'd guess it's either 1 or ignored in Sega hardware.
            } fields;
            uint8_t val;
        };
    } ctrl_2;

    uint8_t nt_base:4;       //Register #2: Name table base address, used as top 3 (SMS) to 4 (SG-1000) bits of the name table base
    uint8_t color_t_base;    // Register #3: Color table base address, used as top 8 bits of the color table base
	uint8_t pt_base:3;       // Register #4: Pattern generator table start address, used as top 3 bits of pattern (bg tile) table
    uint8_t spr_attr_base:7; // Register #5: Sprite attribute table base, top 7 bits of sprite attribute table
    uint8_t spr_tile_base:3; // Register #6: Sprite generator table start address, used as top 3 bits of sprite tile table

    struct bg_fg_col_t { // Register #7: Foreground and background colors, high nibble has foreground, low nibble has background.
        union {
            struct {
                unsigned background:4; // background in all modes, color 0 in text mode
                unsigned foreground:4; // color 1 in text mode
            } fields;
            uint8_t val;
        };
    } bg_fg_col;

    uint8_t bg_x_scroll; // Register #8: 8-bit Horizontal scroll value
    uint8_t bg_y_scroll; // Register #9: 8-bit Vertical scroll value
    uint8_t line_interrupt; // Register #A: 8-bit indicator of line to produce interrupt on (reset value for line counter)
    uint8_t line_int_cur; // Current value of line interrupt counter
    bool line_int_active;
    bool scr_int_active;
    uint64_t curLine;

    // Registers #B-#F exist, but have no effect in the SG-1000 or SMS, apparently.

    struct status_t { // Status register
        union {
            struct {
                unsigned sprite_num: 5; // Number of the 5th sprite on a line
                unsigned collision_flag:1; // Flag indicating collision of 2 sprites
                unsigned overflow_flag:1; // Flag indicating 5+ sprites on a line
                unsigned vblank_flag:1; //Flag set at end of raster/start of vblank
            } fields;
            uint8_t val;
        };
    } status;

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
            } fields;
            struct {
                uint8_t byte1;
                uint8_t byte2;
            } bytes;
            uint16_t val;
        };
    };

    struct sprite_info_t {
        union {
            struct {
                uint8_t x;
                uint8_t y;
                uint8_t tile;
                unsigned color:4;
                unsigned unused:3;
                unsigned left_shift:1; //by 32 pixels
            } fields;
            uint8_t val[4];
        };
    } sprite_info;


    // Mode Graphics I: M1=0, M2=0, M3=0
    // 32x24 background of 256 8x8 tiles, 1-bit of color defined in color table. High-order bits form the right of the tile.
    // Colors: 32 entries, bg_fg_col_t format, where each byte defines the colors for 8 consecutive tile numbers.
    //
    // Mode Graphics II: M1=0, M2=0, M3=1
    // 32x24 background divided into 32x8 strips, each of which have 256 8x8 tiles (768 tiles total).
    // Colors: 6144 entries, one for each byte of the tiles.
    //
    // Mode Multicolor: M1=0, M2=1, M3=0                                                                                                                        AB
    // 64x48 grid of pixels, 768 name entries (i.e. 32x24 tilemap).                                                                                             CD
    // Colors: 1 byte defines 2 macropixels next to each other. The next 7 bytes define the 7 pairs underneath them, so an 8-byte block defines a tall column.  EF
    // Colors are addressed in these 8-byte blocks, like the table to the right.                                                                                GH
    //                                                                                                                                                          IJ
    // Mode Text: M1=1, M2=0, M3=0                                                                                                                              KL
    // 40x24 text positions, 6x8 characters. 256 patterns available. Sprites are disabled. Least-significant 2 bits of the 8-byte tile are ignored.             MN
    // Colors: defined in register 7.                                                                                                                           OP
    //
    // Sprite table:
    // 128 bytes, with 4-byte structs matching sprite_info_t
    // 0xD0 in vertical position field means end of table      AC
    // 2x2 sprites use the quadrant definition to the right:   BD

    // Tile format for SMS:
    //   first byte has bit 0's of first row.
    //   second byte has bit 1's of first row.
    //   fifth byte has bit 0's of second row, etc
    //   So each 8x8 tile is 32 bytes, up to 448 tiles defined, in most cases

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

    struct sms_color_t {
        union {
            struct {
                unsigned red:2;
                unsigned green:2;
                unsigned blue:2;
                unsigned unused:2;
            } component;
            uint8_t val;
        };
    };

    struct gg_color_t {
        union {
            struct {
                unsigned red:4;
                unsigned green:4;
                unsigned blue:4;
                unsigned unused:4;
            } component;
            uint8_t val[2];
        };
    };
};
