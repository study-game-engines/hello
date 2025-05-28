#pragma once
#include "Callbacks.h"
#include "BackEnd/BackEnd.h"
#include "Editor/Editor.h"
#include "API/OpenGL/Renderer/GL_Renderer.h" // Get me the fuck out of here
#include "World/MapManager.h"
#include "World/SectorManager.h"
#include "World/World.h"
#include <iostream>

namespace Callbacks {
    void NewHeightMap(const std::string& filename) {
        std::cout << "TODO: NewHeightMap() callback: " << filename << "\n";
    }

    void NewHouse(const std::string& filename) {
        std::cout << "TODO: NewHouse() callback: " << filename << "\n";
    }

    void NewMap(const std::string& filename) {
        std::cout << "TODO: NewMap() callback: " << filename << "\n";
    }

    void NewSector(const std::string& filename) {
        SectorManager::NewSector(filename);
        Editor::LoadSectorFromDisk(filename);
    }

    void OpenHeightMap(const std::string& filename) {
        Editor::LoadHeightMap(filename);
    }

    void OpenHouse(const std::string& filename) {
        std::cout << "TODO: OpenHouse() callback: " << filename << "\n";
    }

    void SaveHouse() {
        World::SaveHouse();
    }

    void OpenMap(const std::string& filename) {
        World::LoadMap(filename);
    }

    void TestCurrentMap() {
        World::LoadMap("TestMap");
    }

    void OpenSector(const std::string& filename) {
        SectorManager::UpdateSectorFromDisk(filename);
        Editor::LoadSectorFromDisk(filename);
        Editor::OpenSectorEditor();
    }

    void SaveHeightMaps() {
        OpenGLRenderer::SaveHeightMaps(); // Move out of the renderer
    }

    void SaveSector() {
        Editor::SaveSector();
    }

    void RevertEditorSector() {
        //World::RevertToSavedSector();
        std::cout << "RevertEditorSectorToLastSavedState()\n";
    }

    void OpenHeightMapEditor() {
        Editor::OpenHeightMapEditor();
    }

    void OpenHouseEditor() {
        Editor::OpenHouseEditor();
    }
    void OpenMapEditor() {
        Editor::OpenMapEditor();
    }

    void OpenSectorEditor() {
        Editor::OpenSectorEditor();
    }

    void OpenWeaponsEditor() {
        if (Editor::GetEditorMode() != EditorMode::WEAPON_EDITOR) {
            Editor::SetEditorMode(EditorMode::WEAPON_EDITOR);
            if (Editor::IsClosed()) {
                Editor::OpenEditor();
            }
        }
    }

    void BeginAddingDoor() {
        Editor::SetEditorState(EditorState::DOOR_PLACEMENT);
    }

    void BeginAddingPictureFrame() {
        Editor::SetEditorState(EditorState::PICTURE_FRAME_PLACEMENT);
    }

    void BeginAddingTree() {
        Editor::SetEditorState(EditorState::TREE_PLACEMENT);
    }

    void BeginAddingWall() {
        Editor::SetEditorState(EditorState::WALL_PLACEMENT);
    }

    void BeginAddingWindow() {
        Editor::SetEditorState(EditorState::WINDOW_PLACEMENT);
    }

    void QuitProgram() {
        BackEnd::ForceCloseWindow();
    }
}