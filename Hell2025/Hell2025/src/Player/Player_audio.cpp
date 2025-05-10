#include "Player.h"
#include "Audio/Audio.h"
#include "Core/Game.h"

void Player::UpdateAudio() {

    const std::vector<const char*> indoorFootstepFilenames = {
                    "player_step_1.wav",
                    "player_step_2.wav",
                    "player_step_3.wav",
                    "player_step_4.wav",
    };
    const std::vector<const char*> outdoorFootstepFilenames = {
                    "player_step_grass_1.wav",
                    "player_step_grass_2.wav",
                    "player_step_grass_3.wav",
                    "player_step_grass_4.wav",
    };
    const std::vector<const char*> swimFootstepFilenames = {
                    "player_step_swim_1.wav",
                    "player_step_swim_2.wav",
                    "player_step_swim_3.wav",
                    "player_step_swim_4.wav",
    };
    const std::vector<const char*> wadeEndFilenames = {
                    "Water_Wade_End_1.wav",
                    "Water_Wade_End_2.wav",
                    "Water_Wade_End_3.wav",
                    "Water_Wade_End_4.wav",
                    "Water_Wade_End_5.wav",
                    "Water_Wade_End_6.wav",
                    "Water_Wade_End_7.wav",
                    "Water_Wade_End_8.wav",
                    "Water_Wade_End_9.wav",
    };
    const std::vector<const char*> ladderFootstepFilenames = {
                    "player_step_ladder_1.wav",
                    "player_step_ladder_2.wav",
                    "player_step_ladder_3.wav",
                    "player_step_ladder_4.wav",
    };

    // Water
    if (StoppedWading() && false) {
        int random = rand() % 9;
        Audio::PlayAudio(wadeEndFilenames[random], 1.0);
    }

    if (FeetEnteredUnderwater()) {
        Audio::PlayAudio("Water_Impact0.wav", 1.0);
    }

    if (CameraExitedUnderwater()) {
        Audio::PlayAudio("Water_ExitAndPant0.wav", 1.0);
    }

    if (!FeetBelowWater()) { 
        // Footsteps
        if (m_bobOffsetY < -0.04f && !m_footstepPlayed && IsGrounded()) {
            Game::PlayFootstepOutdoorAudio();
            m_footstepPlayed = true;
        }
        if (m_bobOffsetY > 0.0f) {
            m_footstepPlayed = false;
        }

        // Landing on ground
        if (m_grounded && !m_groundedLastFrame) {
            Game::PlayFootstepOutdoorAudio();
            m_headBobTime = 0.0f;
        }
    }
}