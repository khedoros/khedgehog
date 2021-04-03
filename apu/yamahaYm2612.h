#pragma once
#include "apu.h"

class YamahaYm2612: public apu {
public:
    YamahaYm2612(std::shared_ptr<config>& conf);
    void mute(bool) override;
    void writeRegister(uint8_t val) override;
    void setStereo(uint8_t) override;
    std::array<int16_t, 882 * 2>& getSamples() override;
private:
    std::array<int16_t, 882 * 2> buffer;
};

