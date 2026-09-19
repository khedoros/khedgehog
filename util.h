#include<cstdint>
#include<cstdio>

#ifdef DEBUG
    #define dbg_printf printf
#else
    #define dbg_printf dummy
#endif

uint32_t bswap(uint8_t) = delete;
uint32_t bswap(uint32_t in);
uint16_t bswap(uint16_t in);
void dummy(const char* format, ...);

