#pragma once

#include "../vdp.h"

class vdpGenesis: public vdp {
    public:
    vdpGenesis(systemType t = systemType::genesis, systemRegion r = systemRegion::ntsc);
    std::vector<std::vector<uint8_t>> getPartialRender() override; // Render a composited view of the current VDP memory state
    std::vector<std::vector<uint8_t>> getDebugRender() override;
    std::vector<std::vector<uint8_t>>& getFrameBuffer() override; // Get current framebuffer state (e.g. after completing the frame)

    void writeByte(uint8_t, uint8_t, uint64_t) override;
    uint8_t readByte(uint8_t, uint64_t) override;
    uint64_t calc(uint64_t) override;
    bool lineInterrupt() override;
    bool frameInterrupt() override;
    void endLine(uint64_t) override;

    public:
    std::vector<std::vector<uint8_t>> buffer;
};
