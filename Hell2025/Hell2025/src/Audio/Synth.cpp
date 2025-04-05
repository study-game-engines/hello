#include "Synth.h"
#include "fluidsynth.h"
#include "Input/Input.h"
#include "Input/InputMulti.h"

#include <iostream>
#include <thread>
#include <chrono>

#include <dinput.h>
#include <iostream>
#include <thread>
#include <mutex>
#include <chrono>
#include <Windows.h>
#include <dinput.h>

extern IDirectInputDevice8* g_pKeyboard;

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
        fluid_settings_setnum(g_settings, "synth.sample-rate", 44100); // Or 48000

        // --- Low Latency Settings (Based on YOUR available settings) ---

        fluid_settings_setint(g_settings, "audio.wasapi.exclusive-mode", 1);

          // 2. Set Buffer Size using period-size (INCREASED VALUE)
          // Try larger values than before. Start with 96 or 128.
          // If still getting warnings, try 192 or 256.
        fluid_settings_setint(g_settings, "audio.period-size", 128); // EXAMPLE: Increased from 64

        // 3. Set Number of Periods (Usually 2 is fine, can try 3)
        // Increasing this slightly increases latency but adds more buffering.
        fluid_settings_setint(g_settings, "audio.periods", 2); // Can also try 3 if needed

        // Other settings (driver, sample rate, effects off) remain the same...
        fluid_settings_setstr(g_settings, "audio.driver", "wasapi");
        fluid_settings_setnum(g_settings, "synth.sample-rate", 44100);
        fluid_settings_setint(g_settings, "synth.reverb.active", 0);
        fluid_settings_setint(g_settings, "synth.chorus.active", 0);


        fluid_settings_setint(g_settings, "synth.verbose", 0);


        fluid_settings_foreach(g_settings, nullptr, PrintSettings);

        //fluid_settings_setnum(g_settings, "audio.period-size", 16);
        //fluid_settings_setnum(g_settings, "audio.periods", 2);
        //fluid_settings_setstr(g_settings, "audio.driver", "wasapi");
        //fluid_settings_setstr(g_settings, "audio.wasapi.exclusive", "yes"); 
        //fluid_settings_setnum(g_settings, "synth.sample-rate", 44100);
        //fluid_settings_setstr(g_settings, "audio.driver", "wasapi");
        //fluid_settings_setnum(g_settings, "synth.sample-rate", 44100);


        // Load soundfont
        int sfid = fluid_synth_sfload(g_synth, "res/audio/piano/YamahaGrandLiteV2.sf2", 1);
        if (sfid == FLUID_FAILED) {
            std::cerr << "Error: Could not load soundfont!\n";
            return;
        }
    }

    void Update(float deltaTime) {
        // Keys range from #21 (A0) to #108 (C8)

        int velocity = 127;
        fluid_synth_set_gain(g_synth, 1.0);

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

    #pragma comment(lib, "dinput8.lib")
    #pragma comment(lib, "dxguid.lib")

    // Global mutex for console output.
    std::mutex coutMutex;

    // Global DirectInput interfaces.
    IDirectInput8* g_pDI = nullptr;
    IDirectInputDevice8* g_pKeyboard = nullptr;

    
    bool InitDirectInput(HINSTANCE hInstance, HWND hWnd)
    {
        HRESULT hr = DirectInput8Create(hInstance, DIRECTINPUT_VERSION, IID_IDirectInput8, (void**)&g_pDI, nullptr);
        if (FAILED(hr))
        {
            std::lock_guard<std::mutex> lock(coutMutex);
            std::cerr << "DirectInput8Create failed: hr = " << hr << "\n";
            return false;
        }

        hr = g_pDI->CreateDevice(GUID_SysKeyboard, &g_pKeyboard, nullptr);
        if (FAILED(hr))
        {
            std::lock_guard<std::mutex> lock(coutMutex);
            std::cerr << "CreateDevice failed: hr = " << hr << "\n";
            return false;
        }

        hr = g_pKeyboard->SetDataFormat(&c_dfDIKeyboard);
        if (FAILED(hr))
        {
            std::lock_guard<std::mutex> lock(coutMutex);
            std::cerr << "SetDataFormat failed: hr = " << hr << "\n";
            return false;
        }

        // Use DISCL_BACKGROUND to allow input when window is not focused.
        hr = g_pKeyboard->SetCooperativeLevel(hWnd, DISCL_BACKGROUND | DISCL_NONEXCLUSIVE);
        if (FAILED(hr))
        {
            std::lock_guard<std::mutex> lock(coutMutex);
            std::cerr << "SetCooperativeLevel failed: hr = " << hr << "\n";
            return false;
        }

        hr = g_pKeyboard->Acquire();
        if (FAILED(hr))
        {
            std::lock_guard<std::mutex> lock(coutMutex);
            std::cerr << "Acquire failed: hr = " << hr << "\n";
            return false;
        }

        return true;
    }

    void threadFunction() {
        if (!g_pKeyboard) {
            std::lock_guard<std::mutex> lock(coutMutex);
            std::cerr << "Error: g_pKeyboard is not initialized!\n";
            return;
        }

            BYTE keyStates[256] = { 0 };
            static BYTE prevKeyStates[256] = { 0 };


            while (true) {
                HRESULT hr = g_pKeyboard->Acquire();
                if (FAILED(hr)) {
                    std::lock_guard<std::mutex> lock(coutMutex);
                    std::cerr << "Error: Acquire failed, hr = " << hr << "\n";
                }

                hr = g_pKeyboard->GetDeviceState(sizeof(keyStates), (LPVOID)&keyStates);
                if (SUCCEEDED(hr)) {

                    if ((keyStates[DIK_A] & 0x80) && !(prevKeyStates[DIK_A] & 0x80)) {
                        std::lock_guard<std::mutex> lock(coutMutex);
                        std::cout << "A key pressed\n";

                        int velocity = 127;
                        fluid_synth_noteon(g_synth, 0, 60, velocity);
                    }
                    // Check if the "A" key was released.
                    if (!(keyStates[DIK_A] & 0x80) && (prevKeyStates[DIK_A] & 0x80)) {
                        std::lock_guard<std::mutex> lock(coutMutex);
                        std::cout << "A key released\n";
                        fluid_synth_noteoff(g_synth, 0, 60);
                    }
                    // Update previous state.
                    memcpy(prevKeyStates, keyStates, sizeof(keyStates));
                }
                else {
                    std::lock_guard<std::mutex> lock(coutMutex);
                    std::cerr << "Error: GetDeviceState failed, hr = " << hr << "\n";
                }
            }
    }


    void BeginAudioThread() {

        HWND hWnd = GetConsoleWindow();
        HINSTANCE hInstance = GetModuleHandle(NULL);

        if (!InitDirectInput(hInstance, hWnd))
        {
            std::lock_guard<std::mutex> lock(coutMutex);
            std::cerr << "DirectInput initialization failed.\n";
            return;
        }
        // Create and start the input thread.
        std::thread inputThread(threadFunction);
        inputThread.detach();

    }
}