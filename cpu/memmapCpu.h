#pragma once
#include "../memmap.h"

class memmapCpu: public memmap {
protected:
    virtual uint8_t& map(uint32_t addr) = 0;
};
