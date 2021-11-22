#pragma once

#include "../vdp.h"

class vdpGenesis: public vdp {
    public:
    vdpGenesis(systemType t = systemType::genesis, systemRegion r = systemRegion::us_ntsc);
    std::vector<uint8_t> getPartialRender() override; // Render a composited view of the current VDP memory state
    std::vector<uint8_t> getDebugRender() override;
    std::vector<uint8_t>& getFrameBuffer() override; // Get current framebuffer state (e.g. after completing the frame)
    int getStride() override;

    void writeByte(uint8_t, uint8_t, uint64_t) override;
    uint8_t readByte(uint8_t, uint64_t) override;
    uint64_t calc(uint64_t) override;
    bool lineInterrupt() override;
    bool frameInterrupt() override;
    void endLine(uint64_t) override;
    unsigned int resLine() override;

    private:
    void resizeBuffer(unsigned int x, unsigned int y);
    std::vector<uint8_t> buffer;
    unsigned int curXRes;
    unsigned int curYRes;
};
