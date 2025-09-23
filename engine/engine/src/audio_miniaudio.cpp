#include "nyanchu/audio.h"
#include <iostream>

#define MINIAUDIO_IMPLEMENTATION
#include "external/miniaudio.h"

namespace nyanchu
{
    void Audio::init()
    {
        m_engine = new ma_engine();
        ma_result result;
        result = ma_engine_init(NULL, m_engine);
        if (result != MA_SUCCESS) {
            printf("Failed to initialize audio engine.");
            return;
        }
        m_bgm = nullptr;
    }

    void Audio::shutdown()
    {
        if (m_bgm) {
            ma_sound_uninit(m_bgm);
            delete m_bgm;
            m_bgm = nullptr;
        }
        ma_engine_uninit(m_engine);
        delete m_engine;
        m_engine = nullptr;
    }

    void Audio::play_bgm(const char* soundName)
    {
        if (m_bgm) {
            // Stop and uninitialize existing BGM if any
            ma_sound_uninit(m_bgm);
            delete m_bgm;
            m_bgm = nullptr;
        }

        m_bgm = new ma_sound();
        ma_result result = ma_sound_init_from_file(m_engine, soundName, 0, NULL, NULL, m_bgm);
        if (result != MA_SUCCESS) {
            printf("Failed to init sound from file: %s\n", soundName);
            delete m_bgm;
            m_bgm = nullptr;
            return;
        }

        ma_sound_set_looping(m_bgm, MA_TRUE);
        ma_sound_start(m_bgm);
    }
} // namespace nyanchu