#pragma once

#include <miniaudio.h>

namespace nyanchu {

class Audio {
public:
    Audio();
    ~Audio();

    bool init();
    void shutdown();

    void play_bgm(const char* filename, bool loop = true);
    void stop_bgm();

private:
    ma_engine* _engine = nullptr;
    ma_sound* _bgm = nullptr;
    bool _is_initialized = false;
};

} // namespace nyanchu
