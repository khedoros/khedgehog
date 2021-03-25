#include "tiPsg.h"
#include<iostream>
#include<cassert>
#include<fstream>

TiPsg::TiPsg(std::shared_ptr<config>& conf): noiseLfsr(1<<15), cfg(conf), apu(conf), latchedChannel(0), writeCount(0) {
    toneCountReset[3] = 0x10;

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
    //output.open("audio.raw");
}

void TiPsg::mute(bool) {}

void TiPsg::writeRegister(uint8_t val) {
    writes[writeCount] = val;
    writeCount++;
}

void TiPsg::applyRegister(uint8_t val) {
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
            //std::cout<<"PSG: Ch#"<<latchedChannel<<" set attenuation: "<<int(attenuation[latchedChannel])<<"\n";
        }
        else { // wavelength data
            if(latchedChannel < 3) { // Square waves
                toneCountReset[latchedChannel] &= 0b1111110000; // clear lowest 4 bits
                toneCountReset[latchedChannel] |= (val & 0x0f); // set lowest 4 bits
                if(latchedChannel == 2 && noiseShiftRate == 3) {
                    toneCountReset[3] = toneCountReset[2];
                }
                //std::cout<<"PSG: Ch#"<<latchedChannel<<" set wavelength: "<<toneCountReset[latchedChannel]<<"\n";
            }
            else { // Noise channel
                noiseMode = ((val>>2) & 1);
                noiseShiftRate = (val & 0x03);
                noiseLfsr = (1<<15);
                switch(noiseShiftRate) {
                    case 0: toneCountReset[3] = 0x20; break;
                    case 1: toneCountReset[3] = 0x40; break;
                    case 2: toneCountReset[3] = 0x80; break;
                    case 3: toneCountReset[3] =  2 * toneCountReset[2]; break;
                }
                //std::cout<<"PSG: Ch#"<<latchedChannel<<" noise mode: "<<noiseMode<<" shift rate: "<<noiseShiftRate<<" ("<<toneCountReset[3]<<"\n";
            }
        }
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
            //std::cout<<"PSG: Ch#"<<latchedChannel<<" set wavelength: "<<toneCountReset[latchedChannel]<<"\n";
        }
        else {
            noiseMode = ((val>>2) & 1);
            noiseShiftRate = (val & 0x03);
            noiseLfsr = (1<<15);
            switch(noiseShiftRate) {
                case 0: toneCountReset[3] = 0x10; break;
                case 1: toneCountReset[3] = 0x20; break;
                case 2: toneCountReset[3] = 0x40; break;
                case 3: toneCountReset[3] = toneCountReset[2]; break;
            }
            //std::cout<<"PSG: Ch#"<<latchedChannel<<" noise mode: "<<noiseMode<<" shift rate: "<<noiseShiftRate<<" ("<<toneCountReset[3]<<"\n";
        }
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
std::array<int16_t, 882 * 2>& TiPsg::getSamples() {
    //std::cout<<"Write Count this frame: "<<std::dec<<writeCount<<"\n";
    buffer.fill(0);
    int writeIndex = 0;
    for(int i=0;i<sampleCnt*stereoChannels;i+=stereoChannels) {


        // TODO: Man...this is super-hacky. Basically, handle the writes in pairs because that's what Sonic 1+2 GG do
        // And spread register writes evenly through the frame besides that.
        // Fixing it properly would require the CPU to send the write cycle to the memmap, so that writes could be
        // timed properly in the frame. But...this works, so far. Easy way out.
        /**/ float percent = float(i) / float(sampleCnt * stereoChannels);
        /**/ if(int(percent * writeCount) >= writeIndex) {
        /**/    applyRegister(writes[writeIndex++]);
        /**/    if(writeIndex < writeCount) {
        /**/        applyRegister(writes[writeIndex++]);
        /**/    }
        /**/}

        for(int channel = 0; channel < 4; channel++) {
            if(toneCountReset[channel] >= 2) {
                toneCount[channel] -= ticksPerSample;
                if(toneCount[channel] <= 0) {
                    toneCount[channel] += toneCountReset[channel];
                    if(channel < 3) { //square channels
                        currentOutput[channel] = !currentOutput[channel];
                    }
                    else { // noise channel
                        currentOutput[channel] = (noiseLfsr & 0x01);
                        int newVal = 0;
                        if(noiseMode == 0) {
                            newVal = ((noiseLfsr & 0x01)<<15);
                        }
                        else {
                            newVal = ((((noiseLfsr & 0x08) >> 3) ^ (noiseLfsr & 0x01))<<15);
                        }
                        //std::cout<<"PSG: Ch#3 NoiseLFSR "<<std::hex<<noiseLfsr;
                        noiseLfsr >>= 1;
                        noiseLfsr |= newVal;
                        //std::cout<<"->"<<noiseLfsr<<"\n";

                    }
                }// else std::cout<<"PSG: Ch#"<<channel<<" toneCount: "<<toneCount[channel]<<"\n";
            }
            //else std::cout<<"PSG: Ch#"<<channel<<" reset value: "<<toneCountReset[channel]<<"\n";
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
    writeCount = 0;
    //output.write(reinterpret_cast<char*>(buffer.data()), sampleCnt * stereoChannels * 2);
    return buffer;
}

