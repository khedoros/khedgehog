#pragma once
#include<cstdint>
#include<array>

class apu {
    public:
    virtual void mute(bool) = 0;
    virtual void writeRegister(uint8_t val) = 0;
    virtual void setStereo(uint8_t) = 0;
    virtual std::array<int16_t, 735 * 2>& getSamples() = 0;
};
