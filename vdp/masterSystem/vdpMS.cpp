#include "vdpMS.h"
#include "../../util.h"

vdpMS::vdpMS(systemType t, systemRegion r):addr_latch(false), vdpMode(t), vdpRegion(r) {

}

std::vector<std::vector<uint8_t>> vdpMS::getPartialRender() {
    int mode = 8 * ctrl_1.mode_4 + 4 * ctrl_2.mode_3 + 2 * ctrl_1.mode_2 + ctrl_2.mode_1;
    // SG-1000 modes:
    // 0: Graphic I
    // 1: Text I
    // 2: Graphic II
    // 3: Mode 1+2
    // 4: Multicolor
    // 5: Mode 1+3
    // 6: Mode 2+3
    // 7: Mode 1+2+3
    // Only applicable to SMS:
    // 8: Mode 4 (SMS mode)
    // 9: Invalid text mode
    // A: Mode 4
    // B: Invalid Text Mode (VDP1) / Mode 4 (224-line display in VDP2)
    // C: Mode 4
    // D: Invalid text mode
    // E: Mode 4 (VDP1) / Mode 4 (240-line display in VDP2)
    // F: Invalid Text Mode (VDP1) / Mode 4 (VDP2)
    return std::vector<std::vector<uint8_t>>(192, std::vector<uint8_t>(256*3, 0));
}

std::vector<std::vector<uint8_t>> vdpMS::getSpritePartialRender() {
    return std::vector<std::vector<uint8_t>>(192, std::vector<uint8_t>(256*3, 0));
}

std::vector<std::vector<uint8_t>> vdpMS::getBgPartialRender() {
    return std::vector<std::vector<uint8_t>>(192, std::vector<uint8_t>(256*3, 0));
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
    return 0;
}
