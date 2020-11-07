#include "memmapZ80.h"

uint8_t& memmapZ80::readByte(uint32_t addr) {
    return dummy8;
}

uint16_t& memmapZ80::readWord(uint32_t addr) {
    return dummy16;
}

uint32_t& memmapZ80::readLong(uint32_t addr) {
    return dummy32;
}

void memmapZ80::writeByte(uint32_t addr, uint8_t val) {}
void memmapZ80::writeWord(uint32_t addr, uint16_t val) {}
void memmapZ80::writeLong(uint32_t addr, uint32_t val) {}
