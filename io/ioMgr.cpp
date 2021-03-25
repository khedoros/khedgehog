#include "ioMgr.h"
#include<tuple>
#include<iostream>

sdlWindow::sdlWindow() {}

sdlWindow::sdlWindow(unsigned int width, unsigned int height, std::string t) : window(nullptr), renderer(nullptr), texture(nullptr), title(t), xres(width), yres(height)
{
    std::cerr<<"Creating window \""<<title<<"\" at res "<<xres<<" x "<<yres<<"\n";
    resize(xres, yres);
    SDL_SetWindowTitle(window, title.c_str());
    SDL_SetRenderDrawColor(renderer, 255,255,255,255);
    SDL_RenderFillRect(renderer, nullptr);
    SDL_RenderPresent(renderer);
    windowId = SDL_GetWindowID(window);
}

sdlWindow::~sdlWindow() {
    std::cerr<<"Destroying window \""<<title<<"\" at res "<<xres<<" x "<<yres<<"\n";
    if(window) {
        SDL_DestroyWindow(window);
        window = nullptr;
    }
    if(renderer) {
        SDL_DestroyRenderer(renderer);
        renderer = nullptr;
    }
    if(texture) {
        SDL_DestroyTexture(texture);
        texture = nullptr;
    }
}

void sdlWindow::resize(unsigned int width, unsigned int height) {
    std::cerr<<"Resize window \""<<title<<"\" to "<<width<<" x "<<height<<"\n";
    if(window) {
        SDL_DestroyWindow(window);
        window = nullptr;
    }
    if(renderer) {
        SDL_DestroyRenderer(renderer);
        renderer = nullptr;
    }
    if(texture) {
        SDL_DestroyTexture(texture);
        texture = nullptr;
    }

    /* Initialize the SDL library */
    window = SDL_CreateWindow(title.c_str(),
            SDL_WINDOWPOS_CENTERED,
            SDL_WINDOWPOS_CENTERED,
            640, 480,
            SDL_WINDOW_RESIZABLE | SDL_WINDOW_SHOWN);
    if ( !window ) {
        fprintf(stderr, "ioMgr::Couldn't set %dx%dx32 video mode: %s\nStarting without video output.\n", 640, 480, SDL_GetError());
    }

    SDL_SetWindowMinimumSize(window, width, height);

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED/*|SDL_RENDERER_PRESENTVSYNC*/);
    //renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED|SDL_RENDERER_PRESENTVSYNC);
    //renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_SOFTWARE|SDL_RENDERER_PRESENTVSYNC);
    //renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_SOFTWARE/*|SDL_RENDERER_PRESENTVSYNC*/);
    SDL_SetRenderDrawBlendMode(renderer,SDL_BLENDMODE_BLEND);
    if(!renderer) {
        fprintf(stderr, "ioMgr::Couldn't create a renderer: %s\nStarting without video output.\n", SDL_GetError());
        SDL_DestroyWindow(window);
        window = nullptr;
    }

    texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888,SDL_TEXTUREACCESS_STREAMING,width,height);
    if(!texture) {
        fprintf(stderr, "ioMgr::Couldn't create a texture: %s\nStarting without video output.\n", SDL_GetError());
        SDL_DestroyRenderer(renderer);
        renderer = nullptr;
        SDL_DestroyWindow(window);
        window = nullptr;
    }
}

void sdlWindow::updateWindow(int startx, int starty, int stride, const std::vector<uint8_t>& image) {
    std::vector<uint8_t> out_buf((image.size() * 4) / 3, 0xff);
    int width = stride / 3;
    int height = image.size() / stride;
    for(int pixel = 0; pixel < image.size() / 3; pixel++) {
        out_buf[4 * pixel + 0] = image[pixel * 3 + 0];
        out_buf[4 * pixel + 1] = image[pixel * 3 + 1];
        out_buf[4 * pixel + 2] = image[pixel * 3 + 2];
    }
    if(width != xres || height != yres) {
        resize(width, height);
    }

    if(texture) {
        SDL_UpdateTexture(texture, nullptr, out_buf.data(), width * 4);
        SDL_RenderCopy(renderer, texture, nullptr, nullptr);
        SDL_RenderPresent(renderer);
    }

}

ioMgr::ioMgr(std::shared_ptr<config> conf) : cfg(conf) {
    windowList.reserve(4);
    Uint32 sdl_init_flags = SDL_INIT_EVERYTHING|SDL_INIT_JOYSTICK|SDL_INIT_GAMECONTROLLER;
    //if(headless) sdl_init_flags &= (~SDL_INIT_VIDEO);
    //if(!audio)   sdl_init_flags &= (~SDL_INIT_AUDIO);
    if(SDL_Init(sdl_init_flags) < 0 ) {
        fprintf(stderr,"Couldn't initialize SDL: %s\n--nosound would disable audio, --headless would disable video. Those might be worth a try, depending on the above error message.\n", SDL_GetError());
    }
    auto res = cfg->getResolution();

    createWindow(res.first, res.second, "Khedgehog Main Window");

    int channels = 1;
    if(cfg->getSystemType() == systemType::gameGear) {
        channels = 2;
        sampleCnt = 735;
    }
    else if(cfg->getSystemRegion() != systemRegion::pal) {
        sampleCnt = 735;
    }
    else {
        sampleCnt = 882;
    }
    SDL_AudioSpec want;
    SDL_memset(&want, 0, sizeof(want));
    want.freq = 44100;
    want.format = AUDIO_S16LSB;
    want.channels = channels;
    want.samples = 1024;

    audioDev = SDL_OpenAudioDevice(nullptr, 0, &want, &audioSpec, 0);

    if (audioDev == 0) {
        SDL_Log("Failed to open audio: %s", SDL_GetError());
    }
    else {
        SDL_PauseAudioDevice(audioDev, 0);
    }
}


unsigned int ioMgr::createWindow(unsigned int xres, unsigned int yres, std::string title) {
    windowList.emplace_back(xres, yres, title);
    return windowList.size() - 1;
}

bool ioMgr::updateWindow(unsigned int winIndex, int startx, int starty, int stride, const std::vector<uint8_t>& image) {
    windowList.at(winIndex).updateWindow(startx, starty, stride, image);
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
    windowList[winIndex].resize(xres, yres);
    return true;
}

void ioMgr::pushAudio(std::array<int16_t, 882 * 2>& samples) {
    uint32_t enqueuedBytes = SDL_GetQueuedAudioSize(audioDev);
    std::cout<<"audio: "<<enqueuedBytes<<" bytes of queued audio.\n";
    while(SDL_GetQueuedAudioSize(audioDev) > sampleCnt * audioSpec.channels * sizeof(int16_t)) {
        SDL_Delay(1);
    }
    SDL_QueueAudio(audioDev, samples.data(), sampleCnt * audioSpec.channels * sizeof(int16_t));
}
