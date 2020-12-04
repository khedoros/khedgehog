#include "vdpMS.h"
#include "../../util.h"

vdpMS::vdpMS():addr_latch(false) {

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
