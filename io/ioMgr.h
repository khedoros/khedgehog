#pragma once
#include<vector>
#include<cstdint>
#include<SDL2/SDL.h>

#include "ioEvent.h"
#include "../config.h"

class ioMgr;

class ioMgr {
public:
    ioMgr(std::shared_ptr<config> cfg);
    bool reinitWindow(unsigned int xres, unsigned int yres);
    bool updateWindow(int startx, int starty, const std::vector<std::vector<uint8_t>>& image); // need to decide how I want to represent the image. Maybe std::vector<std::vector<uint8_t>>?
    ioEvent getEvent();

private:
    int xres, yres;
    SDL_Window * screen;
    SDL_Renderer * renderer;

    SDL_Surface * buffer; //Output buffer
    SDL_Texture * texture; //Texture used for output

    SDL_Surface * overlay; //overlay surface (if I want to render non-game info)
};
