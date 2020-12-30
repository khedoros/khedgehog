#pragma once

#include "../vdp.h"

class vdpGenesis: public vdp {
    std::vector<std::vector<uint8_t>> getPartialRender() override; // Render a composited view of the current VDP memory state
    std::vector<std::vector<uint8_t>> getSpritePartialRender() override; // Render the sprite layer(s) of the current VDP memory state
    std::vector<std::vector<uint8_t>> getBgPartialRender() override; // Render the background layer(s) of the current VDP memory state
    uint64_t calc(uint64_t) override;

};
