#pragma once

class ioEvent {
    public:
    enum class eventType {
        none,
        smsKey,
        genKey,
        window
    };
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

    enum class windowEvent {
        pause,
        mute,
        exit
    };

    enum class keyState {
        keydown,
        keyup
    };

    eventType type;
    union {
        smsKey sKey;
        genesisKey gKey;
        windowEvent winEvent;
    } key;
    keyState state;

    ioEvent(eventType t) {
        type = t;
    }
    ioEvent(eventType t, smsKey k, keyState s) {
        type = t;
        key.sKey = k;
        state = s;
    }
    ioEvent(eventType t, windowEvent e, keyState s = keyState::keydown) {
        type = t;
        key.winEvent = e;
        state = s;
    }
    ioEvent(eventType t, genesisKey k, keyState s) {
        type = t;
        key.gKey = k;
        state = s;
    }
};
