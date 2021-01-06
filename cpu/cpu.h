#pragma once

#include<cstdint>

class cpu {
public:
    virtual uint64_t calc(uint64_t) = 0;
    virtual void interrupt(uint8_t) = 0;
};
