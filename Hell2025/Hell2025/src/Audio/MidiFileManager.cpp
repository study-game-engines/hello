#include "MidiFileManager.h"
#include "MidiFile.h"
#include <iostream>
namespace MidiFileManager {

    std::vector<MidiFile> g_midiFiles;
    int g_trackIndex = 0;
    bool g_playMusic = false;

    std::vector<std::string> g_debugTextTime;
    std::vector<std::string> g_debugTextEvents;
    std::vector<std::string> g_debugTextTimeDurations;
    std::vector<std::string> g_debugTextTimeVelocities;

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

        if (Input::KeyPressed(HELL_KEY_SCROLL_LOCK)) {
            midiFile.Play();
            g_playMusic = true;
        }
        if (Input::KeyPressed(HELL_KEY_END)) {
            g_trackIndex++;
            g_trackIndex = g_trackIndex % g_midiFiles.size();
            g_midiFiles[g_trackIndex].Play();
            return;
        }

        if (g_playMusic) {

            //if (Input::KeyDown(HELL_KEY_I)) {
            //    midiFile.Update(deltaTime * 10);
            //}

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
        else {
            g_debugTextTime.clear();
            g_debugTextEvents.clear();
            g_debugTextTimeDurations.clear();
            g_debugTextTimeVelocities.clear();
        }

        //std::cout << g_trackIndex << " " << midiFile.GetPlaybackTime() << "/" << midiFile.GetDuration() << " " << g_transitioning << " " << g_transitionTimer << "\n";
    }

    void AddDebugTextTimes(const std::string& text) {
        g_debugTextTime.push_back(text);
        while (g_debugTextTime.size() > 10) {
            g_debugTextTime.erase(g_debugTextTime.begin());
        }
    }

    void AddDebugTextEvent(const std::string& text) {
        g_debugTextEvents.push_back(text);
        while (g_debugTextEvents.size() > 10) {
            g_debugTextEvents.erase(g_debugTextEvents.begin());
        }
    }
    void AddDebugTextDurations(const std::string& text) {
        g_debugTextTimeDurations.push_back(text);
        while (g_debugTextTimeDurations.size() > 10) {
            g_debugTextTimeDurations.erase(g_debugTextTimeDurations.begin());
        }
    }

    void AddDebugTextVelocity(const std::string& text) {
        g_debugTextTimeVelocities.push_back(text);
        while (g_debugTextTimeVelocities.size() > 10) {
            g_debugTextTimeVelocities.erase(g_debugTextTimeVelocities.begin());
        }
    }

    std::string GetDebugTextTime() {
        std::string result;
        for (const std::string& text : g_debugTextTime) {
            result += text + "\n";
        }
        return result;
    }

    std::string GetDebugTextEvents() {
        std::string result;
        for (const std::string& text : g_debugTextEvents) {
            result += text + "\n";
        }
        return result;
    }

    std::string GetDebugTextTimeDurations() {
        std::string result;
        for (const std::string& text : g_debugTextTimeDurations) {
            result += text + "\n";
        }
        return result;
    }

    std::string GetDebugTextVelocity() {
        std::string result;
        for (const std::string& text : g_debugTextTimeVelocities) {
            result += text + "\n";
        }
        return result;
    }
}