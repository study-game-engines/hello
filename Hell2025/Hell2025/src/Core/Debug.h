#pragma once
#include "HellEnums.h"
#include <string>

namespace Debug {
    void Update();
    void AddText(const std::string& text);
    void EndFrame();
    void ToggleDebugText();
    void NextDebugRenderMode();

    bool IsDebugTextVisible();
    const std::string& GetText();
    const DebugRenderMode& GetDebugRenderMode();
}