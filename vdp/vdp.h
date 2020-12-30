#pragma once
#include<vector>
#include<cstdint>

class vdp {
public:
    virtual std::vector<std::vector<uint8_t>> getPartialRender() = 0; // Render a composited view of the current VDP memory state
    virtual std::vector<std::vector<uint8_t>> getSpritePartialRender() = 0; // Render the sprite layer(s) of the current VDP memory state
    virtual std::vector<std::vector<uint8_t>> getBgPartialRender() = 0; // Render the background layer(s) of the current VDP memory state
    virtual uint64_t calc(uint64_t) = 0;
};
