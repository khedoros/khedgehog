#include<cstdint>
#include<cstdio>

//#define dbg_printf std::printf
#define dbg_printf dummy

uint32_t bswap(uint8_t) = delete;
uint32_t bswap(uint32_t in);
uint16_t bswap(uint16_t in);
void dummy(const char* format, ...);

