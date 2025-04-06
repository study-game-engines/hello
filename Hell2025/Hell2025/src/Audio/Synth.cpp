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
        
        fluid_synth_set_gain(g_synth, 2.0);
    }

    void PlayNote(int note) {        
        int velocity = 127;
        fluid_synth_noteon(g_synth, 0, note, velocity);
    }

    void ReleaseNote(int note) {
        fluid_synth_noteoff(g_synth, 0, note);
    }

    void Update(float deltaTime) {
       
    }

    void CleanUp() {
        delete_fluid_audio_driver(g_driver);
        delete_fluid_synth(g_synth);
        delete_fluid_settings(g_settings);
    }
}