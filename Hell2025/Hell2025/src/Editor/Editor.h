#pragma once
#include "HellTypes.h"
#include "Callbacks/Callbacks.h"
#include "CreateInfo.h"
#include "Camera/Camera.h"
#include "Viewport/Viewport.h"
#include "Types/EditorMesh.h"
#include <string>

namespace Editor {
    void Init();
    void ResetCameras();
    void ResetViewports();
    void Update(float deltaTime);
    void UpdateCursor();
    void UpdateDividers();
    void UpdateInput();
    void UpdateUI();
    void UpdateCamera();
    void UpdateMouseRays();
    void UpdateCameraInterpolation(float deltaTime);
    void UpdateDebug();
    void OpenEditor();
    void CloseEditor();
    void ToggleEditorOpenState();
    void ShowEditorSelectMenu();
    void SetEditorMode(EditorMode editorMode);
    void CloseEditorSelectMenu();
    void SetActiveViewportIndex(int index);
    void SetSelectedObjectIndex(int index);
    void SetHoveredObjectIndex(int index);
    void SetSelectedObjectType(ObjectType editorObjectType);
    void SetHoveredObjectType(ObjectType editorObjectType);
    void SetSplitX(float value);
    void SetSplitY(float value);
    void SetViewportView(uint32_t viewportIndex, glm::vec3 viewportOrigin, CameraView targetView);
    void SetEditorState(EditorState editorState);
    void SetViewportOrthographicState(uint32_t viewportIndex, bool state);
    //void SetCameraView(uint32_t cameraViewIndex, CameraView cameraView);
    void SetViewportOrthoSize(uint32_t viewportIndex, float size);
    void SetEditorViewportSplitMode(EditorViewportSplitMode mode);
    void UpdateObjectSelection();

    // Height Map Editor
    void InitHeightMapEditor();
    void OpenHeightMapEditor();
    void LoadHeightMap(const std::string& heightMapName);
    void UpdateHeightMapEditor();
    void ShowNewHeightMapWindow();
    void ShowOpenHeightMapWindow();
    void CloseAllHeightMapEditorWindows();
    void CreateHeigthMapEditorImGuiElements();
     
    // House Editor
    void InitHouseEditor();
    void OpenHouseEditor();
    void UpdateHouseEditor();
    void ShowNewHouseWindow();
    void ShowOpenHouseWindow();
    void CloseAllHouseEditorWindows();
    void CreateHouseEditorImGuiElements();

    // Map Editor
    void InitMapEditor();
    void OpenMapEditor();
    void UpdateMapEditor();
    void ShowNewMapWindow();
    void ShowOpenMapWindow();
    void CloseAllMapEditorWindows();
    void CreateMapEditorImGuiElements();

    // Sector Editor
    void InitSectorEditor();
    void OpenSectorEditor();
    void UpdateSectorEditor();
    void LoadEditorSector(const std::string& sectorName);
    void SaveEditorSector();
    void CreateSectorEditorImGuiElements();
    void ShowNewSectorWindow();
    void ShowOpenSectorWindow();
    void CloseAllSectorEditorWindows();
    SectorCreateInfo* GetEditorSectorCreateInfo();
    const std::string& GetEditorSectorName();

    void CloseAllEditorWindows();

    //EditorUI::NewFileWindow& CreateNewFileWindow(const std::string& name, const std::string& title, NewFileCallback callback);
    //EditorUI::NewFileWindow* GetNewFileWindow(const std::string& name);
    //EditorUI::OpenFileWindow& CreateOpenFileWindow(const std::string& name, const std::string& title, const std::string filepath, OpenFileCallback callback);
    //EditorUI::OpenFileWindow* GetOpenFileWindow(const std::string& name);

    //void SetCurrentMapName(const std::string& filename);
    //const std::string& GetCurrentMapName();

    int GetActiveViewportIndex();
    int GetHoveredViewportIndex();
    int GetSelectedObjectIndex();
    int GetHoveredObjectIndex();
    bool IsEditorOpen();
    bool IsEditorClosed();
    bool IsViewportOrthographic(uint32_t viewportIndex);
    bool EditorIsIdle();
    bool EditorWasIdleLastFrame();
    float GetVerticalDividerXPos();
    float GetHorizontalDividerYPos();
    glm::vec3 GetMouseRayOriginByViewportIndex(int32_t viewportIndex);
    glm::vec3 GetMouseRayDirectionByViewportIndex(int32_t viewportIndex);
    glm::mat4 GetViewportViewMatrix(int32_t viewportIndex);
    float GetEditorOrthoSize(int32_t viewportIndex);
    std::string EditorObjectTypeToString(const ObjectType& type);
   // Camera* GetCameraByIndex(uint32_t index);
    Viewport* GetActiveViewport();
    ShadingMode GetViewportModeByIndex(uint32_t index);
    CameraView GetCameraViewByIndex(uint32_t index);
    ObjectType GetSelectedObjectType();
    ObjectType GetHoveredObjectType();
    EditorState GetEditorState();
    EditorViewportSplitMode GetEditorViewportSplitMode();
    SelectionRectangleState& GetSelectionRectangleState();
    EditorMode& GetEditorMode();

    EditorMesh& GetEditorMesh(); // BROKEN / 5% IMPLEMENTED

    std::string GetCurrentHeightMapName();

    // Dividers
    bool IsVerticalDividerHovered();
    bool IsHorizontalDividerHovered();
}