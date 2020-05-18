#pragma once

class config {
public:
    config(int argc, char **argv, char *config_file = nullptr);
private:
    constexpr static const char* default_cfg_file = "khedgehog.ini";
};
