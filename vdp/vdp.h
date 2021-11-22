#pragma once
#include<vector>
#include<cstdint>
#include "../config.h"

class vdp {
public:
    virtual std::vector<uint8_t> getPartialRender() = 0; // Render a composited view of the current VDP memory state
    virtual std::vector<uint8_t> getDebugRender() = 0; // Render a rawer view of VDP memory
    virtual std::vector<uint8_t>& getFrameBuffer() = 0; // Get current framebuffer state
    virtual int getStride() = 0;


    virtual uint64_t calc(uint64_t) = 0;
    virtual void writeByte(uint8_t, uint8_t, uint64_t) = 0;
    virtual uint8_t readByte(uint8_t, uint64_t) = 0;
    virtual bool lineInterrupt() = 0;
    virtual bool frameInterrupt() = 0;
    virtual void endLine(uint64_t) = 0;
    virtual unsigned int resLine() = 0;
};
