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
    void clearWrites() override;
    static const int sampleRate = 44100;
    static const int nativeOplSampleRate = 49716;
    static const int envAccumRate = 1'000'000 / sampleRate; // Microseconds per sample
private:
    void applyRegister(std::pair<uint8_t, uint8_t>& write);
    void updatePhases();
    void updateEnvelopes();
    class op_t;
    void percKeyOn(op_t* modOp, op_t* carOp);
    void percKeyOff(op_t* modOp, op_t* carOp);
    std::array<int16_t, 882 * 2> buffer;
    static const std::array<int,210> amTable;
    static const std::array<int,64> fmTable; 
    static const std::array<int,128> kslTable;
    static const std::array<int,64> attackTable;
    static const std::array<int,64> decayTable;
    uint8_t curReg;
    uint8_t statusVal;

    std::shared_ptr<config> cfg;
    int sampleCnt;
    double ticksPerSample;

    unsigned int writeIndex;
    std::array<std::pair<uint8_t, uint8_t>, 100> regWrites;

    static const int tremoloMultiplier = 8; // 4.8dB max amplitude
    static const int vibratoMultiplier = 2; // 14 cent max amplitude

    unsigned int envCounter; // Global counter for advancing envelope state

    int amPhase; // index into the amTable, for how deep to currently apply the AM value
    static const int amPhaseSampleLength = (sampleRate * 64) / nativeOplSampleRate; // Number of samples between progressing to a new index

    int fmPhase; // vibrato has 8 phases
    static const int fmPhaseSampleLength = (sampleRate * 1024) / nativeOplSampleRate;

    enum adsrPhase {
        silent,         //Note hit envelope==48dB and key-off
        dampen,         //End of previous note, behaves like a base decay rate of 12, adjusted by rate_key_scale
        attack,         //New note rising after key-on, uses attack-rate
        decay,          //Initial fade to sustain level after reaching max volume, uses decay-rate
        sustain,        //Level to hold at until key-off, or level at which to transition from decay to percussive/sustainRelease phase
        percussiveRelease, //key-off, release for percussive notes, behaves like decay rate of 7
        sustainRelease, //key-off, sustained release, behaves like decay rate of 5
        release         //key-off (note is released), uses release-rate
    };

    static const std::array<std::string,8> adsrPhaseNames;

    int lfsrStepGalois();
    uint32_t galoisState; // LFSR state for the rhythem channels
    bool galoisBit;

    struct inst_t {
        //reg 0
        bool amMod;           //tremolo (amplitude variance) @ 3.7Hz
        int amModAtten;          // current AM (tremolo) attenuation level

        bool vibMod;          //frequency variance @ 6.4Hz

        bool sustMod;         //EG-TYP, 1=sustained tone, 0=no sustain period
        bool keyScaleRateMod; //KSR: modify ADSR rate based on frequency
        unsigned multMod:4;   //frequency multiplier, 1 of 3 elements that define the frequency

        //reg 1
        bool amCar;           //tremolo (amplitude variance) @ 3.7Hz
        int amCarAtten;          // current AM (tremolo) attenuation level

        bool vibCar;          //frequency variance @ 6.4Hz

        bool sustCar;         //EG-TYP, 1=sustained tone, 0=no sustain period
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

    struct op_t {
        void updateEnvelope(unsigned int envCounter, bool mod);
        bool modulator;          // true=modulator, false=carrier
        inst_t* inst;            // pointer to the instrument being used.
        unsigned totalLevel:6;   // level for the modulator, 0.75dB steps (add totalLevel * 0x20 to output value)
        unsigned phaseInc:19;    // Basically the frequency, generated from the instrument's mult, and the fNum and octave/block for the channel
        unsigned phaseCnt:19;    // Current place in the sine phase. 10.9 fixed-point number, where the whole selects the sine sample to use

        // TODO: Modulator feedback state.
        int modFB1;
        int modFB2;

        bool releaseSustain;   //1=key-off has release-rate at 5, 0=key-off has release rate at 7 (both with KSR adjustment)

        adsrPhase envPhase;
        int envLevel; // 0 - 127. 0.375dB steps (add envLevel * 0x10)
        int envAccum; // microsecond count for envLevel increment/decrement

        bool keyOn; // mirror of on-off state of the key
    };

    struct chan_t {
        void keyOn(int chNum);
        void keyOff(int chNum);
        unsigned int fNum; // 2nd of 3 elements that define the frequency
        bool key; //on-off state of the key
        unsigned int octave; //3rd element that defines the frequency
        unsigned int volume; // volume for the channel, 3dB steps (add volume * 0x80 to output value)
        unsigned int instNum;

        unsigned kslIndex: 7; // Index to the KSL table (scales volume level by note)
        unsigned ksrIndex: 4; // Index to the KSR table (scales envelope by note)
        
        // FM/vibrato state tracking.
        int fmRow; // fmRow is decided by the top 3 bits of the current fNum for the channel
        int fmModShift;          // current FM (vibrato) phase shift level
        int fmCarShift;          // current FM (vibrato) phase shift level

        op_t modOp;
        op_t carOp;
    } chan[9];

    bool rhythm;          // Rhythm mode enabled

    struct percChan_t {
        void keyOn();
        void keyOff();
        bool key;
        chan_t* chan;
        op_t* modOp; // nullptr for everything but bass drum
        op_t* carOp;
        inst_t* instrument;
        unsigned int* volume;
    };
    //                          key     channel   modulator op     carrier op    instrument     volume
    percChan_t percChan[5] = { {false, &chan[6], &chan[6].modOp, &chan[6].carOp, &inst[16], &chan[6].volume},  // Bass Drum
                               {false, &chan[7], &chan[8].carOp, &chan[7].modOp, &inst[17], &chan[7].instNum}, // High Hat
                               {false, &chan[7], nullptr,        &chan[7].carOp, &inst[17], &chan[7].volume},  // Snare Drum
                               {false, &chan[8], nullptr,        &chan[8].modOp, &inst[18], &chan[8].instNum}, // Tom-tom
                               {false, &chan[8], &chan[7].modOp, &chan[8].carOp, &inst[18], &chan[8].volume}}; // Top Cymbal

    enum rhythmInsts {
        bassDrum,
        highHat,
        snareDrum,
        tomTom,
        topCymbal
    };

    static constexpr uint8_t multVal[16] = {1, 2, 4, 6, 8, 10, 12, 14, 16, 18, 20, 20, 24, 24, 30, 30};

    // Instrument values taken from https://siliconpr0n.org/archive/doku.php?id=vendor:yamaha:opl2#ym2413_instrument_rom
    // Apparently based on interpreting the on-die ROM on a YM2413b
    static constexpr uint8_t romInst[8*(1+15+3)] = {
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, // 0: User
        0x71,0x61,0x1e,0x17,0xd0,0x78,0x00,0x17, // 1: Violin
        0x13,0x41,0x1a,0x0d,0xd8,0xf7,0x23,0x13, // 2: Guitar
        0x13,0x01,0x99,0x00,0xf2,0xc4,0x11,0x23, // 3: Piano
        0x31,0x61,0x0e,0x07,0xa8,0x64,0x70,0x27, // 4: Flute
        0x32,0x21,0x1e,0x06,0xe0,0x76,0x00,0x28, // 5: Clarinet
        0x31,0x22,0x16,0x05,0xe0,0x71,0x00,0x18, // 6: Oboe
        0x21,0x61,0x1d,0x07,0x82,0x81,0x10,0x07, // 7: Trumpet
        0x23,0x21,0x2d,0x14,0xa2,0x72,0x00,0x07, // 8: Organ
        0x61,0x61,0x1b,0x06,0x64,0x65,0x10,0x17, // 9: Horn
        0x41,0x61,0x0b,0x18,0x85,0xf7,0x71,0x07, // A: Synthesizer
        0x13,0x01,0x83,0x11,0xfa,0xe4,0x10,0x04, // B: Harpsichord
        0x17,0xc1,0x24,0x07,0xf8,0xf8,0x22,0x12, // C: Vibraphone
        0x61,0x50,0x0c,0x05,0xc2,0xf5,0x20,0x42, // D: Synthsizer Bass
        0x01,0x01,0x55,0x03,0xc9,0x95,0x03,0x02, // E: Acoustic Bass
        0x61,0x41,0x89,0x03,0xf1,0xe4,0x40,0x13, // F: Electric Guitar
        0x01,0x01,0x18,0x0f,0xdf,0xf8,0x6a,0x6d, // R: Bass Drum (from VRC7)
        0x01,0x01,0x00,0x00,0xc8,0xd8,0xa7,0x48, // R: High-Hat(M) / Snare Drum(C) (from VRC7)
        0x05,0x01,0x00,0x00,0xf8,0xaa,0x59,0x55, // R: Tom-tom(M) / Top Cymbal(C) (from VRC7)
    };

    static const std::string instNames[];
    static const std::string rhythmNames[];

    static std::array<int, 1024 * 2> logsinTable;
    static std::array<int, 256> expTable;

    void initTables();
    int lookupSin(int val, bool waveForm);
    int lookupExp(int val);
    int convertWavelength(int wavelength);
};
