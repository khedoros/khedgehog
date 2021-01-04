#include<iostream>
#include<cassert>

#include "vdpMS.h"
#include "../../util.h"

vdpMS::vdpMS(systemType t, systemRegion r):addr_latch(false), vdpMode(t), vdpRegion(r) {

}

std::vector<std::vector<uint8_t>> vdpMS::getPartialRender() {
    int mode = 8 * ctrl_1.fields.mode_4 + 
               4 * ctrl_2.fields.mode_3 + 
               2 * ctrl_1.fields.mode_2 + 
                   ctrl_2.fields.mode_1;
    std::vector<std::vector<uint8_t>> buffer(192, std::vector<uint8_t>(256*3, 0));
    if(!ctrl_2.fields.enable_display) return buffer;
    switch(mode) {
        case 0:
            renderGraphic1(buffer);
            break;
        case 1:
            renderText(buffer);
            break;
        case 2:
            renderGraphic2(buffer);
            break;
        case 4:
            renderMulticolor(buffer);
            break;
        case 8:
            renderMode4(buffer);
            break;
        default:
            std::cerr<<"Unsupported rendering mode "<<mode<<"\n";
    }

    // SG-1000 modes:
    // 0: Graphic I                   -- Standard (Mode 0)
    // 1: Text I                      -- Standard (Mode 1)
    // 2: Graphic II                  -- Standard (Mode 2)
    // 3: Mode 1+2
    // 4: Multicolor                  -- Standard (Mode 3)
    // 5: Mode 1+3
    // 6: Mode 2+3
    // 7: Mode 1+2+3
    // Only applicable to SMS:
    // 8: Mode 4 (SMS mode)           -- Standard (Mode 4)
    // 9: Invalid text mode
    // A: Mode 4
    // B: Invalid Text Mode (VDP1) / Mode 4 (224-line display in VDP2)
    // C: Mode 4
    // D: Invalid text mode
    // E: Mode 4 (VDP1) / Mode 4 (240-line display in VDP2)
    // F: Invalid Text Mode (VDP1) / Mode 4 (VDP2)
    return buffer;
}

std::vector<std::vector<uint8_t>> vdpMS::getSpritePartialRender() {
    return std::vector<std::vector<uint8_t>>(192, std::vector<uint8_t>(256*3, 0));
}

std::vector<std::vector<uint8_t>> vdpMS::getBgPartialRender() {
    return std::vector<std::vector<uint8_t>>(192, std::vector<uint8_t>(256*3, 0));
}

void vdpMS::renderGraphic1(std::vector<std::vector<uint8_t>>& buffer) {
    std::cout<<"Graphic I (Mode 0)  render\n";
}

void vdpMS::renderGraphic2(std::vector<std::vector<uint8_t>>& buffer) {
    std::cout<<"Graphic II (Mode 2) render\n";
    for(int y_tile=0;y_tile<24;y_tile++) {
        int y_triad = y_tile / 8;
        for(int x_tile=0;x_tile<32;x_tile++) {
            int tile_num_addr = nt_base.fields.base * 0x800 + y_tile * 32 + x_tile;
            int tile_num = vram.at(tile_num_addr) + 256 * y_triad;
            int tile_addr = pt_base.fields.base * 0x800 + tile_num * 8;

            int color_addr = (color_t_base * 0x40 + tile_num * 8) & 0x3fff ;
            //std::cout<<"Color base: "<<std::hex<<static_cast<unsigned int>(color_t_base)<<" x_tile: "<<x_tile<<" y_tile: "<<y_tile<<" tile number: "<<tile_num<<"\n";

            for(int y = 0; y < 8; y++) {
                bg_fg_col_t colors{.val = vram.at(color_addr + y)};
                uint8_t tile_data = vram.at(tile_addr + y);
                uint8_t mask = 1;
                for(int x = 0; x < 8; x++) {
                    uint8_t color_index = 0;
                    if(tile_data & mask) color_index = colors.fields.foreground;
                    else color_index = colors.fields.background;

                    buffer[y_tile * 8 + y][x_tile * 8 + x * 3 + 0] = tms_palette[color_index * 3 + 0];
                    buffer[y_tile * 8 + y][x_tile * 8 + x * 3 + 1] = tms_palette[color_index * 3 + 1];
                    buffer[y_tile * 8 + y][x_tile * 8 + x * 3 + 2] = tms_palette[color_index * 3 + 2];
                }
            }

        }
    }
}

void vdpMS::renderText(std::vector<std::vector<uint8_t>>& buffer) {
    std::cout<<"Text (Mode 1) render\n";
}

void vdpMS::renderMulticolor(std::vector<std::vector<uint8_t>>& buffer) {
    std::cout<<"MultiColor (Mode 3) render\n";
}

void vdpMS::renderMode4(std::vector<std::vector<uint8_t>>& buffer) {
    std::cout<<"SMS (Mode 4) render\n";
    for(int y=0;y<192;y++) {
        int y_tile = y / 8;
        for(int x=0;x<256;x++) {
            int x_tile = x / 8;

        }
    }
}

uint64_t vdpMS::calc(uint64_t) {
    return 0;
}

void vdpMS::writeByte(uint8_t port, uint8_t val) {
    if(port == 0xbd || port == 0xbf) {
        writeAddress(val);
    }
    else if(port == 0xbe) {
        writeData(val);
    }
}

uint8_t vdpMS::readByte(uint8_t port) {
    if(port == 0xbd || port == 0xbf) {
        return readStatus();
    }
    return 0;
}

void vdpMS::writeAddress(uint8_t val) {
    if(!addr_latch) {
        addr_latch = true;
        addr_buffer = val;
    }
    else {
        addr_latch = false;
        switch(val & 0b11000000) {
        case 0x00: // VRAM read mode
            addr_mode = addr_mode_t::vram_read;
            address = val;
            address <<= 8;
            address += addr_buffer;
            data_buffer = vram[address++];
            dbg_printf(" set read address to %04x", address);
            break;
        case 0x40: // VRAM write mode
            addr_mode = addr_mode_t::vram_write;
            address = val;
            address <<= 8;
            address += addr_buffer;
            dbg_printf(" set write address to %04x", address);
            break;
        case 0x80: // VDP register write mode
            // TODO: Implement register writes
            addr_mode = addr_mode_t::reg_write;
			dbg_printf(" set register %01x to %02x", (val & 0x0f), addr_buffer);
            std::cout<<" set register "<<int(val & 0x0f)<<" to "<<std::hex<<int(addr_buffer)<<"\n";
            switch(val & 0x0f) {
                case 0x00:
                    ctrl_1.val = addr_buffer;
                case 0x01:
                    ctrl_2.val = addr_buffer;
                    break;
                case 0x02:
                    nt_base.val = addr_buffer;
                    break;
                case 0x03:
                    color_t_base = addr_buffer;
                    break;
                case 0x04:
                    pt_base.val = addr_buffer;
                    break;
                case 0x05:
                    spr_attr_base.val = addr_buffer;
                    break;
                case 0x06:
                    spr_tile_base.val = addr_buffer;
                    break;
                case 0x07:
                    bg_fg_col.val = addr_buffer;
                    break;
                case 0x08:
                    bg_x_scroll = addr_buffer;
                    break;
                case 0x09:
                    bg_y_scroll = addr_buffer;
                    break;
                case 0x0a:
                    line_interrupt = addr_buffer;
                    break;
                default:
                    // no effect in SMS or SG-1000 for reg's B-F
                    break;
            }
            break;
        case 0xc0: // CRAM write mode
            addr_mode = addr_mode_t::cram_write;
            pal_address = addr_buffer;
			dbg_printf(" set cram write to address %02x", addr_buffer);
            break;
        }
    }
}

void vdpMS::writeData(uint8_t val) {
	if(addr_mode == addr_mode_t::vram_write || addr_mode == addr_mode_t::vram_read) {
		dbg_printf(" wrote %02x to address %04x", val, address);
		address++;
	}
}

uint8_t vdpMS::readData() {
    return 0;
}

uint8_t vdpMS::readStatus() {
    return 0x80;
}
