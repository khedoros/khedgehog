#include "vdpMS.h"
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
                addr_mode = vram_read;
                address = val;
                address <<= 8;
                address += addr_buffer;
                data_buffer = vram[address++];
                break;
            case 0x40: // VRAM write mode
                addr_mode = vram_write;
                address = val;
                address <<= 8;
                address += addr_buffer;
                break;
            case 0x80: // VDP register write mode
                // TODO: Implement register writes
                addr_mode = reg_write;
                break;
            case 0xc0: // CRAM write mode
                addr_mode = cram_write;
                pal_address = addr_buffer;
                break;
            }
        }
    }
    void vdpMS::writeData(uint8_t val) {}
    uint8_t vdpMS::readData() {
        return 0;
    }
    uint8_t vdpMS::readStatus() {
        return 0;
    }
