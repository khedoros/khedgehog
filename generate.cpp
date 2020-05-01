#include<iostream>
#include<fstream>
#include<cstdint>
#include<byteswap.h>

int main() {
    std::ofstream out("out.bin");
    uint16_t no_op = __bswap_constant_16(0b0100111001110001);
    for(uint32_t i=0; i<=65535; i++) {
        uint16_t t = bswap_16(i);
        out.write(reinterpret_cast<char *>(&t), 2);
        out.write(reinterpret_cast<char *>(&no_op), 2);
        out.write(reinterpret_cast<char *>(&no_op), 2);
    }
    out.close();
}
