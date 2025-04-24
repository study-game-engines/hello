#include "Audio.h"
#include "UniqueID.h"
#include <algorithm>

namespace Audio {
    std::unordered_map<std::string, FMOD::Sound*> g_loadedAudio;
    std::unordered_map<uint64_t, AudioHandle> g_playingAudio;
    constexpr int AUDIO_CHANNEL_COUNT = 512;
    FMOD::System* g_system;

    void Init() {
        // Create the main system object
        FMOD_RESULT result = FMOD::System_Create(&g_system);
        if (result != FMOD_OK) {
            std::cerr << "FMOD: Failed to create system object: " << FMOD_ErrorString(result) << "\n";
            g_system = nullptr;
            return;
        }

        unsigned int dspBufferLength = 512;
        int dspNumBuffers = 4;

        //std::cout << "FMOD: Setting DSP buffer size to " << dspBufferLength << " samples * " << dspNumBuffers << " buffers\n";
        result = g_system->setDSPBufferSize(dspBufferLength, dspNumBuffers);
        if (result != FMOD_OK) {
            std::cerr << "FMOD: Warning - Failed to set custom DSP buffer size: " << FMOD_ErrorString(result) << "\n";
        }
  
        result = g_system->init(AUDIO_CHANNEL_COUNT, FMOD_INIT_NORMAL, nullptr);
        if (result != FMOD_OK) {
            std::cerr << "FMOD: Failed to initialize system object: " << FMOD_ErrorString(result) << "\n";
            // Clean up system if init fails
            if (g_system) {
                g_system->release();
                g_system = nullptr;
            }
            return;
        }

        //std::cout << "FMOD: System initialized successfully.\n";

        // Create the channel group.
        FMOD::ChannelGroup* channelGroup = nullptr; // Should this be stored globally or returned?
        result = g_system->createChannelGroup("inGameSoundEffects", &channelGroup);
        if (result != FMOD_OK) {
            // Log the error, but initialization itself was successful.
            std::cerr << "FMOD: Warning - Failed to create in-game sound effects channel group: " << FMOD_ErrorString(result) << "\n";
        }
    }

    void LoadAudio(const std::string& filename) {
        FMOD_MODE eMode = FMOD_DEFAULT;
        FMOD::Sound* sound = nullptr;
        g_system->createSound(("res/audio/" + filename).c_str(), eMode, nullptr, &sound);
        g_loadedAudio[filename] = sound;
    }

    void FMOD_ERRCHECK(FMOD_RESULT result) {
        if (result != FMOD_OK) {
            std::cout << "FMOD error! (" << result << ") " << FMOD_ErrorString(result) << "\n";
        }
    }

    void Update(float deltaTime) {
        auto it = g_playingAudio.begin();
        while (it != g_playingAudio.end()) {
            uint64_t audioId = it->first;
            AudioHandle& handle = it->second;

            // Fade out any fading out audio
            if (handle.state == AudioHandle::State::FADING_OUT) {
                handle.fadeOutTimeRemaining -= deltaTime;
                float t = std::clamp(handle.fadeOutTimeRemaining / handle.fadeOutDuration, 0.0f, 1.0f);
                float fadedVolume = GetVolume(audioId) * powf(0.001f, 1.0f - t);
                SetVolume(audioId, fadedVolume);

                if (handle.fadeOutTimeRemaining <= 0.0f) {
                    handle.channel->stop();
                }
            }

            // Remove completed audio
            if (handle.channel) {
                bool isPlaying = false;
                FMOD_RESULT result = handle.channel->isPlaying(&isPlaying);
                if (result == FMOD_OK && !isPlaying) {
                    it = g_playingAudio.erase(it);
                }
                else if (result == FMOD_ERR_INVALID_HANDLE || result == FMOD_ERR_CHANNEL_STOLEN) {
                    it = g_playingAudio.erase(it);
                }
                else if (result != FMOD_OK) {
                    FMOD_ERRCHECK(result);
                    it = g_playingAudio.erase(it);
                }
                else {
                    it++;
                }
            }
            else {
                it = g_playingAudio.erase(it);
            }
        }

        // Update FMOD internal hive mind
        g_system->update();
    }

    uint64_t PlayAudio(const std::string& filename, float volume, float frequency) {
        // Load if needed
        if (g_loadedAudio.find(filename) == g_loadedAudio.end()) {  // i think you are removing all these per frame
            LoadAudio(filename);
        }

        uint64_t uniqueId = UniqueID::GetNext();
        AudioHandle& handle = g_playingAudio[uniqueId];
        handle.state = AudioHandle::State::PLAYING;
        handle.sound = g_loadedAudio[filename];
        handle.filename = filename;
        
        FMOD_RESULT result = g_system->playSound(handle.sound, nullptr, false, &handle.channel);
        FMOD_ERRCHECK(result);

        if (result == FMOD_OK && handle.channel) {
            handle.channel->setVolume(volume);
            float currentFrequency = 0.0f;
            // Consider adding error checks for get/setFrequency too if needed
            result = handle.channel->getFrequency(&currentFrequency);
            FMOD_ERRCHECK(result);
            result = handle.channel->setFrequency(currentFrequency * frequency);
            FMOD_ERRCHECK(result);
        }
        else {
            std::cerr << "FMOD Error: Failed to play sound " << filename << " or got null channel.\n";
            // Cleanup the handle created if playback failed
            g_playingAudio.erase(uniqueId);
            return 0;
        }

        return uniqueId;
    }


    uint64_t LoopAudioIfNotPlaying(const std::string& filename, float volume) {
        // Check if the is already playing
        for (const auto& pair : g_playingAudio) {
            const AudioHandle& handle = pair.second;
            if (handle.filename == filename && handle.state != AudioHandle::State::LOOPING) {
                return 0;
            }
        }

        // If not, play it
        return LoopAudio(filename, volume);
    }

    uint64_t LoopAudio(const std::string& filename, float volume) {
        // Load if needed
        if (g_loadedAudio.find(filename) == g_loadedAudio.end()) {
            LoadAudio(filename);
        }
        uint64_t uniqueId = UniqueID::GetNext();
        AudioHandle& handle = g_playingAudio[uniqueId];

        handle.state = AudioHandle::State::LOOPING;
        handle.sound = g_loadedAudio[filename];
        handle.filename = filename;
        handle.channel->setMode(FMOD_LOOP_NORMAL);
        handle.sound->setMode(FMOD_LOOP_NORMAL);
        handle.sound->setLoopCount(-1);
        g_system->playSound(handle.sound, nullptr, false, &handle.channel);
        handle.channel->setVolume(volume);

        return uniqueId;
    }

    void StopAudio(const std::string& filename) {
        for (const auto& pair : g_playingAudio) {
            const AudioHandle& handle = pair.second;
            if (handle.filename == filename) {
                FMOD_RESULT result = handle.channel->stop();
                if (result != FMOD_OK) {
                    std::cerr << "FMOD error: " << FMOD_ErrorString(result) << "\n";
                }
                else {
                    //std::cout << "Sound stopped.\n";
                }
            }
        }
    }

    void StopAudio(uint64_t audioId) {
        if (AudioIsPlaying(audioId)) {
            AudioHandle& handle = g_playingAudio[audioId];
            handle.channel->stop(); 
            g_playingAudio.erase(audioId);
        }
    }

    bool AudioIsPlaying(uint64_t audioId) {
        return (g_playingAudio.find(audioId) != g_playingAudio.end());
    }

    void FadeOut(uint64_t audioId, float duration) {
        if (!AudioIsPlaying(audioId)) return;

        AudioHandle& handle = g_playingAudio[audioId];

        if (handle.state == AudioHandle::State::PLAYING) {
            handle.state = AudioHandle::State::FADING_OUT;
            handle.fadeOutDuration = duration;
            handle.fadeOutTimeRemaining = duration;
        }
    }

    void SetVolume(uint64_t audioId, float volume) {
        if (!AudioIsPlaying(audioId)) return;

        AudioHandle& handle = g_playingAudio[audioId];
        if (handle.channel) {
            handle.channel->setVolume(volume);
        }
    }

    float GetVolume(uint64_t audioId) {
        float result = 0.0f;
        if (!AudioIsPlaying(audioId)) return result;

        AudioHandle& handle = g_playingAudio[audioId];
        if (handle.channel) {
            handle.channel->getVolume(&result);
        }

        return result;
    }
}