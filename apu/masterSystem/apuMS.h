#pragma once
#include<cstdint>
#include<array>
#include <memory>

#include "../tiPsg.h"
#include "../yamahaYm2413.h"
#include "../../config.h"


class apuMS : public apu {
public:
    apuMS(std::shared_ptr<config>& cfg);
    void mute(bool) override;
    void writeRegister(uint8_t val) override;
    void writeRegister(uint8_t port, uint8_t val) override;
    void setStereo(uint8_t) override;
    std::array<int16_t, 882 * 2>& getSamples() override;
    uint8_t readRegister(uint8_t port) override;
    void clearWrites() override;
private:
    std::shared_ptr<config> cfg;
    std::shared_ptr<TiPsg> psg;
    std::shared_ptr<YamahaYm2413> fm;
};

