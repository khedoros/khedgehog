#include "tiPsg.h"
#include<iostream>
#include<cassert>

TiPsg::TiPsg(std::shared_ptr<config>& conf): noiseLfsr(1), cfg(conf), apu(conf), latchedChannel(0) {
    for(int i=0;i<4;i++) {
        stereoLeft[i] = true;
        stereoRight[i] = true;
        attenuation[i] = 15;
        toneCountReset[i] = 0;
        currentOutput[i] = true;
        toneCount[i] = 0;
    }

    for(int i=0;i<882*2;i++) {
        buffer[i] = 0;
    }
    stereoChannels = 1;
	sampleCnt = 882;
    ticksPerSample = 5.03;

    if(cfg->getSystemType() == systemType::gameGear) {
        stereoChannels = 2;
        sampleCnt = 735;
        ticksPerSample = 5.07;
    }
    else if(cfg->getSystemRegion() != systemRegion::pal) {
        sampleCnt = 735;
        ticksPerSample = 5.07;
    }
}

void TiPsg::mute(bool) {}

void TiPsg::writeRegister(uint8_t val) {
    std::cout<<"PSG: Write\n";
    if(val & 0x80) { //register
        //        %1cctdddd
        //        cc = channel
        //        t = type (0 = data, 1 = volume)
        //        dddd = data
        //
        //        Noise: Lowest 2 bits are the shift rate, 3rd is noise mode.
        latchedChannel = ((val>>5)&0x03);
        latchedType = ((val>>4)&0x01);
        if(latchedType) { // volume
            attenuation[latchedChannel] = (val & 0x0f);
        }
        else { // wavelength data
            if(latchedChannel < 3) { // Square waves
                toneCountReset[latchedChannel] &= 0b1111110000; // clear lowest 4 bits
                toneCountReset[latchedChannel] |= (val & 0x0f); // set lowest 4 bits
                if(latchedChannel == 2 && noiseShiftRate == 3) {
                    toneCountReset[3] = toneCountReset[2];
                }
            }
            else { // Noise channel
                noiseMode = ((val>>2) & 1);
                noiseShiftRate = (val & 0x03);
                noiseLfsr = 1;
                switch(noiseShiftRate) {
                    case 0: toneCountReset[3] = 0x10; break;
                    case 1: toneCountReset[3] = 0x20; break;
                    case 2: toneCountReset[3] = 0x40; break;
                    case 3: toneCountReset[3] = toneCountReset[2]; break;
                }
            }
        }
        std::cout<<"PSG: Latched ch#"<<latchedChannel<<" type "<<latchedType<<"\n";
    }
    else { //data
        // %0_dddddd
        // dddddd = data, either top 6 bits of 10-bit wavelength counter, 4 bits of volume, 3 bits of noise settings.
        if(latchedChannel < 3) {
            toneCountReset[latchedChannel] &= 0b1111;
            toneCountReset[latchedChannel] |= ((val & 0b00111111)<<4);
            if(latchedChannel == 2 && noiseShiftRate == 3) {
                toneCountReset[3] = toneCountReset[2];
            }
        }
        else {
            noiseMode = ((val>>2) & 1);
            noiseShiftRate = (val & 0x03);
            noiseLfsr = 1;
            switch(noiseShiftRate) {
                case 0: toneCountReset[3] = 0x10; break;
                case 1: toneCountReset[3] = 0x20; break;
                case 2: toneCountReset[3] = 0x40; break;
                case 3: toneCountReset[3] = toneCountReset[2]; break;
            }
        }
        std::cout<<"PSG: Ch#"<<latchedChannel<<" reset: "<<toneCountReset[latchedChannel]<<"\n";
    }
}

void TiPsg::setStereo(uint8_t val) {
    for(int i=0;i<4;i++) {
        stereoRight[i] = (val & 0x01);
        val>>=1;
    }
    for(int i=0;i<4;i++) {
        stereoLeft[i] = (val & 0x01);
        val>>=1;
    }
}

// NTSC:
// 3579545Hz clock, 16x divisor, 60 FPS, 735 samples per frame
// 59659 cycles per frame, 3728.69 max half-waves per frame
// Each 5.07 divided-clock-ticks makes an output sample.
//
// PAL:
// 3546893Hz clock, 16x divisor, 50 FPS, 882 samples per frame
// 70938 cycles per frame, 4433.62 max half-waves per frame
// Each 5.03 divided-clock-ticks makes an output sample.
// TODO: Fix the audio code to expect something besides 60 FPS
std::array<int16_t, 882 * 2>& TiPsg::getSamples() {
    buffer.fill(0);
	for(int i=0;i<sampleCnt*stereoChannels;i+=stereoChannels) {
        for(int channel = 0; channel < 3; channel++) {
            toneCount[channel] -= ticksPerSample;
            if(toneCount[channel] <= 0) {
                toneCount[channel] = toneCountReset[channel];
                currentOutput[channel] = !currentOutput[channel];
                if(toneCount[channel] < 2) currentOutput[channel] = 1;
            }
            int16_t sampleVal = volume_table[attenuation[channel]] / 4;
            if(currentOutput[channel]) sampleVal *= -1;

            if(stereoChannels == 1) {
                buffer[i] += sampleVal;
            }
            if(stereoChannels == 2) {
                if(stereoLeft[channel]) {
                    buffer[i] += sampleVal;
                }
                if(stereoRight[channel]) {
                    buffer[i+1] += sampleVal;
                }
            }
            //std::cout<<"Ch#"<<channel<<": "<<buffer[i]<<"\t";
        }
        //std::cout<<"\n";
	}
    return buffer;
}

