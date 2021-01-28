#pragma once
#include<vector>
#include<cstdint>
#include<SDL2/SDL.h>
#include<string>

#include "ioEvent.h"
#include "../config.h"

class ioMgr;

class sdlWindow {
public:
    sdlWindow();
    sdlWindow(unsigned int xres, unsigned int yres, std::string title);
    ~sdlWindow();
    void updateWindow(int startx, int starty, const std::vector<std::vector<uint8_t>>& image);
    void updateOverlay(int startx, int starty, const std::vector<std::vector<uint8_t>>& image);
    void resize(unsigned int xres, unsigned int yres);

private:
    std::string title;
    int xres, yres;
    SDL_Window * screen;
    SDL_Renderer * renderer;

    SDL_Surface * buffer; //Output buffer
    SDL_Texture * texture; //Texture used for output

    SDL_Surface * overlay; //overlay surface (if I want to render non-game info)
};

class ioMgr {
public:
    ioMgr(std::shared_ptr<config> cfg);
    unsigned int createWindow(unsigned int xres, unsigned int yres, std::string title);
    bool resizeWindow(unsigned int winIndex, unsigned int xres, unsigned int yres);
    bool updateWindow(unsigned int winIndex, int startx, int starty, const std::vector<std::vector<uint8_t>>& image); // need to decide how I want to represent the image. Maybe std::vector<std::vector<uint8_t>>?
    bool overlayWindow(unsigned int winIndex, int startx, int starty, const std::vector<std::vector<uint8_t>>& image);
    bool closeWindow(unsigned int winIndex);
    ioEvent getEvent();

private:
    std::vector<sdlWindow> window;
};
