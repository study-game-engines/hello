#pragma once

namespace Synth {
    void Init();
    void Update(float deltaTime);
    void PlayNote(int note);
    void ReleaseNote(int note);
}