#pragma once
#include<cstdint>

class apu {
    virtual void mute(bool) = 0;
    virtual void writeRegister(uint8_t val) = 0;
    virtual void setStereo(uint8_t) = 0;
};
