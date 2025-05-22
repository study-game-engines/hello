#include "Editor/Editor.h"

#include "Audio/Audio.h"
#include "Core/JSON.h"
#include "Editor/Gizmo.h"
#include "Input/Input.h"
#include "ImGui/EditorImgui.h"
#include "Renderer/Renderer.h"
#include "World/MapManager.h"
#include "World/SectorManager.h"
#include "World/World.h"
#include "Viewport/ViewportManager.h"
#include "World/HeightMapManager.h"

#include "Imgui/ImguiBackEnd.h"
#include <ImGui/imgui.h>

namespace Editor {

    struct HeightMapEditorEditorImguiElements {
        EditorUI::FileMenu fileMenu;
        EditorUI::LeftPanel leftPanel;
        EditorUI::CollapsingHeader rendererSettingsHeader;
        EditorUI::CollapsingHeader heightMapPropertiesHeader;
        EditorUI::StringInput heightMapNameInput;
        EditorUI::DropDown heightMapDropwDownN;
        EditorUI::DropDown heightMapDropwDownS;
        EditorUI::DropDown heightMapDropwDownE;
        EditorUI::DropDown heightMapDropwDownW;
        EditorUI::DropDown heightMapDropwDownNE;
        EditorUI::DropDown heightMapDropwDownNW;
        EditorUI::DropDown heightMapDropwDownSE;
        EditorUI::DropDown heightMapDropwDownSW;
        EditorUI::CheckBox drawGrass;
        EditorUI::CheckBox drawWater;
        EditorUI::NewFileWindow newFileWindow;
        EditorUI::OpenFileWindow openFileWindow;
        EditorUI::FloatSliderInput brushSize;
        EditorUI::FloatSliderInput brushStrength;
        EditorUI::FloatSliderInput noiseStrength;
        EditorUI::FloatSliderInput noiseScale;
        EditorUI::FloatInput minPaintHeight;
        EditorUI::FloatInput maxPaintHeight;
    } g_heightMapEditorImguiElements;

    float g_brushSize = 16;
    float g_brushStrength = 1;
    float g_noiseStrength = 1;
    float g_minPaintHeight = 0.0f;
    float g_noiseScale = 0.5f;
    float g_maxPaintHeight = HEIGHTMAP_SCALE_Y;

    void InitHeightMapEditorFileMenu();
    void InitHeightMapEditorPropertiesElements();
    void ReconfigureHeightMapEditorImGuiElements();

    void InitHeightMapEditor() {
        InitHeightMapEditorFileMenu();
        InitHeightMapEditorPropertiesElements();
    }

    void InitHeightMapEditorFileMenu() {
        HeightMapEditorEditorImguiElements& elements = g_heightMapEditorImguiElements;

        EditorUI::FileMenuNode& file = elements.fileMenu.AddMenuNode("File", nullptr);
        file.AddChild("New", []() { ShowNewHeightMapWindow(); }, "F2");
        file.AddChild("Open", []() { ShowOpenHeightMapWindow(); }, "F3");
        file.AddChild("Save", []() { Callbacks::SaveHeightMaps(); }, "Ctrl+S");
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

    void InitHeightMapEditorPropertiesElements() {
        HeightMapEditorEditorImguiElements& elements = g_heightMapEditorImguiElements;
        elements.rendererSettingsHeader.SetTitle("Renderer Settings");
        elements.drawGrass.SetText("Draw Grass");
        elements.drawWater.SetText("Draw Water");
        elements.heightMapPropertiesHeader.SetTitle("Height Map Properties");
        elements.heightMapNameInput.SetLabel("Name");
        elements.heightMapDropwDownN.SetText("Height Map N");
        elements.heightMapDropwDownS.SetText("Height Map S");
        elements.heightMapDropwDownE.SetText("Height Map E");
        elements.heightMapDropwDownW.SetText("Height Map W");
        elements.heightMapDropwDownNE.SetText("Height Map NE");
        elements.heightMapDropwDownNW.SetText("Height Map NW");
        elements.heightMapDropwDownSE.SetText("Height Map SE");
        elements.heightMapDropwDownSW.SetText("Height Map SW");

        elements.minPaintHeight.SetText("Min Height");
        elements.minPaintHeight.SetRange(0.0f, HEIGHTMAP_SCALE_Y);
        elements.minPaintHeight.SetValue(0.0f);

        elements.maxPaintHeight.SetText("Max Height");
        elements.maxPaintHeight.SetRange(0.0f, HEIGHTMAP_SCALE_Y);
        elements.maxPaintHeight.SetValue(HEIGHTMAP_SCALE_Y);

        elements.brushStrength.SetText("Brush Strength");
        elements.brushStrength.SetRange(0.0f, 1.0f);
        elements.brushStrength.SetValue(g_brushStrength);

        elements.noiseStrength.SetText("Noise Strength");
        elements.noiseStrength.SetRange(0.0f, 1.0f);
        elements.noiseStrength.SetValue(g_noiseStrength);

        elements.noiseScale.SetText("Noise Scale");
        elements.noiseScale.SetRange(0.0f, 1.0f);
        elements.noiseScale.SetValue(g_noiseScale);

        elements.brushSize.SetText("Brush Size");
        elements.brushSize.SetRange(0.0f, 100.0f);
        elements.brushSize.SetValue(g_brushSize);

        elements.newFileWindow.SetTitle("New Height Map");
        elements.newFileWindow.SetCallback(Callbacks::NewHeightMap);
        elements.openFileWindow.SetTitle("Open Height Map");
        elements.openFileWindow.SetPath("res/height_maps/");
        elements.openFileWindow.SetCallback(Callbacks::OpenHeightMap);
    }

    void ReconfigureHeightMapEditorImGuiElements() {
        HeightMapEditorEditorImguiElements& elements = g_heightMapEditorImguiElements;

        // Update name input with height map name
        elements.heightMapNameInput.SetText(SectorManager::GetSectorHeightMapName("HeightMapEditor_Center"));

        // Height map neighbor drop downs
        std::vector<std::string> heightMaps = { "None" };
        heightMaps.insert(heightMaps.end(), HeightMapManager::GetHeigthMapNames().begin(), HeightMapManager::GetHeigthMapNames().end());

        // First zero set all heightmaps to none
        elements.heightMapDropwDownN.SetOptions(heightMaps);
        elements.heightMapDropwDownS.SetOptions(heightMaps);
        elements.heightMapDropwDownE.SetOptions(heightMaps);
        elements.heightMapDropwDownW.SetOptions(heightMaps);
        elements.heightMapDropwDownNE.SetOptions(heightMaps);
        elements.heightMapDropwDownNW.SetOptions(heightMaps);
        elements.heightMapDropwDownSE.SetOptions(heightMaps);
        elements.heightMapDropwDownSW.SetOptions(heightMaps);

        MapCreateInfo* mapCreateInfo = MapManager::GetHeightMapEditorMapCreateInfo();
        if (mapCreateInfo) {

        }


        RendererSettings& renderSettings = Renderer::GetCurrentRendererSettings();
        elements.drawGrass.SetState(renderSettings.drawGrass);
    }

    void CreateHeigthMapEditorImGuiElements() {
        HeightMapEditorEditorImguiElements& elements = g_heightMapEditorImguiElements;
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
        if (elements.heightMapPropertiesHeader.CreateImGuiElement()) {
            elements.heightMapNameInput.CreateImGuiElement();

            if (elements.brushSize.CreateImGuiElements()) {
                g_brushSize = elements.brushSize.GetValue();
            }

            if (elements.minPaintHeight.CreateImGuiElements()) {
                g_minPaintHeight = elements.minPaintHeight.GetValue();
            }

            if (elements.maxPaintHeight.CreateImGuiElements()) {
                g_maxPaintHeight = elements.maxPaintHeight.GetValue();
            }

            if (elements.brushStrength.CreateImGuiElements()) {
                g_brushStrength = elements.brushStrength.GetValue();
            }

            if (elements.noiseStrength.CreateImGuiElements()) {
                g_noiseStrength = elements.noiseStrength.GetValue();
            }

            if (elements.noiseScale.CreateImGuiElements()) {
                g_noiseScale = elements.noiseScale.GetValue();
            }

            bool reloadRequired = false;

            if (reloadRequired) {
                MapCreateInfo* mapCreateInfo = MapManager::GetMapCreateInfoByName("HeightMapEditorMap");
                World::LoadMap(mapCreateInfo);
                Renderer::RecalculateAllHeightMapData();
            }
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

    void OpenHeightMapEditor() {
        Audio::PlayAudio(AUDIO_SELECT, 1.0f);

        if (IsClosed()) {
            OpenEditor();
        }

        if (GetEditorMode() != EditorMode::HEIGHTMAP_EDITOR) {
            SetEditorMode(EditorMode::HEIGHTMAP_EDITOR);
        }

        std::string sectorName = "TestSector";
        SectorCreateInfo* sectorCreateInfo = SectorManager::GetSectorCreateInfoByName(sectorName);
        if (sectorCreateInfo) {
            World::LoadSingleSector(sectorCreateInfo);
        }

        //World::LoadSingleSector(sectorCreateInfo);
        //MapCreateInfo* mapCreateInfo = MapManager::GetHeightMapEditorMapCreateInfo();
        //World::LoadMap(mapCreateInfo);

        ReconfigureHeightMapEditorImGuiElements();
    }

    void LoadHeightMap(const std::string& heightMapName) {

        SectorManager::GetSectorCreateInfoByName("HeightMapEditor_Center");

        SectorManager::SetSectorHeightMap("HeightMapEditor_Center", heightMapName);
        SectorManager::SetSectorHeightMap("HeightMapEditor_SW", "");
        SectorManager::SetSectorHeightMap("HeightMapEditor_S", "");
        SectorManager::SetSectorHeightMap("HeightMapEditor_SE", "");
        SectorManager::SetSectorHeightMap("HeightMapEditor_W", "");
        SectorManager::SetSectorHeightMap("HeightMapEditor_E", "");
        SectorManager::SetSectorHeightMap("HeightMapEditor_NW", "");
        SectorManager::SetSectorHeightMap("HeightMapEditor_N", "");
        SectorManager::SetSectorHeightMap("HeightMapEditor_NE", "");

        MapCreateInfo* mapCreateInfo = MapManager::GetHeightMapEditorMapCreateInfo();
        World::LoadMap(mapCreateInfo);
        ReconfigureHeightMapEditorImGuiElements();

        std::cout << "Loaded height map: " << heightMapName << "\n";
    }

    void UpdateHeightMapEditor() {
        int mapWidth = World::GetMapWidth();
        int mapDepth = World::GetMapDepth();
        float worldWidth = World::GetWorldSpaceWidth();
        float worldDepth = World::GetWorldSpaceDepth();

        int viewportIndex = Editor::GetHoveredViewportIndex();
        const Viewport* viewport = ViewportManager::GetViewportByIndex(viewportIndex);
        const glm::vec3 rayOrigin = Editor::GetMouseRayOriginByViewportIndex(viewportIndex);
        const glm::vec3 rayDir = Editor::GetMouseRayDirectionByViewportIndex(viewportIndex);

        ivecXZ hovered = ivecXZ(-1, -1);

        // Draw grid and check for mouse hover
        for (int x = 0; x < mapWidth; x++) {
            for (int z = 0; z < mapDepth; z++) {
                glm::vec3 p0 = glm::vec3((x + 0) * 64.0f, 0.0f, (z + 0) * 64.0f);
                glm::vec3 p1 = glm::vec3((x + 0) * 64.0f, 0.0f, (z + 1) * 64.0f);
                glm::vec3 p2 = glm::vec3((x + 1) * 64.0f, 0.0f, (z + 0) * 64.0f);
                glm::vec3 p3 = glm::vec3((x + 1) * 64.0f, 0.0f, (z + 1) * 64.0f);
                float t = 0;
                if (Util::RayIntersectsTriangle(rayOrigin, rayDir, p0, p1, p2, t) ||
                    Util::RayIntersectsTriangle(rayOrigin, rayDir, p1, p3, p2, t)) {
                    hovered = ivecXZ(x, z);
                }
                Renderer::DrawLine(p0, p1, GRID_COLOR, true);
                Renderer::DrawLine(p0, p2, GRID_COLOR, true);
                Renderer::DrawLine(p2, p3, GRID_COLOR, true);
                Renderer::DrawLine(p1, p3, GRID_COLOR, true);
            }
        }

        // Draw hovered
        if (hovered != ivecXZ(-1, -1)) {
            glm::vec3 p0 = glm::vec3((hovered.x + 0) * 64.0f, 0.0f, (hovered.z + 0) * 64.0f);
            glm::vec3 p1 = glm::vec3((hovered.x + 0) * 64.0f, 0.0f, (hovered.z + 1) * 64.0f);
            glm::vec3 p2 = glm::vec3((hovered.x + 1) * 64.0f, 0.0f, (hovered.z + 0) * 64.0f);
            glm::vec3 p3 = glm::vec3((hovered.x + 1) * 64.0f, 0.0f, (hovered.z + 1) * 64.0f);
            Renderer::DrawLine(p0, p1, WHITE, true);
            Renderer::DrawLine(p0, p2, WHITE, true);
            Renderer::DrawLine(p2, p3, WHITE, true);
            Renderer::DrawLine(p1, p3, WHITE, true);
        }
    }

    void ShowNewHeightMapWindow() {
        CloseAllEditorWindows();
        HeightMapEditorEditorImguiElements& elements = g_heightMapEditorImguiElements;
        elements.newFileWindow.Show();
    }

    void ShowOpenHeightMapWindow() {
        CloseAllEditorWindows();
        HeightMapEditorEditorImguiElements& elements = g_heightMapEditorImguiElements;
        elements.openFileWindow.Show();
    }

    void CloseAllHeightMapEditorWindows() {
        HeightMapEditorEditorImguiElements& elements = g_heightMapEditorImguiElements;
        elements.newFileWindow.Close();
        elements.openFileWindow.Close();
    }

    float GetHeightMapNoiseScale() {
        return g_noiseScale;
    }

    float GetHeightMapBrushSize() {
        return g_brushSize;
    }

    float GetHeightMapBrushStrength() {
        return g_brushStrength;
    }

    float GetHeightMapNoiseStrength() {
        return g_noiseStrength;
    }

    float GetHeightMapMinPaintHeight() {
        return g_minPaintHeight;
    }

    float GetHeightMapMaxPaintHeight() {
        return g_maxPaintHeight;
    }
}