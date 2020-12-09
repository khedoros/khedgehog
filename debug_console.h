#pragma once
#include<cstdint>
#include<string>
#include<tuple>

class dbg_con {
    public:
    static void init();
    static void reset();
    static void write_control(uint8_t val);
    static void write_data(uint8_t val);
    static void render();
    private:
    static uint8_t row;
    static uint8_t col;
    static uint8_t attr;
    static uint8_t top_row;
    static const std::string bg_col[16];
    static const std::string fg_col[16];
    static uint8_t mode;
    static std::array<std::array<std::pair<char,unsigned char>, 80>, 25> buffer;
};
