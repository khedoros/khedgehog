#pragma once

class ioEvent {
    enum class smsKey {
        dpad_up,
        dpad_down,
        dpad_left,
        dpad_right,
        button_1,
        button_2,
        button_pause
    };

    enum class genesisKey {
        dpad_up,
        dpad_down,
        dpad_left,
        dpad_right,
        button_a,
        button_b,
        button_c,
        button_x,
        button_y,
        button_z,
        button_start,
        button_mode
    };

    enum class keyState {
        keydown,
        keyup
    };

    union {
        smsKey sKey;
        genesisKey gKey;
    };
    keyState state;
};
