#pragma once
#include<cstdint>

// 3579545Hz for NTSC systems and 3546893Hz
// 16x divisor for internal clock
//
class TiPsg: public apu {
public:
    TiPsg();
    void mute(bool);
    void writeRegister(uint8_t val);
    void setStereo(uint8_t);
private:
    unsigned latchedChannel:2;
    bool latchedType:1; // 0 = data, 1 = volume
    unsigned toneCountReset[4]:10; // 0 and 1 just give a constant "1" output. First audible signal on a real SMS2 was at Reset value 6, at 18643Hz
                                   // Noise channel resets: b00 = 0x10, b01 = 0x20, b02 = 0x40, b03 = value of Tone2's reset(?)
                                   // Noise channel uses 2 different Linear-Feedback Shift Registers (looks like it starts with 1, is 15 bits wide for SG-1000,
                                   // 16 bits for the others, shifts right, and at least on the 16-bit one, has the 13th and 16th bits tapped with a XOR.
    unsigned attenuation[4]:4; // Attenuation values would output volumes like this:
    //  int volume_table[16]={ 32767, 26028, 20675, 16422, 13045, 10362,  8231,  6568, 5193,  4125,  3277,  2603,  2067,  1642,  1304,     0};
 }
    bool currentOutput[4]; // Currently outputting low or high voltage
    unsigned noiseShiftRate:2;
    bool noiseMode; // 0 = periodic, 1 = white

    // Port 6 on the Game Gear, bits 0-3 are channels 0-3 on the right, bits 4-7 are channels 0-3 on the left.
    bool stereoLeft[4];
    bool stereoRight[4];
};
