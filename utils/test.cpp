#include<iostream>
#include<cstdint>
#include<cassert>

int main() {
    uint32_t reg = 0xff00ff;
    int16_t offset = -32;
    uint32_t res = reg + offset;
    std::printf("%06x + %d = %06x (%d)\n", reg, offset, res, reg + offset);

    union basicDisplacement {
        uint16_t val;
        #pragma pack(push, 1)
        struct {
            unsigned unused:1;
            unsigned scale:2;
            unsigned size:1;
            unsigned regnum:3;
            unsigned da:1;
            signed displacement:8;
        } indexWord;
        #pragma pack(pop)
    } w;

    std::cout<<(sizeof(w))<<std::endl;
    assert(sizeof(w) == 2);

    assert(sizeof(w.indexWord) == 2);
    w.val = 0xffff;

    std::cout<<w.indexWord.da<<std::endl;
    std::cout<<w.indexWord.displacement<<std::endl;

    return 0;
}
