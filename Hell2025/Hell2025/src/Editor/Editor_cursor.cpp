#include "Editor.h"
#include "HellDefines.h"
#include "BackEnd/BackEnd.h"

namespace Editor {

    void UpdateCursor() {
        // Resizing dividers
        if (IsHorizontalDividerHovered() && IsVerticalDividerHovered()) {
            BackEnd::SetCursor(HELL_CURSOR_CROSSHAIR);
        }
        else if (IsHorizontalDividerHovered()) {
            BackEnd::SetCursor(HELL_CURSOR_HRESIZE);
        }
        else if (IsVerticalDividerHovered()) {
            BackEnd::SetCursor(HELL_CURSOR_VRESIZE);
        }        
        // Hovering dividers
        else if (GetEditorState() == EditorState::RESIZING_HORIZONTAL_VERTICAL) {
            BackEnd::SetCursor(HELL_CURSOR_CROSSHAIR);
        }
        else if (GetEditorState() == EditorState::RESIZING_HORIZONTAL) {
            BackEnd::SetCursor(HELL_CURSOR_HRESIZE);
        }
        else if (GetEditorState() == EditorState::RESIZING_VERTICAL) {
            BackEnd::SetCursor(HELL_CURSOR_VRESIZE);
        }
        else if (GetEditorState() == EditorState::WALL_PLACEMENT) {
            BackEnd::SetCursor(HELL_CURSOR_CROSSHAIR);
        }
        // Nothing? Then the arrow
        else {
            BackEnd::SetCursor(HELL_CURSOR_ARROW);
        }
    }
}