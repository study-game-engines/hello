#include "Synth.h"
#include "fluidsynth.h"
#include "Input/Input.h"
#include "Input/InputMulti.h"
#include <iostream>

void PrintSettings(void* data, const char* name, int type) {
    std::cout << "Setting: Name='" << name << "', Type=" << type << "\n";
}

namespace Synth {
    fluid_audio_driver_t* g_driver;
    fluid_settings_t* g_settings;
    fluid_synth_t* g_synth;
    
    void Init() {
        g_settings = new_fluid_settings();
        g_synth = new_fluid_synth(g_settings);
        g_driver = new_fluid_audio_driver(g_settings, g_synth);

        fluid_settings_setstr(g_settings, "audio.driver", "wasapi");
        fluid_settings_setnum(g_settings, "synth.sample-rate", 44100);
        fluid_settings_setint(g_settings, "audio.wasapi.exclusive-mode", 1);
        fluid_settings_setint(g_settings, "audio.period-size", 128);
        fluid_settings_setint(g_settings, "audio.periods", 2);
        fluid_settings_setstr(g_settings, "audio.driver", "wasapi");
        fluid_settings_setnum(g_settings, "synth.sample-rate", 44100);
        fluid_settings_setint(g_settings, "synth.reverb.active", 0);
        fluid_settings_setint(g_settings, "synth.chorus.active", 0);
        fluid_settings_setint(g_settings, "synth.verbose", 0);

        //fluid_settings_foreach(g_settings, nullptr, PrintSettings);

        // Load sound font
        int sfid = fluid_synth_sfload(g_synth, "res/audio/piano/YamahaGrandLiteV2.sf2", 1);
        if (sfid == FLUID_FAILED) {
            return;
        }
    }

    void PlayNote(int note) {
        fluid_synth_set_gain(g_synth, 2.0);
        std::cout << "Playing note " << note << "\n";

        int velocity = 127;
        fluid_synth_noteon(g_synth, 0, note, velocity);
    }

    void ReleaseNote(int note) {
        fluid_synth_noteoff(g_synth, 0, note);
    }

    void Update(float deltaTime) {
        return;

        // Keys range from #21 (A0) to #108 (C8)

        int velocity = 127;
        fluid_synth_set_gain(g_synth, 2.0);

        if (InputMulti::KeyPressed(0, 0, HELL_KEY_C)) {
            std::cout << "b \n";
            fluid_synth_noteon(g_synth, 0, 60, velocity);
        }
        if (!InputMulti::KeyDown(0, 0, HELL_KEY_C)) {
            fluid_synth_noteoff(g_synth, 0, 60);
        }

        if (InputMulti::KeyPressed(0, 0, HELL_KEY_V)) {
            std::cout << "n \n";
            fluid_synth_noteon(g_synth, 0, 62, velocity);
        }
        if (!InputMulti::KeyDown(0, 0, HELL_KEY_V)) {
            fluid_synth_noteoff(g_synth, 0, 62);
        }

        if (InputMulti::KeyPressed(0, 0, HELL_KEY_B)) {
            std::cout << "n \n";
            fluid_synth_noteon(g_synth, 0, 64, velocity);
        }
        if (!InputMulti::KeyDown(0, 0, HELL_KEY_B)) {
            fluid_synth_noteoff(g_synth, 0, 64);
        }

        if (InputMulti::KeyPressed(0, 0, HELL_KEY_N)) {
            std::cout << "m \n";
            fluid_synth_noteon(g_synth, 0, 65, velocity);
        }

        if (!InputMulti::KeyDown(0, 0, HELL_KEY_N)) {
            fluid_synth_noteoff(g_synth, 0, 65);
        }

        if (InputMulti::KeyPressed(0, 0, HELL_KEY_M)) {
            std::cout << "b \n";
            fluid_synth_noteon(g_synth, 0, 67, velocity);
        }
        if (!InputMulti::KeyDown(0, 0, HELL_KEY_M)) {
            fluid_synth_noteoff(g_synth, 0, 67);
        }

    }

    void CleanUp() {
        delete_fluid_audio_driver(g_driver);
        delete_fluid_synth(g_synth);
        delete_fluid_settings(g_settings);
    }
}