#pragma once
#include "apu.h"
#include<utility>
#include<string>

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
    class inst_t;
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

    unsigned int envCounter; // Global counter for advancing envelope state

    enum adsrPhase {
        silent,         //Note hit envelope==48dB
        dampen,         //End of previous note, behaves like a base decay rate of 12, adjusted by rate_key_scale
        attack,         //New note rising after key-on
        decay,          //Initial fade to sustain level after reaching max volume
        sustain,        //Level to hold at until key-off, or level at which to transition from decay to sustainRelease phase
        sustainRelease, //sustain for percussive notes
        release         //key-off
    };

    struct op_t {
        void updateEnvelope(unsigned int envCounter, bool mod);
        bool modulator;          // true=modulator, false=carrier
        inst_t* inst;            // pointer to the instrument being used.
        unsigned totalLevel:6;   // level for the modulator, 0.75dB steps (add totalLevel * 0x20 to output value)
        unsigned phaseInc:19;    // Basically the frequency, generated from the instrument's mult, and the fNum and octave/block for the channel
        unsigned phaseCnt:19;    // Current place in the sine phase. 10.9 fixed-point number, where the whole selects the sine sample to use

        // TODO: AM/tremolo state. amPhase is a placeholder.
        unsigned amPhase:4;

        // TODO: FM/vibrato state. vibPhase is a placeholder.
        unsigned vibPhase:4;

        // TODO: Modulator feedback state.
        unsigned int modFB1;
        unsigned int modFB2;

        bool releaseSustain;   //1=key-off has release-rate at 5, 0=key-off has release rate at 7 (both with KSR adjustment)

        adsrPhase envPhase;
        unsigned int envLevel; // 0 - 127. 0.375dB steps (add envLevel * 0x10)


    };

    struct inst_t {
        //reg 0
        bool amMod;           //tremolo (amplitude variance) @ 3.7Hz
        bool vibMod;          //frequency variance @ 6.4Hz
        bool sustMod;         //1=sustained tone, 0=no sustain period
        bool keyScaleRateMod; //KSR: modify ADSR rate based on frequency
        unsigned multMod:4;   //frequency multiplier, 1 of 3 elements that define the frequency

        //reg 1
        bool amCar;           //tremolo (amplitude variance) @ 3.7Hz
        bool vibCar;          //frequency variance @ 6.4Hz
        bool sustCar;         //1=sustained tone, 0=no sustain period
        bool keyScaleRateCar; //KSR: modify ADSR rate based on frequency
        unsigned multCar:4;   //frequency multiplier, 1 of 3 elements that define the frequency

        //reg 2
        unsigned keyScaleLevelMod:2; //KSL: modify volume based on frequency
        unsigned totalLevelMod:6;    //modulation attenuation

        //reg 3
        unsigned keyScaleLevelCar:2; //KSL: modify volume based on frequency
        bool waveformMod;            //0=sine, 1=only positive
        bool waveformCar;            //0=sine, 1=only positive
        unsigned feedbackMod:3;      //feedback level into modulator operator (I think)

        //reg 4
        unsigned attackMod:4;
        unsigned decayMod:4;
        //reg 5
        unsigned attackCar:4;
        unsigned decayCar:4;
        //reg 6
        unsigned sustainLevelMod:4;
        unsigned releaseMod:4;
        //reg 7
        unsigned sustainLevelCar:4;
        unsigned releaseCar:4;
    } inst[19];

    struct chan_t {
        unsigned int fNum; // 2nd of 3 elements that define the frequency
        bool keyOn; //on-off state of the key
        unsigned int octave; //3rd element that defines the frequency
        unsigned int volume; // volume for the channel, 3dB steps (add volume * 0x80 to output value)
        unsigned int instNum;
        op_t modOp;
        op_t carOp;
    } chan[9];

    bool rhythm;          // Rhythm mode enabled

    struct percChan_t {
        bool keyOn;
        chan_t* chan;
        op_t* modOp; // nullptr for everything but bass drum
        op_t* carOp;
        inst_t* instrument;
        unsigned int* volume;

    };

    percChan_t percChan[5] = { {false, &chan[6], &chan[6].modOp, &chan[6].carOp, &inst[16], &chan[6].volume},  // Bass Drum
                               {false, &chan[7], nullptr,        &chan[7].modOp, &inst[17], &chan[7].instNum}, // High Hat
                               {false, &chan[7], nullptr,        &chan[7].carOp, &inst[17], &chan[7].volume},  // Snare Drum
                               {false, &chan[8], nullptr,        &chan[8].modOp, &inst[18], &chan[8].instNum}, // Tom-tom
                               {false, &chan[8], nullptr,        &chan[8].carOp, &inst[18], &chan[8].volume}}; // Top Cymbal

    enum rhythmInsts {
        bassDrum,
        highHat,
        snareDrum,
        tomTom,
        topCymbal
    };

                    /*
    bool bassKeyOn;       //slot 13+16      addr 16  =  20
    bool highHatKeyOn;    //slot 14              27  =  05
    bool tomKeyOn;        //slot 15              18  =  C0
    bool snareKeyOn;      //slot 17              17  =  50
    bool topCymbalKeyOn;  //slot 18              26  =  05
                             //                     28  =  01
    unsigned bass_vol:4;     //0x36-low
    unsigned snare_vol:4;    //0x37-low
    unsigned tom_vol:4;      //0x38-high
    unsigned high_hat_vol:4; //0x37-high
    unsigned top_cym_vol:4;  //0x38-low

    op_t * bassMod = &chan[6].modOp;
    op_t * bassCar = &chan[6].carOp;
    op_t * highHatCar = &chan[7].modOp;
    op_t * snareCar = &chan[7].carOp;
    op_t * tomCar = &chan[8].modOp;
    op_t * topCymCar = &chan[8].carOp;
*/
    static constexpr uint8_t multVal[16] = {1, 2, 4, 6, 8, 10, 12, 14, 16, 18, 20, 20, 24, 24, 30, 30};

    static constexpr uint8_t romInst[8*(1+15+3)] = {
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

	static const std::string instNames[];
	static const std::string rhythmNames[];

    static int logsinTable[256];
    static int expTable[256];

    void initTables();
    int lookupSin(int val);
    int lookupExp(int val);
};
