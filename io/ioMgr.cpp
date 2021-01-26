#include "ioMgr.h"
#include<tuple>
#include<iostream>

    sdlWindow::sdlWindow(unsigned int width, unsigned int height, std::string t) : screen(nullptr), renderer(nullptr), buffer(nullptr), texture(nullptr), overlay(nullptr), title(t), xres(width), yres(height)
    {
        std::cerr<<"Creating window \""<<title<<"\" at res "<<xres<<" x "<<yres<<"\n";
        resize(xres, yres);
        SDL_SetWindowTitle(screen, title.c_str());
        SDL_SetRenderDrawColor(renderer, 255,255,255,255);
        SDL_RenderFillRect(renderer, nullptr);
        SDL_RenderPresent(renderer);
    }

    sdlWindow::~sdlWindow() {
        std::cerr<<"Destroying window \""<<title<<"\" at res "<<xres<<" x "<<yres<<"\n";
        if(screen) {
            SDL_DestroyWindow(screen);
            screen = nullptr;
        }
        if(renderer) {
            SDL_DestroyRenderer(renderer);
            renderer = nullptr;
        }
        if(texture) {
            SDL_DestroyTexture(texture);
            texture = nullptr;
        }
        if(buffer) {
            SDL_FreeSurface(buffer);
            buffer = nullptr;
        }
        if(overlay) {
            SDL_FreeSurface(overlay);
            overlay = nullptr;
        }
    }

    void sdlWindow::resize(unsigned int width, unsigned int height) {
        std::cerr<<"Resize window \""<<title<<"\" to "<<width<<" x "<<height<<"\n";
        if(screen) {
            SDL_DestroyWindow(screen);
            screen = NULL;
        }
        if(renderer) {
            SDL_DestroyRenderer(renderer);
            renderer = NULL;
        }
        if(texture) {
            SDL_DestroyTexture(texture);
            texture = NULL;
        }

        /* Initialize the SDL library */
        screen = SDL_CreateWindow(title.c_str(),
                                SDL_WINDOWPOS_CENTERED,
                                SDL_WINDOWPOS_CENTERED,
                                width*2, height*2,
                                SDL_WINDOW_RESIZABLE);
        if ( !screen ) {
            fprintf(stderr, "ioMgr::Couldn't set %dx%dx32 video mode: %s\nStarting without video output.\n", width*2, height*2, SDL_GetError());
        }

        SDL_SetWindowMinimumSize(screen, width, height);

        renderer = SDL_CreateRenderer(screen, -1, SDL_RENDERER_ACCELERATED/*|SDL_RENDERER_PRESENTVSYNC*/);
        //renderer = SDL_CreateRenderer(screen, -1, SDL_RENDERER_ACCELERATED|SDL_RENDERER_PRESENTVSYNC);
        //renderer = SDL_CreateRenderer(screen, -1, SDL_RENDERER_SOFTWARE|SDL_RENDERER_PRESENTVSYNC);
        //renderer = SDL_CreateRenderer(screen, -1, SDL_RENDERER_SOFTWARE/*|SDL_RENDERER_PRESENTVSYNC*/);
        SDL_SetRenderDrawBlendMode(renderer,SDL_BLENDMODE_BLEND);
        if(!renderer) {
            fprintf(stderr, "ioMgr::Couldn't create a renderer: %s\nStarting without video output.\n", SDL_GetError());
            SDL_DestroyWindow(screen);
            screen = NULL;
        }

        texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888,SDL_TEXTUREACCESS_STREAMING,width,height);
        if(!texture) {
            fprintf(stderr, "ioMgr::Couldn't create a texture: %s\nStarting without video output.\n", SDL_GetError());
            SDL_DestroyRenderer(renderer);
            renderer = NULL;
            SDL_DestroyWindow(screen);
            screen = NULL;
        }
    }

    void sdlWindow::updateWindow(int startx, int starty, const std::vector<std::vector<uint8_t>>& image) {
        int width = image[0].size() / 3;
        int height = image.size();
        std::vector<uint8_t> out_buf(height * width * 4, 0xff);
        for(int line = 0; line < height; line++) {
            for(int pixel = 0; pixel < width; pixel++) {
                out_buf[4 * (line * width + pixel) + 0] = image[line][pixel * 3];
                out_buf[4 * (line * width + pixel) + 1] = image[line][pixel * 3 + 1];
                out_buf[4 * (line * width + pixel) + 2] = image[line][pixel * 3 + 2];
            }
        }
        if(texture) {
            SDL_UpdateTexture(texture, NULL, out_buf.data(), 256 * 4);
            SDL_RenderCopy(renderer, texture, NULL, NULL);
            SDL_RenderPresent(renderer);
        }

    }

    void sdlWindow::updateOverlay(int startx, int starty, const std::vector<std::vector<uint8_t>>& image) {}

ioMgr::ioMgr(std::shared_ptr<config> cfg) {
    Uint32 sdl_init_flags = SDL_INIT_EVERYTHING|SDL_INIT_JOYSTICK|SDL_INIT_GAMECONTROLLER;
    //if(headless) sdl_init_flags &= (~SDL_INIT_VIDEO);
    //if(!audio)   sdl_init_flags &= (~SDL_INIT_AUDIO);
    if(SDL_Init(sdl_init_flags) < 0 ) {
        fprintf(stderr,"Couldn't initialize SDL: %s\n--nosound would disable audio, --headless would disable video. Those might be worth a try, depending on the above error message.\n", SDL_GetError());
    }
    auto res = cfg->getResolution();

    window.emplace_back(res.first, res.second, "Khedgehog Main Window");
}

unsigned int ioMgr::createWindow(unsigned int xres, unsigned int yres, std::string title) {
    window.emplace_back(xres, yres, title);
    return window.size() - 1;
}

bool ioMgr::updateWindow(unsigned int winIndex, int startx, int starty, const std::vector<std::vector<uint8_t>>& image) {
    window[winIndex].updateWindow(startx, starty, image);
    return true;
}

ioEvent ioMgr::getEvent() {
    SDL_Event event;
    while(SDL_PollEvent(&event)) {
        if(event.type == SDL_KEYDOWN) { /* Handle a KEYDOWN event */
            switch(event.key.keysym.scancode) {
                case SDL_SCANCODE_Q:
                    SDL_Quit();
                    return ioEvent{ioEvent::eventType::window, ioEvent::windowEvent::exit};
                case SDL_SCANCODE_A:
                    return ioEvent{ioEvent::eventType::smsKey, ioEvent::smsKey::dpad_left, ioEvent::keyState::keydown};
                case SDL_SCANCODE_S:
                    return ioEvent{ioEvent::eventType::smsKey, ioEvent::smsKey::dpad_down, ioEvent::keyState::keydown};
                case SDL_SCANCODE_D:
                    return ioEvent{ioEvent::eventType::smsKey, ioEvent::smsKey::dpad_right, ioEvent::keyState::keydown};
                case SDL_SCANCODE_W:
                    return ioEvent{ioEvent::eventType::smsKey, ioEvent::smsKey::dpad_up, ioEvent::keyState::keydown};
                case SDL_SCANCODE_H:
                    return ioEvent{ioEvent::eventType::smsKey, ioEvent::smsKey::button_pause, ioEvent::keyState::keydown};
                case SDL_SCANCODE_K:
                    return ioEvent{ioEvent::eventType::smsKey, ioEvent::smsKey::button_1, ioEvent::keyState::keydown};
                case SDL_SCANCODE_L:
                    return ioEvent{ioEvent::eventType::smsKey, ioEvent::smsKey::button_2, ioEvent::keyState::keydown};
                default:
                    break;
            }
        }
        else if(event.type == SDL_KEYUP) { /* Handle a KEYUP event*/
            switch(event.key.keysym.scancode) {
                case SDL_SCANCODE_A:
                    return ioEvent{ioEvent::eventType::smsKey, ioEvent::smsKey::dpad_left, ioEvent::keyState::keyup};
                case SDL_SCANCODE_S:
                    return ioEvent{ioEvent::eventType::smsKey, ioEvent::smsKey::dpad_down, ioEvent::keyState::keyup};
                case SDL_SCANCODE_D:
                    return ioEvent{ioEvent::eventType::smsKey, ioEvent::smsKey::dpad_right, ioEvent::keyState::keyup};
                case SDL_SCANCODE_W:
                    return ioEvent{ioEvent::eventType::smsKey, ioEvent::smsKey::dpad_up, ioEvent::keyState::keyup};
                case SDL_SCANCODE_H:
                    return ioEvent{ioEvent::eventType::smsKey, ioEvent::smsKey::button_pause, ioEvent::keyState::keyup};
                case SDL_SCANCODE_K:
                    return ioEvent{ioEvent::eventType::smsKey, ioEvent::smsKey::button_1, ioEvent::keyState::keyup};
                case SDL_SCANCODE_L:
                    return ioEvent{ioEvent::eventType::smsKey, ioEvent::smsKey::button_2, ioEvent::keyState::keyup};
                default:
                    break;
            }
        }
        else if(event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE) {
            SDL_Quit();
            return ioEvent(ioEvent::eventType::window, ioEvent::windowEvent::exit);
        }
        else if(event.type == SDL_QUIT) {
                SDL_Quit();
                return ioEvent(ioEvent::eventType::window, ioEvent::windowEvent::exit);
        }
    }
    return ioEvent(ioEvent::eventType::none);
}

bool ioMgr::resizeWindow(unsigned int winIndex, unsigned int xres, unsigned int yres) {
    window[winIndex].resize(xres, yres);
    return true;
}
