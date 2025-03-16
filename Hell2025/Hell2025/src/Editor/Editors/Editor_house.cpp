#include "Editor/Editor.h"

#include "Core/Audio.h"
#include "ImGui/EditorImgui.h"
#include "ImGui/Types/Types.h"
#include "Renderer/Renderer.h"
#include "World/MapManager.h"
#include "World/World.h"

#include <imgui/imgui.h>

namespace Editor {

    struct HouseEditorEditorImguiElements {
        EditorUI::FileMenu fileMenu;
        EditorUI::LeftPanel leftPanel;
        //EditorUI::CollapsingHeader rendererSettingsHeader;
        EditorUI::CollapsingHeader housePropertiesHeader;
        EditorUI::StringInput houseNameInput;
        //EditorUI::CheckBox drawGrass;
        //EditorUI::CheckBox drawWater;
        EditorUI::NewFileWindow newFileWindow;
        EditorUI::OpenFileWindow openFileWindow;
        //EditorUI::IntegerInput test;
        //EditorUI::FloatSliderInput test2;
    } g_houseEditorImguiElements;

    void InitHouseEditorFileMenu();
    void InitHouseEditorPropertiesElements();
    void ReconfigureHMapEditorImGuiElements();

    void InitHouseEditor() {
        InitHouseEditorFileMenu();
        InitHouseEditorPropertiesElements();
    }

    void InitHouseEditorFileMenu() {
        HouseEditorEditorImguiElements& elements = g_houseEditorImguiElements;

        EditorUI::FileMenuNode& file = elements.fileMenu.AddMenuNode("File", nullptr);
        file.AddChild("New", []() { ShowNewMapWindow(); }, "F2");
        file.AddChild("Open", []() { ShowOpenMapWindow(); }, "F3");
        file.AddChild("Save", nullptr, "Ctrl+S");
        file.AddChild("Revert", nullptr);
        file.AddChild("Delete", nullptr);
        file.AddChild("Duplicate", nullptr);
        file.AddChild("Quit", &Callbacks::QuitProgram, "Esc");

        EditorUI::FileMenuNode& editor = elements.fileMenu.AddMenuNode("Editor");
        editor.AddChild("House", &Callbacks::OpenHouseEditor, "F4");
        editor.AddChild("Sector", &Callbacks::OpenSectorEditor, "F5");
        editor.AddChild("Height Map", &Callbacks::OpenHeightMapEditor, "F6");
        editor.AddChild("Map", &Callbacks::OpenMapEditor, "F7");
        editor.AddChild("Weapons", &Callbacks::OpenWeaponsEditor, "F8");
    }

    void InitHouseEditorPropertiesElements() {
        HouseEditorEditorImguiElements& elements = g_houseEditorImguiElements;
        //elements.rendererSettingsHeader.SetTitle("Renderer Settings");
        //elements.drawGrass.SetText("Draw Grass");
        //elements.drawWater.SetText("Draw Water");
        elements.housePropertiesHeader.SetTitle("House Properties");
        elements.houseNameInput.SetLabel("House name");
        //elements.test.SetText("Integer Test");
        //elements.test.SetRange(-1, 10);
        //elements.test.SetValue(8);
        //elements.test2.SetText("Slider Test");
        //elements.test2.SetRange(-1.0f, 10.0f);
        //elements.test2.SetValue(8.0f);

        elements.newFileWindow.SetTitle("New House");
        elements.newFileWindow.SetCallback(Callbacks::NewHouse);
        elements.openFileWindow.SetTitle("Open House");
        elements.openFileWindow.SetPath("res/houses/");
        elements.openFileWindow.SetCallback(Callbacks::OpenHouse);
    }

    void ReconfigureHouseEditorImGuiElements() {
        HouseEditorEditorImguiElements& elements = g_houseEditorImguiElements;

        // Update name input with height map name
        elements.houseNameInput.SetText(World::GetCurrentMapName());

        //RendererSettings& renderSettings = Renderer::GetCurrentRendererSettings();
        //elements.drawGrass.SetState(renderSettings.drawGrass);
    }

    void CreateHouseEditorImGuiElements() {
        HouseEditorEditorImguiElements& elements = g_houseEditorImguiElements;
        elements.fileMenu.CreateImguiElements();
        elements.leftPanel.BeginImGuiElement();

        // Renderer settings
        //if (elements.rendererSettingsHeader.CreateImGuiElement()) {
        //    if (elements.drawGrass.CreateImGuiElements()) {
        //        RendererSettings& renderSettings = Renderer::GetCurrentRendererSettings();
        //        renderSettings.drawGrass = elements.drawGrass.GetState();
        //    }
        //    if (elements.drawWater.CreateImGuiElements()) {
        //        std::cout << elements.drawWater.GetState();
        //    }
        //    ImGui::Dummy(ImVec2(0.0f, 10.0f));
        //}

        // Height map properties
        if (elements.housePropertiesHeader.CreateImGuiElement()) {
            elements.houseNameInput.CreateImGuiElement();
            //elements.test.CreateImGuiElements();
            //elements.test2.CreateImGuiElements();

        }

        elements.leftPanel.EndImGuiElement();

        // Windows
        if (elements.newFileWindow.IsVisible()) {
            elements.newFileWindow.CreateImGuiElements();
        }
        if (elements.openFileWindow.IsVisible()) {
            elements.openFileWindow.CreateImGuiElements();
        }
    }

    void OpenHouseEditor() {
        Audio::PlayAudio(AUDIO_SELECT, 1.0f);

        World::ResetWorld();
        World::LoadEmptyWorld();

        if (Editor::GetEditorMode() != EditorMode::HOUSE_EDITOR) {
            Editor::SetEditorMode(EditorMode::HOUSE_EDITOR);
            if (Editor::IsEditorClosed()) {
                Editor::OpenEditor();
            }
        }


    }

    void UpdateHouseEditor() {
        // Draw Grid
        float gridWorldSpaceSize = 5.0f;
        float gridSpacing = 0.5f;

        for (float x = -gridWorldSpaceSize; x <= gridWorldSpaceSize; x += gridSpacing) {
            for (float z = -gridWorldSpaceSize; z <= gridWorldSpaceSize; z += gridSpacing) {
                glm::vec3 p1 = glm::vec3(x, 0, -gridWorldSpaceSize);
                glm::vec3 p2 = glm::vec3(x, 0, gridWorldSpaceSize);
                glm::vec3 p3 = glm::vec3(-gridWorldSpaceSize, 0.0f, z);
                glm::vec3 p4 = glm::vec3(gridWorldSpaceSize, 0.0f, z);
                Renderer::DrawLine(p1, p2, GRID_COLOR, true);
                Renderer::DrawLine(p3, p4, GRID_COLOR, true);
            }
        }
    }

    void ShowNewHouseWindow() {
        CloseAllEditorWindows();
    }

    void ShowOpenHouseWindow() {
        CloseAllEditorWindows();
    }

    void CloseAllHouseEditorWindows() {

    }
}