
#include "Audio/Audio.h"
#include "Editor/Editor.h"
#include "Input/Input.h"
#include "World/World.h"

namespace Editor {
    uint64_t g_placementObjectId = 0;
    EditorState g_lastPlacementState = EditorState::IDLE;

    void UpdateObjectPlacement() {
        if (GetEditorState() == EditorState::DOOR_PLACEMENT) UpdateDoorPlacement();
        if (GetEditorState() == EditorState::PICTURE_FRAME_PLACEMENT) UpdatePictureFramePlacement();
        if (GetEditorState() == EditorState::TREE_PLACEMENT) UpdateTreePlacement();
        if (GetEditorState() == EditorState::WALL_PLACEMENT) UpdateWallPlacement();
        if (GetEditorState() == EditorState::WINDOW_PLACEMENT) UpdateWindowPlacement();

        // Store last placement state
        if (GetEditorState() == EditorState::DOOR_PLACEMENT ||
            GetEditorState() == EditorState::PICTURE_FRAME_PLACEMENT ||
            GetEditorState() == EditorState::TREE_PLACEMENT ||
            GetEditorState() == EditorState::WINDOW_PLACEMENT) {
            g_lastPlacementState = GetEditorState();
        }

        // Re-insert last
        if (Input::KeyDown(HELL_KEY_LEFT_CONTROL_GLFW) && Input::KeyPressed(HELL_KEY_T)) {

            // House editor
            if (GetEditorMode() == EditorMode::HOUSE_EDITOR) {
                if (g_lastPlacementState == EditorState::PICTURE_FRAME_PLACEMENT) {
                    Audio::PlayAudio(AUDIO_SELECT, 1.0f);
                    SetEditorState(EditorState::PICTURE_FRAME_PLACEMENT);
                }
            }
            // Sector editor
            if (GetEditorMode() == EditorMode::SECTOR_EDITOR) {
                if (g_lastPlacementState == EditorState::TREE_PLACEMENT) {
                    Audio::PlayAudio(AUDIO_SELECT, 1.0f);
                    SetEditorState(EditorState::TREE_PLACEMENT);
                }
            }
        }
    }

    void ExitObjectPlacement() {
        SetPlacementObjectId(0);
        SetEditorState(EditorState::IDLE);
    }

    uint64_t GetPlacementObjectId() {
        return g_placementObjectId;
    }

    void SetPlacementObjectId(uint64_t objectId) {
        g_placementObjectId = objectId;
    }

}