#include<iostream>
#include<fstream>
#include<iterator>
#include<vector>
#include "m68k.h"

using namespace std;

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
    romfile.close();
    for(int i=0;i<16;i++) {
        cout<<hex<<uint32_t(rom[i])<<" ";
    }
    cout<<endl;
    return 0;
}
