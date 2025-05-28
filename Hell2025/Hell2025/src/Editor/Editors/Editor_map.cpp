#include "Editor/Editor.h"

#include "Audio/Audio.h"
#include "ImGui/Types/Types.h"
#include "Renderer/Renderer.h"
#include "World/MapManager.h"
#include "World/World.h"

#include <imgui/imgui.h>

namespace Editor {

    struct MapEditorEditorImguiElements {
        EditorUI::FileMenu fileMenu;
        EditorUI::LeftPanel leftPanel;
        EditorUI::CollapsingHeader rendererSettingsHeader;
        EditorUI::CollapsingHeader mapPropertiesHeader;
        EditorUI::StringInput mapNameInput;
        EditorUI::CheckBox drawGrass;
        EditorUI::CheckBox drawWater;
        EditorUI::NewFileWindow newFileWindow;
        EditorUI::OpenFileWindow openFileWindow;
        EditorUI::IntegerInput test;
        EditorUI::FloatSliderInput test2;
    } g_mapEditorImguiElements;

    void InitMapEditorFileMenu();
    void InitMapEditorPropertiesElements();
    void ReconfigureHMapEditorImGuiElements();

    void InitMapEditor() {
        InitMapEditorFileMenu();
        InitMapEditorPropertiesElements();
    }

    void InitMapEditorFileMenu() {
        MapEditorEditorImguiElements& elements = g_mapEditorImguiElements;

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

    void InitMapEditorPropertiesElements() {
        MapEditorEditorImguiElements& elements = g_mapEditorImguiElements;
        elements.rendererSettingsHeader.SetTitle("Renderer Settings");
        elements.drawGrass.SetText("Draw Grass");
        elements.drawWater.SetText("Draw Water");
        elements.mapPropertiesHeader.SetTitle("Map Properties");
        elements.test.SetText("Integer Test");
        elements.test.SetRange(-1, 10);
        elements.test.SetValue(8);
        elements.test2.SetText("Slider Test");
        elements.test2.SetRange(-1.0f, 10.0f);
        elements.test2.SetValue(8.0f);

        elements.newFileWindow.SetTitle("New Height Map");
        elements.newFileWindow.SetCallback(Callbacks::NewHeightMap);
        elements.openFileWindow.SetTitle("Open Height Map");
        elements.openFileWindow.SetPath("res/height_maps/");
        elements.openFileWindow.SetCallback(Callbacks::OpenHeightMap);
    }

    void ReconfigureHMapEditorImGuiElements() {
        MapEditorEditorImguiElements& elements = g_mapEditorImguiElements;

        // Update name input with height map name
        elements.mapNameInput.SetText(World::GetCurrentMapName());

        RendererSettings& renderSettings = Renderer::GetCurrentRendererSettings();
        elements.drawGrass.SetState(renderSettings.drawGrass);
    }

    void CreateMapEditorImGuiElements() {
        MapEditorEditorImguiElements& elements = g_mapEditorImguiElements;
        elements.fileMenu.CreateImguiElements();
        elements.leftPanel.BeginImGuiElement();

        // Renderer settings
        if (elements.rendererSettingsHeader.CreateImGuiElement()) {
            if (elements.drawGrass.CreateImGuiElements()) {
                RendererSettings& renderSettings = Renderer::GetCurrentRendererSettings();
                renderSettings.drawGrass = elements.drawGrass.GetState();
            }
            if (elements.drawWater.CreateImGuiElements()) {
                std::cout << elements.drawWater.GetState();
            }
            ImGui::Dummy(ImVec2(0.0f, 10.0f));
        }

        // Height map properties
        if (elements.mapPropertiesHeader.CreateImGuiElement()) {
            elements.mapNameInput.CreateImGuiElement();
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

    void OpenMapEditor() {
        Audio::PlayAudio(AUDIO_SELECT, 1.0f);

        if (Editor::IsClosed()) {
            Editor::OpenEditor();
        }

        if (Editor::GetEditorMode() != EditorMode::MAP_EDITOR) {
            Editor::SetEditorMode(EditorMode::MAP_EDITOR);
        }

        //if (World::GetCurrentMapCreateInfo() == nullptr) {
        //    World::ResetWorld();
        //    ShowOpenMapWindow();
        //} 
        //else {
            World::LoadMap("TestMap");
        //}
    }

    void UpdateMapEditor() {
        int mapWidth = World::GetMapWidth();
        int mapDepth = World::GetMapDepth();
        float worldWidth = (mapWidth) * 64.0f;
        float worldDepth = (mapDepth) * 64.0f;

        for (int x = 0; x <= mapWidth; x++) {
            glm::vec3 p0 = glm::vec3(x * 64.0f, 0.0f, 0);
            glm::vec3 p1 = glm::vec3(x * 64.0f, 0.0f, worldDepth);
            Renderer::DrawLine(p0, p1, GRID_COLOR, true);
        }
        for (int z = 0; z <= mapDepth; z++) {
            glm::vec3 p0 = glm::vec3(0.0f, 0.0f, z * 64.0f);
            glm::vec3 p1 = glm::vec3(worldWidth, 0.0f, z * 64.0f);
            Renderer::DrawLine(p0, p1, GRID_COLOR, true);
        }
    }

    void ShowNewMapWindow() {
        CloseAllEditorWindows();
    }

    void ShowOpenMapWindow() {
        CloseAllEditorWindows();
    }

    void CloseAllMapEditorWindows() {

    }
}