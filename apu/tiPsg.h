#pragma once
#include<cstdint>
#include<array>
#include "apu.h"
#include<fstream>

// 3579545Hz for NTSC systems and 3546893Hz
// 16x divisor for internal clock
//
class TiPsg: public apu {
public:
    TiPsg(std::shared_ptr<config>& conf);
    void mute(bool) override;
    void writeRegister(uint8_t val) override;
    uint8_t readRegister(uint8_t reg) override;
    void setStereo(uint8_t) override;
    std::array<int16_t, 882 * 2>& getSamples() override;
private:
    void applyRegister(uint8_t val);
	std::shared_ptr<config> cfg;
    unsigned latchedChannel:2;
    bool latchedType:1; // 0 = data, 1 = volume
    uint16_t toneCountReset[4]; // 0 and 1 just give a constant "1" output. First audible signal on a real SMS2 was at Reset value 6, at 18643Hz
                                   // Noise channel resets: b00 = 0x10, b01 = 0x20, b02 = 0x40, b03 = value of Tone2's reset(?)
    double toneCount[4];        // Current count for wavelength. Resets when it hits 0.
    uint8_t attenuation[4]; // Attenuation values would output volumes like this:
    int volume_table[16]={ 32767, 26028, 20675, 16422, 13045, 10362,  8231,  6568, 5193,  4125,  3277,  2603,  2067,  1642,  1304,     0};
    bool currentOutput[4]; // Currently outputting low or high voltage
    unsigned noiseShiftRate:2;
    bool noiseMode; // 0 = periodic, 1 = white
                                   // Noise channel uses 2 different Linear-Feedback Shift Registers (looks like it starts with 1, is 15 bits wide for SG-1000,
                                   // 16 bits for the others, shifts right)
                                   //   On the 16-bit one, has the 13th and 16th bits tapped with a XOR.
                                   //   For the 15-bit one, the tapped bits are the 15th and 11th bits, feeding back into the first.
                                   // For periodic "noise", it looks like it's just a frequency-divisor, with the only tap being on the highest bit, copying it back to the lowest bit.
    unsigned noiseLfsr:16;


    // Port 6 on the Game Gear, bits 0-3 are channels 0-3 on the right, bits 4-7 are channels 0-3 on the left.
    bool stereoLeft[4];
    bool stereoRight[4];

    std::array<int16_t, 882 * 2> buffer;

	int stereoChannels; // channel count to render
	int sampleCnt; // number of samples to render per request
    double ticksPerSample; // How many ticks in a sample
    std::ofstream output;
	int writeCount;
	std::array<uint8_t, 2048> writes;
};
