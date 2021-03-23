#pragma once
#include "../apu.h"
#include<array>

class apuGenesis : public apu {
public:
    apuGenesis(std::shared_ptr<config>& cfg);
    void mute(bool) override;
    void writeRegister(uint8_t val) override;
    void setStereo(uint8_t) override;
    std::array<int16_t, 882*2>& getSamples() override;

private:
    std::array<int16_t, 882*2> buffer;
};
