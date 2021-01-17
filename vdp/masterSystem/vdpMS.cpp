#include<iostream>
#include<cassert>

#include "vdpMS.h"
#include "../../util.h"


vdpMS::vdpMS(systemType t, systemRegion r):addr_latch(false), vdpMode(t), vdpRegion(r) {
    if(vdpMode == systemType::gameGear) pal_ram.resize(0x40, 0);
    else                                pal_ram.resize(0x20, 0);

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
		case 8: case 10:
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

void vdpMS::renderGraphic1(std::vector<std::vector<uint8_t>>& buffer) {
    std::cout<<"Graphic I (Mode 0)  render\n";
    for(int y_tile=0;y_tile<24;y_tile++) {
        for(int x_tile=0;x_tile<32;x_tile++) {
            int tile_num_addr = (name_tab_base() + y_tile * 32 + x_tile) & 0x3fff;
            int tile_num = vram.at(tile_num_addr);
            int tile_addr = (bg_tile_base() + tile_num * 8) & 0x3fff;

            int color_addr = (col_tab_base() + tile_num / 8) & 0x3fff;
            bg_fg_col_t colors{.val = vram.at(color_addr)};
            //std::cout<<"Color base: "<<std::hex<<static_cast<unsigned int>(color_t_base)<<" x_tile: "<<x_tile<<" y_tile: "<<y_tile<<" tile number: "<<tile_num<<"\n";

            for(int y = 0; y < 8; y++) {
                uint8_t tile_data = vram.at(tile_addr + y);
                uint8_t mask = 128;
                for(int x = 0; x < 8; x++) {
                    uint8_t color_index = 0;
                    if((tile_data & mask) == mask) color_index = colors.fields.foreground;
                    else color_index = colors.fields.background;

                    buffer[y_tile * 8 + y][3 * (x_tile * 8 + x) + 0] = tms_palette[color_index * 3 + 0];
                    buffer[y_tile * 8 + y][3 * (x_tile * 8 + x) + 1] = tms_palette[color_index * 3 + 1];
                    buffer[y_tile * 8 + y][3 * (x_tile * 8 + x) + 2] = tms_palette[color_index * 3 + 2];

                    mask>>=1;
                }
            }
        }
    }

}

void vdpMS::renderGraphic2(std::vector<std::vector<uint8_t>>& buffer) {
    //std::cout<<"Graphic II (Mode 2) render ntbase: "<<name_tab_base()<<" ttbase: "<<bg_tile_base()<<" ctbase: "<<col_tab_base()<<"\n";
    for(int y_tile=0;y_tile<24;y_tile++) {
        int y_triad = y_tile / 8;
        for(int x_tile=0;x_tile<32;x_tile++) {
            int tile_num_addr = (name_tab_base() + y_tile * 32 + x_tile) & 0x3fff;
			//std::printf("%04x ", tile_num_addr);
            int tile_num = vram.at(tile_num_addr) + 256 * y_triad;
			//std::printf("%03x ", tile_num);
            int tile_addr = ((bg_tile_base() & 0x2000) + tile_num * 8) & 0x3fff;

            int color_addr = ((col_tab_base() & 0x2000) + tile_num * 8) & 0x3fff ;
            //std::cout<<"Color base: "<<std::hex<<static_cast<unsigned int>(color_t_base)<<" x_tile: "<<x_tile<<" y_tile: "<<y_tile<<" tile number: "<<tile_num<<"\n";

            for(int y = 0; y < 8; y++) {
                bg_fg_col_t colors{.val = vram.at(color_addr + y)};
                uint8_t tile_data = vram.at(tile_addr + y);
                uint8_t mask = 128;
                for(int x = 0; x < 8; x++) {
                    uint8_t color_index = 0;
                    if((tile_data & mask) == mask) color_index = colors.fields.foreground;
                    else color_index = colors.fields.background;

                    buffer[y_tile * 8 + y][3 * (x_tile * 8 + x) + 0] = tms_palette[color_index * 3 + 0];
                    buffer[y_tile * 8 + y][3 * (x_tile * 8 + x) + 1] = tms_palette[color_index * 3 + 1];
                    buffer[y_tile * 8 + y][3 * (x_tile * 8 + x) + 2] = tms_palette[color_index * 3 + 2];

                    mask>>=1;
                }
            }
        }
		//printf("\n");
    }
}

void vdpMS::renderText(std::vector<std::vector<uint8_t>>& buffer) {
    std::cout<<"Text (Mode 1) render\n";
}

void vdpMS::renderMulticolor(std::vector<std::vector<uint8_t>>& buffer) {
    std::cout<<"MultiColor (Mode 3) render\n";
}

void vdpMS::renderMode4(std::vector<std::vector<uint8_t>>& buffer) {
    //std::cout<<"SMS (Mode 4) render: NT: "<<std::hex<<name_tab_base()<<" BG Tiles: "<<bg_tile_base()<<" Palette: ";
    //for(int i=0;i<32;i++) std::cout<<int(pal_ram.at(i))<<" ";
    //std::cout<<"\n";
    for(int y_tile=0;y_tile<24;y_tile++) {
        int y_tile_off = (y_tile + (bg_y_scroll / 8)) % 24;
        for(int x_tile=0;x_tile<32;x_tile++) {
            int x_tile_off = (x_tile + (bg_x_scroll / 8)) % 32;
            tile_info_t tile_info;
            uint16_t tile_info_addr = (name_tab_base() + (y_tile_off * 64) + (x_tile_off * 2)) & 0x3fff;
            tile_info.bytes.byte1 = vram.at(tile_info_addr);
            tile_info.bytes.byte2 = vram.at(tile_info_addr + 1);
            uint16_t tile_addr = (bg_tile_base() + 32 * tile_info.fields.tile_num) & 0x3fff;
            for(int y = 0; y < 8; y++) {
                int y_off = (y + (bg_y_scroll % 8)) % 8;
                uint8_t byte0 = vram.at(tile_addr + y_off*4);
                uint8_t byte1 = vram.at(tile_addr + y_off*4 + 1);
                uint8_t byte2 = vram.at(tile_addr + y_off*4 + 2);
                uint8_t byte3 = vram.at(tile_addr + y_off*4 + 3);
                for(int x = 0; x < 8; x++) {
                    int x_off = (x + (bg_x_scroll % 8)) % 8;
                    uint32_t mask = 0x80>>x_off;
                    int color_index = (((mask & byte0) + 2*(mask & byte1) + 4*(mask & byte2) + 8*(mask&byte3)) >> (7-x_off)) + 16 * tile_info.fields.palnum;
                    //mask>>=1;
                    sms_color_t color{.val = pal_ram.at(color_index)};
                    buffer[y_tile*8+y][3*(x_tile*8+x)] = sms_pal_component[color.component.blue];
                    buffer[y_tile*8+y][3*(x_tile*8+x)+1] = sms_pal_component[color.component.green];
                    buffer[y_tile*8+y][3*(x_tile*8+x)+2] = sms_pal_component[color.component.red];
                }
            }
        }
    }
}

uint16_t vdpMS::name_tab_base() { // Register 2, starting address for Name Table sub-block (background layout)
    if(vdpMode == systemType::sg_1000) {
        return 0x400 * (nt_base & 0x0f);
    }
    return 0x400 * (nt_base & 0x0e);
}

uint16_t vdpMS::col_tab_base() { // Register 3, starting address for the Color Table
    return 0x40 * color_t_base;
}

uint16_t vdpMS::bg_tile_base() { // Register 4, starting address for the Pattern Generator Sub-block (background tiles)
    if(vdpMode == systemType::sg_1000) {
        return 0x800 * pt_base;
    }
    return 0;
}

uint16_t vdpMS::sprite_attr_tab_base() { // Register 5, starting address for the sprite attribute table (sprite locations, colors, etc)
    return 0x80 * spr_attr_base;
}

uint16_t vdpMS::sprite_tile_base() { // Register 6, starting address for the Sprite Pattern Generate sub-block (sprite tiles)
    return 0x800 * spr_tile_base;
}

uint64_t vdpMS::calc(uint64_t) {
    return 0;
}

bool vdpMS::lineInterrupt() {
    if(vdpMode != systemType::sg_1000 && ctrl_1.fields.line_interrupts) return line_int_active;
    else return false;
}

bool vdpMS::frameInterrupt() {
    if(ctrl_2.fields.frame_interrupts) return scr_int_active;
    else return false;
}

void vdpMS::endLine(uint64_t lineNum) {
    uint64_t line = lineNum % 262;
    curLine = line; //VCounter
    if(line == 191) scr_int_active = true;
    if(line < 192 && line_int_cur) {
        line_int_cur--;
    }
    if(line < 192 && !line_int_cur) {
        line_int_active = true;
        line_int_cur = line_interrupt;
    }
    else if(line >= 192) {
        line_int_cur = line_interrupt;
    }
}

void vdpMS::writeByte(uint8_t port, uint8_t val, uint64_t cycle) {
    std::printf("Wrote val(%02x) to port(%02x)\n", val, port);
    if(port % 2 == 1) writeAddress(val);
    else {
        addr_latch = false;
        writeData(val);
    }
}

uint8_t vdpMS::readByte(uint8_t port, uint64_t cycle) {
    switch(port & 0b11000001) {
        case 0x40: return readVCounter(cycle);
        case 0x41: return readHCounter(cycle);
        case 0x80: addr_latch = false; return readData();
        case 0x81: addr_latch = false; return readStatus(cycle);
        default: std::cerr<<"Shouldn't have reached the VDP\n";
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
        address = 0x100 * val + addr_buffer;

        switch(val & 0b11000000) {
        case 0x00: // VRAM read mode
            addr_mode = addr_mode_t::vram_read;
            data_buffer = vram[address++];
            dbg_printf(" set read address to %04x", address);
            break;
        case 0x40: // VRAM write mode
            addr_mode = addr_mode_t::vram_write;
            dbg_printf(" set write address to %04x", address);
            break;
        case 0x80: // VDP register write mode
            addr_mode = addr_mode_t::reg_write;
            dbg_printf(" set register %01x to %02x", (val & 0x0f), (address & 0x00ff));
            std::cout<<" set register "<<int(val & 0x0f)<<" to "<<std::hex<<int(address & 0x00ff)<<"\n";
            switch(val & 0x0f) {
                case 0x00:
                    ctrl_1.val = (address & 0x00ff);
                case 0x01:
                    ctrl_2.val = (address & 0x00ff);
                    break;
                case 0x02:
                    nt_base = (address & 0x00ff);
                    break;
                case 0x03:
                    color_t_base = (address & 0x00ff);
                    break;
                case 0x04:
                    pt_base = (address & 0x00ff);
                    break;
                case 0x05:
                    spr_attr_base = (address & 0x00ff);
                    break;
                case 0x06:
                    spr_tile_base = (address & 0x00ff);
                    break;
                case 0x07:
                    bg_fg_col.val = (address & 0x00ff);
                    break;
                case 0x08:
                    bg_x_scroll = (address & 0x00ff);
                    break;
                case 0x09:
                    bg_y_scroll = (address & 0x00ff);
                    break;
                case 0x0a:
                    line_interrupt = (address & 0x00ff);
                    break;
                default:
                    // no effect in SMS or SG-1000 for reg's B-F
                    break;
            }
            break;
        case 0xc0: // CRAM write mode
            addr_mode = addr_mode_t::cram_write;
            dbg_printf(" set cram write to address %02x", (address & 0x00ff));
            break;
        }
    }
}

void vdpMS::writeData(uint8_t val) {
    if(addr_mode == addr_mode_t::vram_write || addr_mode == addr_mode_t::vram_read || addr_mode == addr_mode_t::reg_write) {
        //dbg_printf(" wrote %02x to address %04x\n", val, address);
		std::printf(" wrote %02x to address %04x\n", val, address);
                   vram[address++] = val;
                   data_buffer = val;
    }
    else if(addr_mode == addr_mode_t::cram_write) {
        pal_ram[address % pal_ram.size()] = val;
        //std::printf(" wrote %02x to palette address %04x\n", val, address);
        data_buffer = val;
        address++;
    }
}

uint8_t vdpMS::readData() {
    uint8_t retval = data_buffer;
    data_buffer = vram[address++];
    return retval;
}

uint8_t vdpMS::readStatus(uint64_t cycle) {
    std::printf("Read VDP Status\n");
    vdpMS::status_t temp;

    // TODO: Calc sprite number
    // TODO: Keep track of overflow flag
    // TODO: Keep track of collision flag
    temp.val = 0;
    temp.fields.vblank_flag = scr_int_active;
    scr_int_active = false;
    line_int_active = false;
    return temp.val;
    //return 0x80;
}

uint8_t vdpMS::readVCounter(uint64_t cycle) {
    //std::printf("v: %ld\n", (cycle / 342) % 262);
    return curLine;
    //return (cycle / 342) % 262;
}

uint8_t vdpMS::readHCounter(uint64_t cycle) {
    //std::printf("h: %ld\n", (cycle / 262) % 342);
    return (cycle / 262) % 342;
}
