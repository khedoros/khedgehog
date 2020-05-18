#include "util.h"

uint32_t bswap(uint8_t) = delete;
uint32_t bswap(uint32_t in) {
    uint8_t * conv = reinterpret_cast<uint8_t*>(&in);
    std::swap(conv[0], conv[3]);
    std::swap(conv[1], conv[2]);
    return in;
}
uint16_t bswap(uint16_t in) {
    uint8_t * conv = reinterpret_cast<uint8_t*>(&in);
    std::swap(conv[0], conv[1]);
    return in;
}

