#pragma once
#include "../apu.h"

class apuGenesis : public apu {
    apuGenesis();
    void mute(bool) override;
    void writeRegister(uint8_t val) override;
    void setStereo(uint8_t) override;

};
