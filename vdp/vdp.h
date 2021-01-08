#pragma once
#include<vector>
#include<cstdint>
#include "../config.h"

class vdp {
public:
    virtual std::vector<std::vector<uint8_t>> getPartialRender() = 0; // Render a composited view of the current VDP memory state
    virtual uint64_t calc(uint64_t) = 0;
    virtual void writeByte(uint8_t, uint8_t, uint64_t) = 0;
    virtual uint8_t readByte(uint8_t, uint64_t) = 0;
};
