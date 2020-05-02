#include<iostream>
#include<fstream>
#include<iterator>
#include<vector>
#include "m68k.h"

using namespace std;

uint16_t get_word(std::vector<uint8_t>& rom, uint32_t index) {
	uint16_t val = rom[index];
	val *= 256;
	val += rom[index+1];
	return val;
}

uint32_t get_long(std::vector<uint8_t>& rom, uint32_t index) {
	uint32_t val = rom[index];
	val *= 256;
	val += rom[index+1];
	val *= 256;
	val += rom[index+2];
	val *= 256;
	val += rom[index+3];
	return val;
}

uint32_t bswap(uint8_t) = delete;
uint32_t bswap(uint32_t in) {
	uint8_t * conv = reinterpret_cast<uint8_t*>(&in);
	swap(conv[0], conv[3]);
	swap(conv[1], conv[2]);
	return in;
}
uint16_t bswap(uint16_t in) {
	uint8_t * conv = reinterpret_cast<uint8_t*>(&in);
	swap(conv[0], conv[1]);
	return in;
}

int main(int argc, char *argv[]) {
    if(argc != 2) {
        std::cerr<<"Give me a filename."<<std::endl;
        return 1;
    }
    ifstream romfile(argv[1]);
    if(!romfile.is_open()) {
        cerr<<"Couldn't open file at "<<argv[1]<<"."<<endl;
        return 1;
    }
    std::istream_iterator<uint8_t> start(romfile), end;
    vector<uint8_t> rom(start, end);
    std::vector<uint32_t> entry_points;
    romfile.close();
    for(int i=0;i<16;i++) {
        if(i==0) {
            std::printf("Starting stack address: ");
            entry_points.push_back(get_long(rom, i*2));
            std::printf("(%08x) ", entry_points.back());
        }
        else if(i==2) {
            std::printf("\nReset vector: ");
            entry_points.push_back(get_long(rom, i*2));
            std::printf("(%08x) ", entry_points.back());
        }
        else if(i==4) {
            std::printf("\n");
        }
        std::printf("%02x%02x ", rom[i*2], rom[i*2+1]);
    }
	std::printf("\n\n%08x %08x\n", entry_points.front(), bswap(entry_points.front()));
	std::printf("\n\n%08x %08x\n", entry_points.front(), bswap(entry_points.back()));
    cout<<endl;
    return 0;
}
