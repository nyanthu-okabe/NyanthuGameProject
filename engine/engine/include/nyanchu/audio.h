#pragma once

// Foward declaration
typedef struct ma_engine ma_engine;
typedef struct ma_sound ma_sound;

namespace nyanchu {

class Audio {
public:
    void init();
    void shutdown();
    void play_bgm(const char* soundName);
private:
    ma_engine* m_engine;
    ma_sound* m_bgm;
};

} // namespace nyanchu
