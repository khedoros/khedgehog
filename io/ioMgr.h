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
    void updateWindow(int startx, int starty, int stride, const std::vector<uint8_t>& image);
    void resize(unsigned int xres, unsigned int yres);

private:
    std::string title;
    int xres, yres;
    int windowId;
    SDL_Window * window;
    SDL_Renderer * renderer;
    SDL_Texture * texture; //Texture used for output
};

class ioMgr {
public:
    ioMgr(std::shared_ptr<config> cfg);
    unsigned int createWindow(unsigned int xres, unsigned int yres, std::string title);
    bool resizeWindow(unsigned int winIndex, unsigned int xres, unsigned int yres);
    bool updateWindow(unsigned int winIndex, int startx, int starty, int stride, const std::vector<uint8_t>& image); // need to decide how I want to represent the image. Maybe std::vector<std::vector<uint8_t>>?
    bool closeWindow(unsigned int winIndex);
    ioEvent getEvent();
    void pushAudio(std::array<int16_t, 882 * 2>& samples);

private:
	std::shared_ptr<config> cfg;
    std::vector<sdlWindow> windowList;

    SDL_AudioDeviceID audioDev;
    SDL_AudioSpec audioSpec;
	size_t sampleCnt;
};
