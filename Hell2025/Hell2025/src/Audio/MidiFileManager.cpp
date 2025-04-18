#include "MidiFileManager.h"
#include "MidiFile.h"
#include <iostream>

namespace MidiFileManager {

    std::vector<MidiFile> g_midiFiles;
    int g_trackIndex = 0;
    bool g_playMusic = false;

    void Init() {
        //LoadMidiFile("Czardas.mid");
        LoadMidiFile("Goat.mid");
        LoadMidiFile("Nocturne.mid");
    }

    void LoadMidiFile(const std::string& filename) {
        if (MidiFile midiFile; midiFile.LoadFromFile(filename)) {
            g_midiFiles.push_back(std::move(midiFile));
        }
    }

    void Update(float deltaTime) {

        MidiFile& midiFile = g_midiFiles[g_trackIndex];

        if (Input::KeyPressed(HELL_KEY_U)) {
            midiFile.Play();
            g_playMusic = true;
        }
        if (Input::KeyPressed(HELL_KEY_O)) {
            g_trackIndex++;
            g_trackIndex = g_trackIndex % g_midiFiles.size();
            g_midiFiles[g_trackIndex].Play();
            return;
        }

        if (g_playMusic) {

            if (Input::KeyDown(HELL_KEY_I)) {
                midiFile.Update(deltaTime * 10);
            }

            if (midiFile.IsComplete()) {
                midiFile.Stop();
                g_trackIndex++;
                g_trackIndex = g_trackIndex % g_midiFiles.size();
                g_midiFiles[g_trackIndex].Play();
            }
            else {
                midiFile.Update(deltaTime);
            }
        }

        //std::cout << g_trackIndex << " " << midiFile.GetPlaybackTime() << "/" << midiFile.GetDuration() << " " << g_transitioning << " " << g_transitionTimer << "\n";
    }
}