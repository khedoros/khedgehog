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
    std::cout<<"If you have an m68k-compatible copy of objdump, you can disassemble the output binary like this:\n\n\tm68k-linux-gnu-objdump -m m68k:68000 -b binary -D out.bin\n\n";
}
