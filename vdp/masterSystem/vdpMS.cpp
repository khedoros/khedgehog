#include<iostream>
#include<cassert>
#include<algorithm>

#include "vdpMS.h"
#include "../../util.h"
#include "../font.h"


vdpMS::vdpMS(systemType t, systemRegion r):addr_latch(false), vdpMode(t), vdpRegion(r) {
    if(vdpMode == systemType::gameGear) pal_ram.resize(0x40, 0);
    else                                pal_ram.resize(0x20, 0);

    switch(t) {
        case systemType::sg_1000:
            std::cout<<"VDP started in SG-1000 mode\n";
            resizeBuffer(256, 192);
            break;
        case systemType::gameGear:
            std::cout<<"VDP started in GameGear mode\n";
            resizeBuffer(160, 144);
            break;
        case systemType::masterSystem:
            std::cout<<"VDP startedin Master System mode\n";
            resizeBuffer(256, 192);
            break;
    }
}

void vdpMS::resizeBuffer(unsigned int x, unsigned int y) {
    curXRes = x;
    curYRes = y;
    buffer.resize(curYRes * curXRes * 3, 0);
}

vdpMS::graphicsMode_t vdpMS::getMode() {
    int mode = 8 * ctrl_1.fields.mode_4 +
        4 * ctrl_2.fields.mode_3 +
        2 * ctrl_1.fields.mode_2 +
        ctrl_2.fields.mode_1;
    switch(mode) {
        case 0: return graphicsMode_t::graphics1;
        case 1: return graphicsMode_t::text;
        case 2: return graphicsMode_t::graphics2;
        case 4: return graphicsMode_t::multicolor;
        case 8: case 10: case 12: return graphicsMode_t::mode4;
        case 11: return graphicsMode_t::mode4_224;
        case 14: return graphicsMode_t::mode4_240;
        default: return graphicsMode_t::unknown;
    }
}

int vdpMS::getStride() {
    return curXRes * 3;
}

std::vector<uint8_t>& vdpMS::getFrameBuffer() {
    return buffer;
}

void vdpMS::renderLine(unsigned int line, std::vector<uint8_t>& renderBuffer) {
    if((line >= curYRes && vdpMode != systemType::gameGear) || line >= curYRes + 3 * 24 ) return;
    if(!ctrl_2.fields.enable_display) {
               if(vdpMode == systemType::gameGear && line >= 3 * 24) line -= 3 * 24;
        for(int x=0;x<curXRes;x++) {
            renderBuffer[curXRes * 3 * line + x * 3 + 0] = 0;
            renderBuffer[curXRes * 3 * line + x * 3 + 1] = 0;
            renderBuffer[curXRes * 3 * line + x * 3 + 2] = 0;
        }
        return;
    }
    switch(getMode()) {
        case graphicsMode_t::graphics1:
            renderGraphic1(line, renderBuffer);
            break;
        case graphicsMode_t::graphics2:
            renderGraphic2(line, renderBuffer);
            break;
        case graphicsMode_t::text:
            renderText(line, renderBuffer);
            break;
        case graphicsMode_t::multicolor:
            renderMulticolor(line, renderBuffer);
            break;
        case graphicsMode_t::mode4:
            renderMode4(line, renderBuffer);
            break;
        case graphicsMode_t::mode4_224:
            std::cerr<<"224-line mode 4 not implemented yet\n";
            break;
        case graphicsMode_t::mode4_240:
            std::cerr<<"240-line mode 4 not implemented yet\n";
            break;
        default:
            std::cerr<<"Unsupported rendering mode "<<static_cast<int>(getMode())<<"\n";
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

}

std::vector<uint8_t> vdpMS::getPartialRender() {
    std::vector<uint8_t> buffer(192 * 256 * 3, 0);
    if(!ctrl_2.fields.enable_display) return buffer;
    for(int i=0;i<192;i++) renderLine(i, buffer);
    return buffer;
}

void vdpMS::renderGraphic1(unsigned int line, std::vector<uint8_t>& buffer) {
    std::cout<<"Graphic I (Mode 0)  render\n";
    int y_tile = line / 8;
    int y_line = line % 8;
    for(int x_tile=0;x_tile<32;x_tile++) {
        int tile_num_addr = (name_tab_base() + y_tile * 32 + x_tile) & 0x3fff;
        int tile_num = vram.at(tile_num_addr);
        int tile_addr = (bg_tile_base() + tile_num * 8) & 0x3fff;

        int color_addr = (col_tab_base() + tile_num / 8) & 0x3fff;
        bg_fg_col_t colors{.val = vram.at(color_addr)};

        auto tileLine = getG2TileLine(tile_addr, y_line);
        for(int x = 0; x < 8; x++) {
            uint8_t color_index = tileLine[x] ? colors.fields.foreground: colors.fields.background;
            setPixelSG(buffer, x_tile * 8 + x, y_tile * 8 + y_line, color_index);
        }
    }
}

void vdpMS::renderGraphic2(unsigned int line, std::vector<uint8_t>& buffer) {
    //std::cout<<"Graphic II (Mode 2) render ntbase: "<<name_tab_base()<<" ttbase: "<<bg_tile_base()<<" ctbase: "<<col_tab_base()<<"\n";
    int yTile = line / 8;
    int yTriad = yTile / 8;
    int yLine = line % 8;
    for(int x_tile=0;x_tile<32;x_tile++) {
        int tile_num_addr = (name_tab_base() + yTile * 32 + x_tile) & 0x3fff;
        int tile_num = vram.at(tile_num_addr) + 256 * yTriad;
        int tile_addr = ((bg_tile_base() & 0x2000) + tile_num * 8) & 0x3fff;

        int color_addr = ((col_tab_base() & 0x2000) + tile_num * 8) & 0x3fff ;

        bg_fg_col_t colors{.val = vram.at(color_addr + yLine)};
        auto line = getG2TileLine(tile_addr, yLine);
        for(int x = 0; x < 8; x++) {
            uint8_t color_index = line[x] ? colors.fields.foreground : colors.fields.background;
            setPixelSG(buffer, x_tile * 8 + x, yTile * 8 + yLine, color_index);
        }
    }
    renderSgSprites(line, buffer);
}

void vdpMS::renderSgSprites(unsigned int line, std::vector<uint8_t>& buffer) {
    int sprHeight = 8;
    if(ctrl_2.fields.doubled_sprites && ctrl_2.fields.large_sprites) sprHeight = 32;
    else if(ctrl_2.fields.doubled_sprites || ctrl_2.fields.large_sprites) sprHeight = 16;
    uint16_t sprAttrTabAddr = sprite_attr_tab_base();
    std::array<uint8_t, 4> sprSearch;
    int sprCount = 0;

    // Find sprites on this line, and handle overflow
    for(int spr = 0; spr < 32 && sprCount != 5; spr++) {
        int sprY = vram.at(sprAttrTabAddr + spr * 4);
        if(line >= sprY && line - sprY < sprHeight) {
            if(sprCount == 4) {
                status.fields.sprite_num = spr;
                status.fields.overflow_flag = true;
            }
            else {
                sprSearch[sprCount] = spr;
            }
            sprCount++;
        }
    }

    // Check collision
    std::vector<uint8_t> sprCollis(sprCount);
    for(int i=0;i<sprCount;i++) {
        int sprX = vram.at(sprAttrTabAddr + sprSearch[i] * 4 + 1); // sprite X coordinate
        int info = vram.at(sprAttrTabAddr + sprSearch[i] * 4 + 3); // sprite info
        if(info & 0x80) sprX -= 32;
        sprCollis[i] = sprX;
    }
    std::sort(sprCollis.begin(), sprCollis.end());
    int first = 0;
    if(sprCount > 0) first = sprCollis[0];
    for(int i=1;i<sprCount;i++) {
        if(sprCollis[i] - first < sprHeight) status.fields.collision_flag = 1;
        first = sprCollis[i];
    }

    // Draw the sprites
    int tileRepeat = ctrl_2.fields.large_sprites + 1; // 4 tiles drawn in a square
    int pixelRepeat = ctrl_2.fields.doubled_sprites + 1; // tile pixels are doubled

    for(int spr = 0; spr < sprCount; spr++) {
        int sprY = vram.at(sprAttrTabAddr + sprSearch[spr] * 4 + 0);
        int sprX = vram.at(sprAttrTabAddr + sprSearch[spr] * 4 + 1);
        int tile = vram.at(sprAttrTabAddr + sprSearch[spr] * 4 + 2);
        int info = vram.at(sprAttrTabAddr + sprSearch[spr] * 4 + 3);

        if(info & 0x80) sprX -= 32;
        int color = (info & 0x0f);

        int sprLine = line - sprY;
        if(ctrl_2.fields.doubled_sprites) {
            sprLine /= 2;
        }
        if(ctrl_2.fields.large_sprites && sprLine > 7) {
            tile++; // vertical tile increment
            sprLine -= 8;
        }
        for(int t = 0; t < tileRepeat; t++) {
            auto tileData = getG2TileLine(sprite_tile_base() + tile * 8, sprLine);
            for(int x = 0; x < 8; x++) {
                for(int r = 0; r < pixelRepeat; r++) {
                    if(tileData[x])
                        setPixelSG(buffer, sprX + pixelRepeat * x + r, line, color);    
                }
            }
            tile += 2; // horizontal tile increment
        }

    }
}

void vdpMS::renderSmsSprites(unsigned int line, std::vector<uint8_t>& buffer) {

}

// 

void vdpMS::renderText(unsigned int line, std::vector<uint8_t>& buffer) {
    std::cout<<"Text (Mode 1) render\n";
}

void vdpMS::renderMulticolor(unsigned int line, std::vector<uint8_t>& buffer) {
    std::cout<<"MultiColor (Mode 3) render\n";
}

void vdpMS::renderMode4(unsigned int line, std::vector<uint8_t>& buffer) {

    int scrYStart = 0, scrYEnd = 192, scrXStart = 0, scrXEnd = 256;
    if(vdpMode == systemType::gameGear) {
        scrYStart = 3 * 8;
        scrYEnd = 192 - 3 * 8;
        scrXStart = 6 * 8;
        scrXEnd = 256 - 6 * 8;
    }

    if(line < scrYStart || line >= scrYEnd) return;

    int sprHeight = (ctrl_2.fields.large_sprites)? 16 : 8;

    // Argument is `line` for consistency with other functions,
    // but "scrY" was the original loop variable when this function wasn't line-based
    int scrY = line;

    std::array<int, 8> sprSearch;
    int sprCount = 0;
    for(int i = 0; vram.at(sprite_attr_tab_base() + i) != 0xd0 && i < 64;i++) {
        int y = vram.at(sprite_attr_tab_base() + i) + 1;
        if(scrY - y >= 0 && scrY - y < sprHeight) {
            if(sprCount == 8) {
                status.fields.overflow_flag = 1;
                break;
            }
            else {
                sprSearch[sprCount++] = i;
            }
        }
    }

    std::array<int, 256> lineBuffer{0};

    // Draw sprites into lineBuffer
    for(int spr = 0; spr < sprCount; spr++) {
        int y = vram.at(sprite_attr_tab_base() + sprSearch[spr]) + 1;
        int x = vram.at(sprite_attr_tab_base() + 128 + sprSearch[spr] * 2);
        int tile = vram.at(sprite_attr_tab_base() + 128 + sprSearch[spr] * 2 + 1);
        int fineY = scrY - y;
        //std::cout<<"Sprite#: "<<spr<<" Line: "<<scrY<<" Spr_y: "<<y
        assert(fineY >= 0);
        assert(fineY < sprHeight);

        uint16_t tile_addr = (sprite_tile_base() + 32 * tile) & 0x3fff;
        auto line = getM4TileLine(tile_addr, fineY);

        for(int xFine = 0; xFine < 8 && xFine + x < 256; xFine++) {
            int color_index = line[xFine] + 16;
            if(color_index != 16) {
                if(lineBuffer[x + xFine] != 0)  {
                    status.fields.collision_flag = 1;
                }
                lineBuffer[x + xFine] = color_index;
            }
        }
    }

    for(int scrX = scrXStart; scrX < scrXEnd; scrX++) {

        int bgY = scrY; // Lock last 8 tiles so they don't scroll vertically
        if(scrX < 192 || !ctrl_1.fields.v_scroll_lock) { // Otherwise, allow them to scroll
            bgY = (scrY + bg_y_scroll) % (28*8);
        }
        int yTile = bgY / 8;
        int yFine = bgY % 8;

        int bgX = scrX; // Lock first 2 tiles so they don't scroll horizontally
        if(!ctrl_1.fields.h_scroll_lock || line >= 16) {  // Otherwise, allow them to scroll
            bgX = (scrX + (255 - bg_x_scroll)) % (32 * 8);
        }
        int xTile = bgX / 8;
        int xFine = bgX % 8;

        uint16_t tile_info_addr = (name_tab_base() + (yTile * 64) + (xTile * 2)) & 0x3fff;
        tile_info_t tile_info;
        tile_info.bytes.byte1 = vram.at(tile_info_addr);
        tile_info.bytes.byte2 = vram.at(tile_info_addr + 1);
        uint16_t tile_addr = (bg_tile_base() + 32 * tile_info.fields.tile_num) & 0x3fff;
        int tileLine = yFine;
        if(tile_info.fields.vflip) {
            tileLine = 7 - yFine;
        }
        uint8_t byte0 = vram.at(tile_addr + tileLine * 4);
        uint8_t byte1 = vram.at(tile_addr + tileLine * 4 + 1);
        uint8_t byte2 = vram.at(tile_addr + tileLine * 4 + 2);
        uint8_t byte3 = vram.at(tile_addr + tileLine * 4 + 3);

        uint32_t mask = 0x80>>xFine;
        uint32_t indexShift = 7-xFine;
        if(tile_info.fields.hflip) {
            mask = 0x01<<xFine;
            indexShift = xFine;
        }

        // Actual background pixel-drawing
        // Find color of correct background tile
        int color_index = (((mask & byte0) + 2*(mask & byte1) + 4*(mask & byte2) + 8*(mask&byte3)) >> indexShift) + 16 * tile_info.fields.palnum;

        //
        if(lineBuffer[scrX] && !tile_info.fields.priority) color_index = lineBuffer[scrX];
        if(!color_index) color_index = 16 + bg_fg_col.fields.background;

        if(vdpMode == systemType::masterSystem) {
            if(scrX < 8 && ctrl_1.fields.hide_left) { // hide left column
                color_index = 16 + bg_fg_col.fields.background;
            }
            setPixelSMS(buffer, scrX, scrY, color_index);
        }
        else if(vdpMode == systemType::gameGear) {
            setPixelGG(buffer, scrX - scrXStart, scrY - scrYStart, color_index);
        }
    }
}

std::vector<uint8_t> vdpMS::getDebugRender() {
    if(getMode() == graphicsMode_t::mode4) {
        return getDBM4Render();
    }
    else if(getMode() == graphicsMode_t::graphics2) {
        return getDBG2Render();
    }
    return std::vector<uint8_t>(512 * 512*3,0xff);
}

std::vector<uint8_t> vdpMS::getDBG2Render() {
    std::vector<uint8_t> buffer(512*512*3, 0);
    for(int tileRow = 0; tileRow < 64; tileRow++) {
        for(int tileCol = 0; tileCol < 32; tileCol++) {
            for(int fineRow = 0; fineRow < 8; fineRow++) {
                uint16_t tileAddr = 256 * tileRow + 8 * tileCol;
                auto line = getG2TileLine(tileAddr, fineRow);
                for(int x = 0; x < 8; x++) {
                    uint8_t toSet = line[x] * 255;
                    buffer.at(512*3*((tileRow * 8) + fineRow) + (3 * ((tileCol * 8) +  x) + 0)) = toSet;
                    buffer.at(512*3*((tileRow * 8) + fineRow) + (3 * ((tileCol * 8) +  x) + 1)) = toSet;
                    buffer.at(512*3*((tileRow * 8) + fineRow) + (3 * ((tileCol * 8) +  x) + 2)) = toSet;
                }
            }
        }
    }
    return buffer;
}

std::vector<uint8_t> vdpMS::getDBM4Render() {
    std::vector<uint8_t> buffer(512 * 512 * 3, 0);
    for(int tileRow = 0; tileRow < 32; tileRow++) {
        for(int tileCol = 0; tileCol < 16; tileCol++) {
            for(int fineRow = 0; fineRow < 8; fineRow++) {
                uint16_t tileAddr = tileRow * 512 + tileCol * 32;
                auto line = getM4TileLine(tileAddr, fineRow);
                for(int x = 0; x < 8; x++) {
                    buffer.at(512 * 3 * ((tileRow * 8) + fineRow)+(3 * ((tileCol * 8) +  x) + 0)) = tms_palette[line[x] * 3 + 0];
                    buffer.at(512 * 3 * ((tileRow * 8) + fineRow)+(3 * ((tileCol * 8) +  x) + 1)) = tms_palette[line[x] * 3 + 1];
                    buffer.at(512 * 3 * ((tileRow * 8) + fineRow)+(3 * ((tileCol * 8) +  x) + 2)) = tms_palette[line[x] * 3 + 2];
                }
            }
        }
    }
    return buffer;
}

std::array<uint8_t, 8> vdpMS::getG2TileLine(uint16_t tileAddr, uint8_t row) {
    std::array<uint8_t, 8> retval;
    uint8_t val = vram.at(tileAddr + row);
    uint8_t mask = 128;
    for(int x = 0; x < 8; x++) {
        retval[x] = (val & mask) >> (7 - x);
        mask>>=1;
    }
    return retval;
}

std::array<uint8_t, 8> vdpMS::getM4TileLine(uint16_t tileAddr, uint8_t row) {
    std::array<uint8_t, 8> retval;
    uint8_t data0 = vram.at(tileAddr + row * 4 + 0);
    uint8_t data1 = vram.at(tileAddr + row * 4 + 1);
    uint8_t data2 = vram.at(tileAddr + row * 4 + 2);
    uint8_t data3 = vram.at(tileAddr + row * 4 + 3);

    uint8_t mask = 128;
    for(int x = 0; x < 8; x++) {
        retval[x] = ((data0 & mask) >> (7 - x)) + 2 * ((data1 & mask) >> (7 - x)) + 4 * ((data2 & mask) >> (7 - x)) + 8 * ((data3 & mask) >> (7 - x));
        mask>>=1;
    }
    return retval;
}

void vdpMS::setPixelSG(std::vector<uint8_t>& buffer, int x, int y, int index) {
    if(x < 0 || x >= curXRes || y < 0 || y >= curYRes) return;
    //std::cout<<"y: "<<y<<" x: "<<x<<"\n";
    buffer[y * 256 * 3 + 3 * x + 0] = tms_palette[index * 3 + 2];
    buffer[y * 256 * 3 + 3 * x + 1] = tms_palette[index * 3 + 1];
    buffer[y * 256 * 3 + 3 * x + 2] = tms_palette[index * 3 + 0];
}

void vdpMS::setPixelGG(std::vector<uint8_t>& buffer, int x, int y, int index) {
    //assert(x >= 0);
    //assert(x < 160);
    //assert(y >= 0);
    //assert(y < 144);
    //assert(buffer.size() == 144 * 160 * 3);
    if(x >= curXRes || y >= curYRes) return;
    gg_color_t color;
    if(index == 0) index = bg_fg_col.fields.background;
    else index %= pal_ram.size();
    color.val[0] = pal_ram.at(index * 2);
    color.val[1] = pal_ram.at(index * 2 + 1);
    buffer[y * 160 * 3 + 3 * x + 0] = gg_pal_component[color.component.blue];
    buffer[y * 160 * 3 + 3 * x + 1] = gg_pal_component[color.component.green];
    buffer[y * 160 * 3 + 3 * x + 2] = gg_pal_component[color.component.red];
}

void vdpMS::setPixelSMS(std::vector<uint8_t>& buffer, int x, int y, int index) {
    if(x >= curXRes || y >= curYRes) return;
    if(index == 0) index = bg_fg_col.fields.background;
    else index %= pal_ram.size();
    sms_color_t color{.val = pal_ram.at(index)};
    buffer[y * 256 * 3 + 3 * x + 0] = sms_pal_component[color.component.blue];
    buffer[y * 256 * 3 + 3 * x + 1] = sms_pal_component[color.component.green];
    buffer[y * 256 * 3 + 3 * x + 2] = sms_pal_component[color.component.red];
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
    //return (pt_base & 0x4) * 0x800;
    return 0;
}

uint16_t vdpMS::sprite_attr_tab_base() { // Register 5, starting address for the sprite attribute table (sprite locations, colors, etc)
    if(vdpMode == systemType::sg_1000) {
        return 0x80 * spr_attr_base;
    }
    return (0x80 * (spr_attr_base & 0x7e));
}

uint16_t vdpMS::sprite_tile_base() { // Register 6, starting address for the Sprite Pattern Generate sub-block (sprite tiles)
    if(vdpMode == systemType::sg_1000) {
        return 0x800 * spr_tile_base;
    }
    if(spr_tile_base & 0x04) return 0x2000;
    else return 0;
    //std::printf("%04x\n", spr_tile_base);
    //return 0x0000;
}

uint64_t vdpMS::calc(uint64_t) {
    return 0;
}

bool vdpMS::lineInterrupt() {
    if(vdpMode != systemType::sg_1000 && ctrl_1.fields.line_interrupts) {
        bool retval = line_int_active;
        line_int_active = false;
        return retval;
    }
    else return false;
}

bool vdpMS::frameInterrupt() {
    if(ctrl_2.fields.frame_interrupts) {
        bool retval = scr_int_active;
        scr_int_active = false;
        return retval;
    }
    else return false;
}

unsigned int vdpMS::getFrameLines() {
    if(vdpRegion == systemRegion::ntsc || vdpMode == systemType::gameGear) return 262;
    else if(vdpRegion == systemRegion::pal) return 314;
    return 288;
}

void vdpMS::endLine(uint64_t lineNum) {
    uint64_t line = lineNum % getFrameLines();
    curLine = line; //VCounter
    if(line == 191) {
        scr_int_active = true;
        status.fields.vblank_flag = 1;
    }
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
    renderLine(line, buffer);
}

void vdpMS::writeByte(uint8_t port, uint8_t val, uint64_t cycle) {
    //std::printf("Wrote val(%02x) to port(%02x) = ", val, port);
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
        //std::printf("low byte of address\n");
    }
    else {
        addr_latch = false;
        address = 0x100 * val + addr_buffer;

        switch(val & 0b11000000) {
            case 0x00: // VRAM read mode
                addr_mode = addr_mode_t::vram_read;
                data_buffer = vram[address++];
                //std::printf(" set read address to %04x\n", address);
                break;
            case 0x40: // VRAM write mode
                addr_mode = addr_mode_t::vram_write;
                //std::printf(" set write address to %04x\n", address);
                break;
            case 0x80: // VDP register write mode
                addr_mode = addr_mode_t::reg_write;
                //std::printf(" set register %01x to %02x\n", (val & 0x0f), (address & 0x00ff));
                switch(val & 0x0f) {
                    case 0x00:
                        ctrl_1.val = (address & 0x00ff);
                        curMode = getMode();
                        break;
                    case 0x01:
                        ctrl_2.val = (address & 0x00ff);
                        curMode = getMode();
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
                        //std::printf("scroll bg to %d\n", bg_x_scroll);
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
                //std::printf(" set cram write to address %02x\n", (address & 0x00ff));
                break;
        }
    }
}

void vdpMS::writeData(uint8_t val) {
    if(addr_mode == addr_mode_t::vram_write || addr_mode == addr_mode_t::vram_read || addr_mode == addr_mode_t::reg_write) {
        //dbg_printf(" wrote %02x to address %04x\n", val, address);
        //std::printf(" wrote %02x to address %04x\n", val, address);
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
    //std::printf("Read VDP Status\n");
    //vdpMS::status_t temp;

    // TODO: Calc sprite number (SG-1000 only) (DONE)
    // TODO: Keep track of overflow flag (SG-1000: 5th sprite (DONE), SMS: 9th sprite)
    // TODO: Keep track of collision flag (SG: includes transparent pixels (DONE), SMS: Doesn't?)
    //temp.val = 0;
    //temp.fields.vblank_flag = scr_int_active;
    //scr_int_active = false;
    //line_int_active = false;
    //return temp.val;
    //return 0x80;
    uint8_t retval = status.val;
    status.fields.collision_flag = 0;
    status.fields.overflow_flag = 0;
    status.fields.vblank_flag = 0;
    return retval;
}

uint8_t vdpMS::readVCounter(uint64_t cycle) {
    if(vdpRegion == systemRegion::ntsc) {
        // NTSC, 256x192 00-DA, D5-FF
        // NTSC, 256x224 00-EA, E5-FF
        // NTSC, 256x240 00-FF, 00-06
        switch(getMode()) {
            case graphicsMode_t::graphics1:
            case graphicsMode_t::text:
            case graphicsMode_t::graphics2:
            case graphicsMode_t::multicolor:
            case graphicsMode_t::mode4: break;
            case graphicsMode_t::mode4_224: break;
            case graphicsMode_t::mode4_240: break;
        }
    }
    else { //systemRegion == pal
        // PAL,  256x192 00-F2, BA-FF
        // PAL,  256x224 00-FF, 00-02, CA-FF
        // PAL,  256x240 00-FF, 00-0A, D2-FF
        switch(getMode()) {
            case graphicsMode_t::graphics1:
            case graphicsMode_t::text:
            case graphicsMode_t::graphics2:
            case graphicsMode_t::multicolor:
            case graphicsMode_t::mode4: break;
            case graphicsMode_t::mode4_224: break;
            case graphicsMode_t::mode4_240: break;
        }
    }
    return curLine;
}

uint8_t vdpMS::readHCounter(uint64_t cycle) {
    //std::printf("h: %ld\n", (cycle / 262) % 342);
    return (cycle / 262) % 342;
}
