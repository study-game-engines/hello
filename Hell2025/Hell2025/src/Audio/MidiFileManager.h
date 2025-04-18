#pragma once
#include <string>
#include <vector>

#include "Input/Input.h"

namespace MidiFileManager {
    void Init();
    void LoadMidiFile(const std::string& filename);
    void Update(float deltaTime);
}