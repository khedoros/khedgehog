#pragma once
#include "../apu.h"
#include<array>

class apuGenesis : public apu {
public:
    apuGenesis();
    void mute(bool) override;
    void writeRegister(uint8_t val) override;
    void setStereo(uint8_t) override;
    std::array<int16_t, 735*2>& getSamples() override;

private:
    std::array<int16_t, 735*2> buffer;
};
