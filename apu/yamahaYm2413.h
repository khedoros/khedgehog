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
	void applyRegister(std::pair<uint8_t, uint8_t>& write);
    std::array<int16_t, 882 * 2> buffer;
    uint8_t curReg;
    uint8_t statusVal;

	std::shared_ptr<config> cfg;
	int sampleCnt;
	double ticksPerSample;

    unsigned int writeIndex;
    std::array<std::pair<uint8_t, uint8_t>, 100> regWrites;
    double sine[512];

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

	enum adsrPhase {
		dampen,  //End of previous note, behaves like a base decay rate of 12, adjusted by rate_key_scale
		attack,  //New note rising after key-on
		decay,   //Initial fade to sustain level after reaching max volume
		sustain, //Level to hold at until key-off, or level at which to transition from decay to release phase
		release, //key-off for non-percussive notes, sustain for percussive
        release2 //key-off for percussive notes
	};

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

        //ADSR reg 4-7
        unsigned attack:4;
        unsigned decay:4;
        unsigned sustain_level:4;
        unsigned release:4;

        unsigned phaseInc:18;
		unsigned phaseCnt:18;
		adsrPhase envPhase;
		//TODO: Add something about an envelope counter

    } op[18];

    struct chan_t {
        unsigned fNum:9; // 2nd of 3 elements that define the frequency
        bool sustain; //1=key-off has release-rate at 5, 0=key-off has release-rate at 7 (both with KSR adjustment)
        bool trigger; //on-off state of the key
        unsigned octave:3; //3rd element that defines the frequency
        unsigned instrument:4; //instrument choice 0=custom, 1-f=built-in instruments
        unsigned volume:4; // volume for the channel
        op_t mod_op;
		op_t car_op;
    } chan[9];

    static constexpr uint8_t multVal[16] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 10, 12, 12, 15, 15};

	static constexpr uint8_t instruments[8*(1+15+3)] = {
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, // 0: User
		0x71,0x61,0x1e,0x17,0xd0,0x78,0x00,0x17, // 1: Violin
		0x13,0x41,0x1a,0x0d,0xd8,0xf7,0x23,0x13, // 2: Guitar
		0x13,0x01,0x99,0x00,0xf2,0xc4,0x21,0x23, // 3: Piano
		0x11,0x61,0x0e,0x07,0x8d,0x64,0x70,0x27, // 4: Flute
		0x32,0x21,0x1e,0x06,0xe1,0x76,0x01,0x28, // 5: Clarinet
		0x31,0x22,0x16,0x05,0xe0,0x71,0x00,0x18, // 6: Oboe
		0x21,0x61,0x1d,0x07,0x82,0x81,0x11,0x07, // 7: Trumpet
		0x33,0x21,0x2d,0x13,0xb0,0x70,0x00,0x07, // 8: Organ
		0x61,0x61,0x1b,0x06,0x64,0x65,0x10,0x17, // 9: Horn
		0x41,0x61,0x0b,0x18,0x85,0xf0,0x81,0x07, // A: Synthesizer
		0x33,0x01,0x83,0x11,0xea,0xef,0x10,0x04, // B: Harpsichord
		0x17,0xc1,0x24,0x07,0xf8,0xf8,0x22,0x12, // C: Vibraphone
		0x61,0x50,0x0c,0x05,0xd2,0xf5,0x40,0x42, // D: Synthsizer Bass
		0x01,0x01,0x55,0x03,0xe9,0x90,0x03,0x02, // E: Acoustic Bass
		0x41,0x41,0x89,0x03,0xf1,0xe4,0xc0,0x13, // F: Electric Guitar
		0x01,0x01,0x18,0x0f,0xdf,0xf8,0x6a,0x6d, // R: Bass Drum (from VRC7)
		0x01,0x01,0x00,0x00,0xc8,0xd8,0xa7,0x68, // R: High-Hat(M) / Snare Drum(C) (from VRC7)
		0x05,0x01,0x00,0x00,0xf8,0xaa,0x59,0x55, // R: Tom-tom(M) / Top Cymbal(C) (from VRC7)
	};
};
