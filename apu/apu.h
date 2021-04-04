#pragma once
#include<cstdint>
#include<array>
#include<memory>

#include "../config.h"

class apu {
    public:
    apu(std::shared_ptr<config>& cfg);
    virtual void mute(bool) = 0;
    virtual void writeRegister(uint8_t val) = 0;
    virtual void writeRegister(uint8_t port, uint8_t val) = 0;
    virtual void setStereo(uint8_t) = 0;
    virtual std::array<int16_t, 882 * 2>& getSamples() = 0;
    virtual uint8_t readRegister(uint8_t reg) = 0;
};
