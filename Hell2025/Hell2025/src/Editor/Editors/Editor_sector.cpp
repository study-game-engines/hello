#include "Editor/Editor.h"

#include "Callbacks/Callbacks.h"
#include "Config/Config.h"
#include "Audio/Audio.h"
#include "Core/JSON.h"
#include "Editor/Gizmo.h"
#include "ImGui/EditorImgui.h"
#include "Renderer/Renderer.h"
#include "World/HeightMapManager.h"
#include "World/SectorManager.h"
#include "World/World.h"

#include "Imgui/ImguiBackEnd.h"
#include <ImGui/imgui.h>

namespace Editor {

    struct SectorEditorImguiElements {
        EditorUI::FileMenu fileMenu;
        EditorUI::LeftPanel leftPanel;
        EditorUI::CollapsingHeader sectorPropertiesHeader;
        EditorUI::CollapsingHeader rendererSettingsHeader;
        EditorUI::CollapsingHeader objectSettingsHeader;
        EditorUI::CollapsingHeader outlinerHeader;
        EditorUI::Outliner outliner;
        EditorUI::DropDown heightMapDropDown;
        EditorUI::StringInput sectorNameInput;
        EditorUI::CheckBox drawGrass;
        EditorUI::CheckBox drawWater;
        EditorUI::Vec3Input objectPositon;
        EditorUI::Vec3Input objectRotation;
        EditorUI::Vec3Input objectScale;
        EditorUI::NewFileWindow newFileWindow;
        EditorUI::OpenFileWindow openFileWindow;
    } g_sectorEditorImguiElements;


    std::string g_sectorName = "TestSector";

    void InitSectorEditorFileMenu();
    void InitSectorEditorPropertiesElements();
    void ReconfigureSectorEditorImGuiElements();

    void InitSectorEditor() {
        InitSectorEditorFileMenu();
        InitSectorEditorPropertiesElements();
    }

    void InitSectorEditorFileMenu() {
        SectorEditorImguiElements& elements = g_sectorEditorImguiElements;

        EditorUI::FileMenuNode& file = elements.fileMenu.AddMenuNode("File", nullptr);
        file.AddChild("New", []() { ShowNewSectorWindow(); }, "F2");
        file.AddChild("Open", []() { ShowOpenSectorWindow(); }, "F3");
        file.AddChild("Save", []() { Callbacks::SaveEditorSector(); }, "Ctrl+S");
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

        EditorUI::FileMenuNode& insert = elements.fileMenu.AddMenuNode("Insert");
        insert.AddChild("Reinsert last", []() { nullptr; }, "Ctrl T");

        EditorUI::FileMenuNode& nature = insert.AddChild("Nature", nullptr);
        nature.AddChild("Tree", nullptr);

        EditorUI::FileMenuNode& pickups = insert.AddChild("Pick Ups", nullptr);

        EditorUI::FileMenuNode& weapons = pickups.AddChild("Weapons", nullptr);
        weapons.AddChild("AKS74U", nullptr);
        weapons.AddChild("FN-P90", nullptr);
        weapons.AddChild("Glock", nullptr);
        weapons.AddChild("Golden Glock", nullptr);
        weapons.AddChild("Remington 870", nullptr);
        weapons.AddChild("SPAS", nullptr);
        weapons.AddChild("Tokarev", nullptr);

        EditorUI::FileMenuNode& ammo = pickups.AddChild("Ammo", nullptr);
        ammo.AddChild("AKS74U", nullptr);
        ammo.AddChild("FN-P90", nullptr);
        ammo.AddChild("Glock", nullptr);
        ammo.AddChild("Shotgun Shells Buckshot", nullptr);
        ammo.AddChild("Shotgun Shells Slug", nullptr);
        ammo.AddChild("Tokarev", nullptr);

        EditorUI::FileMenuNode& run = elements.fileMenu.AddMenuNode("Run");
        run.AddChild("New Run", nullptr, "F1");
        run.AddChild("Test Sector", nullptr);
    }

    void InitSectorEditorPropertiesElements() {
        SectorEditorImguiElements& elements = g_sectorEditorImguiElements;
        elements.sectorPropertiesHeader.SetTitle("Sector Properties");
        elements.rendererSettingsHeader.SetTitle("Renderer Settings");
        elements.objectSettingsHeader.SetTitle("Object Settings");
        elements.outlinerHeader.SetTitle("Outliner");
        elements.sectorNameInput.SetLabel("Name");
        elements.heightMapDropDown.SetText("Height Map");
        elements.drawGrass.SetText("Draw Grass");
        elements.drawWater.SetText("Draw Water");
        elements.objectPositon.SetText("Position");
        elements.objectRotation.SetText("Rotation");
        elements.objectScale.SetText("Scale");
        elements.outliner.AddType("Game Objects");
        elements.outliner.AddType("Lights");
        elements.outliner.AddType("Pick Ups");
        elements.outliner.AddType("Trees");
        elements.newFileWindow.SetTitle("New Sector");
        elements.newFileWindow.SetCallback(Callbacks::NewSector);
        elements.openFileWindow.SetTitle("Open Sector");
        elements.openFileWindow.SetPath("res/sectors/");
        elements.openFileWindow.SetCallback(Callbacks::OpenSector);
    }

    void CreateSectorEditorImGuiElements() {
        SectorEditorImguiElements& elements = g_sectorEditorImguiElements;

        elements.fileMenu.CreateImguiElements();
        elements.leftPanel.BeginImGuiElement();

        SectorCreateInfo* sectorCreateInfo = SectorManager::GetSectorCreateInfoByName(g_sectorName);
        if (!sectorCreateInfo) return;

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

        // Sector properties
        if (elements.sectorPropertiesHeader.CreateImGuiElement()) {
            elements.sectorNameInput.CreateImGuiElement();
            if (elements.heightMapDropDown.CreateImGuiElements()) {
                sectorCreateInfo->heightMapName = elements.heightMapDropDown.GetSelectedOptionText();
                World::LoadSingleSector(sectorCreateInfo);
                Renderer::RecalculateAllHeightMapData();
            }
            ImGui::Dummy(ImVec2(0.0f, 20.0f));
        }

        // Object settings
        if (elements.objectSettingsHeader.CreateImGuiElement()) {         
            if (elements.objectPositon.CreateImGuiElements()) {
                std::cout << Util::Vec3ToString(elements.objectPositon.GetValue()) << "\n";;
            }
            if (elements.objectRotation.CreateImGuiElements()) {
                std::cout << Util::Vec3ToString(elements.objectRotation.GetValue()) << "\n";;
            }
            if (elements.objectScale.CreateImGuiElements()) {
                std::cout << Util::Vec3ToString(elements.objectScale.GetValue()) << "\n";;
            }
            ImGui::Dummy(ImVec2(0.0f, 20.0f));
        }

        // Outliner settings
        if (elements.outlinerHeader.CreateImGuiElement()) {
            elements.outliner.CreateImGuiElements();
            ImGui::Dummy(ImVec2(0.0f, 20.0f));
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

    void ReconfigureSectorEditorImGuiElements() {
        SectorEditorImguiElements& elements = g_sectorEditorImguiElements;

        RendererSettings& renderSettings = Renderer::GetCurrentRendererSettings();
        elements.drawGrass.SetState(renderSettings.drawGrass);

        SectorCreateInfo* sectorCreateInfo = SectorManager::GetSectorCreateInfoByName(g_sectorName);
        if (!sectorCreateInfo) return;

        // Sector name
        elements.sectorNameInput.SetText(sectorCreateInfo->sectorName);

        // Height map
        std::vector<std::string> heightMaps = { "None" };
        heightMaps.insert(heightMaps.end(), HeightMapManager::GetHeigthMapNames().begin(), HeightMapManager::GetHeigthMapNames().end());
        elements.heightMapDropDown.SetOptions(heightMaps);
        elements.heightMapDropDown.SetCurrentOption(sectorCreateInfo->heightMapName);

        // Outliner
        std::vector<std::string> gameObjects = { "shit", "fuck" };
        elements.outliner.SetItems("Game Objects", gameObjects);
    }

    void ShowNewSectorWindow() {
        CloseAllEditorWindows();
        SectorEditorImguiElements& elements = g_sectorEditorImguiElements;
        elements.newFileWindow.Show();
    }

    void ShowOpenSectorWindow() {
        CloseAllEditorWindows();
        SectorEditorImguiElements& elements = g_sectorEditorImguiElements;
        elements.openFileWindow.Show();
    }

    void CloseAllSectorEditorWindows() {
        SectorEditorImguiElements& elements = g_sectorEditorImguiElements;
        elements.newFileWindow.Close();
        elements.openFileWindow.Close();
    }

    void OpenSectorEditor() {
        Audio::PlayAudio(AUDIO_SELECT, 1.0f);

        if (IsEditorClosed()) {
            OpenEditor();
        }

        if (GetEditorMode() != EditorMode::SECTOR_EDITOR) {
            SetEditorMode(EditorMode::SECTOR_EDITOR);
            Gizmo::SetPosition(glm::vec3(32.0f, 0.0f, 32.0f));
        }

        // Attempt to load last sector
        SectorCreateInfo* sectorCreateInfo = SectorManager::GetSectorCreateInfoByName(g_sectorName);
        if (sectorCreateInfo) {
            // Load it into the world
            World::LoadSingleSector(sectorCreateInfo);
        }
        // If it failed, open the "Open Sector" window
        else {
            World::ResetWorld();
            ShowOpenSectorWindow();
        }

        ReconfigureSectorEditorImGuiElements();
    }

    void UpdateSectorEditor() {
        // Update the current SectorCreateInfo with the actual state of the objects in the world (for saving)
        SectorCreateInfo* sectorCreateInfo = SectorManager::GetSectorCreateInfoByName(g_sectorName);
        if (!sectorCreateInfo) return;

        sectorCreateInfo->gameObjects.clear();
        sectorCreateInfo->lights.clear();
        sectorCreateInfo->pickUps.clear();
        sectorCreateInfo->trees.clear();

        sectorCreateInfo->gameObjects.reserve(World::GetRenderItemsAlphaDiscarded().size());
        sectorCreateInfo->lights.reserve(World::GetLights().size());
        sectorCreateInfo->pickUps.reserve(World::GetPickUps().size());
        sectorCreateInfo->trees.reserve(World::GetTrees().size());

        for (GameObject& gameObject : World::GetGameObjects()) {
            sectorCreateInfo->gameObjects.emplace_back(gameObject.GetCreateInfo());
        }
        for (Light& light : World::GetLights()) {
            sectorCreateInfo->lights.emplace_back(light.GetCreateInfo());
        }
        for (PickUp& pickUp : World::GetPickUps()) {
            sectorCreateInfo->pickUps.emplace_back(pickUp.GetCreateInfo());
        }
        for (Tree& tree : World::GetTrees()) {
            sectorCreateInfo->trees.emplace_back(tree.GetCreateInfo());
        }

        // Draw sector perimeter
        glm::vec3 p0 = glm::vec3(0.0f, 0.0f, 0.0f);
        glm::vec3 p1 = glm::vec3(64.0f, 0.0f, 0.0f);
        glm::vec3 p2 = glm::vec3(0.0f, 0.0f, 64.0f);
        glm::vec3 p3 = glm::vec3(64.0f, 0.0f, 64.0f);
        Renderer::DrawLine(p0, p1, GRID_COLOR, true);
        Renderer::DrawLine(p0, p2, GRID_COLOR, true);
        Renderer::DrawLine(p2, p3, GRID_COLOR, true);
        Renderer::DrawLine(p1, p3, GRID_COLOR, true);
    }

    void LoadEditorSector(const std::string& sectorName) {
        SectorCreateInfo* sectorCreateInfo = SectorManager::GetSectorCreateInfoByName(sectorName);
        if (!sectorCreateInfo) {
            std::cout << "Editor::LoadEditorSector() failed, sector name '" << sectorName << "' was not found\n";
            return;
        }
        g_sectorName = sectorName;
        World::LoadSingleSector(sectorCreateInfo);
    }

    void SaveEditorSector() {
        SectorCreateInfo* sectorCreateInfo = SectorManager::GetSectorCreateInfoByName(g_sectorName);
        if (sectorCreateInfo) {
            std::string filePath = "res/sectors/" + sectorCreateInfo->sectorName + ".json";
            JSON::SaveSector(filePath, *sectorCreateInfo);
        } 
        else {
            std::cout << "Editor::SaveEditorSector() failed because g_sectorName '" << g_sectorName << "' was not found\n";
        }
    }

    const std::string& GetEditorSectorName() {
        return g_sectorName;
    }

    SectorCreateInfo* GetEditorSectorCreateInfo() {
        return SectorManager::GetSectorCreateInfoByName(g_sectorName);
    }   
}

