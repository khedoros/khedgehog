#pragma once
#include "apu.h"
#include<utility>

class YamahaYm2413: public apu {
public:
    YamahaYm2413(std::shared_ptr<config>& conf);
    void mute(bool) override;
    void writeRegister(uint8_t val) override;
    void writeRegister(uint8_t port, uint8_t val) override;
    uint8_t readRegister(uint8_t reg) override;
    void setStereo(uint8_t) override;
    std::array<int16_t, 882 * 2>& getSamples() override;
private:
    std::array<int16_t, 882 * 2> buffer;
    uint8_t curReg;
    uint8_t statusVal;

    uint8_t reg[0x39] = {0};

    unsigned int writeIndex;
    std::array<std::pair<uint8_t, uint8_t>, 100> regWrites;

    bool rhythm;             //               rhythm f-nums
    bool bass_trigger;       //slot 13+16      addr 16  =  20
    bool snare_trigger;      //slot 17              17  =  50
    bool tom_trigger;        //slot 15              18  =  C0
    bool top_cymbal_trigger; //slot 18              26  =  05
    bool high_hat_trigger;   //slot 14              27  =  05
                             //                     28  =  01
    unsigned bass_vol:4;  //0x36-low
    unsigned snare_vol:4; //0x37-low
    unsigned tom_vol:4; //0x38-high
    unsigned high_hat_vol:4; //0x37-high
    unsigned top_cym_vol:4; //0x38-low

    struct op_t {
        //reg 0+1
        bool am;      //tremolo (amplitude variance) @ 3.7Hz
        bool vibrato; //frequency variance @ 6.4Hz
        bool sustain;        //1=sustained tone, 0=no sustain period
        bool rate_key_scale; //KSR: modify ADSR rate based on frequency
        unsigned mult:4;     //frequency multiplier, 1 of 3 elements that define the frequency

        //reg 2+3
        unsigned level_key_scale:2; //KSL: modify volume based on frequency
        unsigned mod_total_level:6; //modulation attenuation
        bool waveform;              //0=sine, 1=only positive
        unsigned feedback:3;        //feedback level into modulator operator (I think)

        //ADSR
        unsigned attack:4;
        unsigned decay:4;
        unsigned sustain_level:4;
        unsigned release:4;
    } op[18];

    struct chan_t {
        unsigned fNum:9; // 2nd of 3 elements that define the frequency
        bool sustain; //1=key-off has release-rate at 5
        bool trigger; //on-off state of the key
        unsigned octave:3; //3rd element that defines the frequency
        unsigned instrument:4; //instrument choice 0=custom, 1-f=built-in instruments
        unsigned volume:4; // volume for the channel
    } chan[9];
};
