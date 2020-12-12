#include "debug_console.h"
#include "util.h"
#include<iostream>

void dbg_con::init() {
    attr = 0x0f;
    reset();
}

void dbg_con::reset() {
    row = 0;
    col = 0;
    mode = 0;
    top_row = 0;
}

void dbg_con::write_control(uint8_t val) {
    //std::cout<<"\ndeb control: "<<std::hex<<(int(val))<<"\n";
    if(mode == 3) {
        attr = val;
        //std::cout<<bg_col[attr>>4]<<fg_col[attr & 0x0f];
        mode = 0;
    }
    else if(mode == 41) {
        row = val;
        mode = 42;
    }
    else if(mode == 42) {
        col = val;
        mode = 0;
    }
    else {
        switch(val) {
            case 1: //suspend emulation
                break;
            case 2: //clear console
                //std::cout<<bg_col[attr>>4]<<fg_col[attr & 0x0f];
                for(int r = 0; r < 25; r++) {
                    for(int c = 0; c< 80; c++) {
                        buffer[r][c].first = ' ';
                        buffer[r][c].second = attr;
                    }
                }
                top_row = 0;
                render();
                break;
            case 3: //set attribute byte (read a new byte)
                mode = 3;
                break;
            case 4: //move cursor (read row byte, then column byte)
                mode = 41;
                break;
            default: //unknown
                break;
        }
    }
}

void dbg_con::write_data(uint8_t val) {
    //std::cout<<"\ndeb data: "<<std::hex<<(int(val))<<"\n";
    if(val >= 0x20 && val < 127) {
        //print char
        buffer[row][col].first = val;
        buffer[row][col].second = attr;
        if(col == 79) {
            col = 0;
            row++;
        }
        else {
            col++;
        }
        if(row == 25) {
            row = 0;
            top_row++;
            top_row%=25;
        }
    }
    else if(val == 10) {
        //line-feed; cursor to beginning of line, row moves down.
        col = 0;
        row++;
        if(row == 25) {
            top_row++;
            top_row %= 25;
            row = 0;
        }
    }
    else if( val == 13) {
        //carriage return; cursor to beginning of line
        col = 0;
    }
    render();
}

void dbg_con::render() {
    std::cout<<"\n/";
    for(int c = 0; c < 80; c++) {
        std::cout<<"-";
    }
    std::cout<<"\\\n";
    for(int r = 0; r < 25; r++) {
        std::cout<<"|";
        for(int c = 0; c < 80; c++) {
            int cur_row = (r + top_row) % 25;
            //std::cout<<bg_col[buffer[cur_row][c].second>>4];
            //std::cout<<(fg_col[buffer[cur_row][c].second & 0x0f])
            std::cout<<buffer[cur_row][c].first;
        }
        std::cout<<"|\n";
    }
    std::cout<<"\\";
    for(int c = 0; c < 80; c++) {
        std::cout<<"-";
    }
    std::cout<<"/\n";

}

const std::string dbg_con::bg_col[16] = {
        "\033[40",
        "\033[44",
        "\033[42",
        "\033[46",
        "\033[41",
        "\033[45",
        "\033[43",
        "\033[40",
        "\033[47",
        "\033[44",
        "\033[42",
        "\033[46",
        "\033[41",
        "\033[45",
        "\033[43",
        "\033[47"
};

const std::string dbg_con::fg_col[16] = {
        "\033[30",
        "\033[34",
        "\033[32",
        "\033[36",
        "\033[31",
        "\033[35",
        "\033[33",
        "\033[1;30",
        "\033[37",
        "\033[1;34",
        "\033[1;32",
        "\033[1;36",
        "\033[1;31",
        "\033[1;35",
        "\033[1;33",
        "\033[1;37"
};

uint8_t dbg_con::mode = 0;
uint8_t dbg_con::col = 0;
uint8_t dbg_con::row = 0;
uint8_t dbg_con::attr = 0x0f;
uint8_t dbg_con::top_row = 0;
std::array<std::array<std::pair<char, unsigned char>, 80>, 25> dbg_con::buffer;
