#pragma once
#include "HellTypes.h"
#include "Callbacks/Callbacks.h"
#include "CreateInfo.h"
#include "Camera/Camera.h"
#include "Viewport/Viewport.h"
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
    void SetEditorMode(EditorMode editorMode);
    void SetActiveViewportIndex(int index);
    //void SetSelectedObjectIndex(int index);
    //void SetHoveredObjectIndex(int index);
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
    //void UpdateObjectSelection();


    void UpdateGizmoInteract();

    // Object hover
    void UpdateObjectHover();
    void SetHoveredObjectType(ObjectType objectType);
    void SetHoveredObjectId(uint64_t objectId);
    ObjectType GetHoveredObjectType();
    uint64_t GetHoveredObjectId();

    // Object selection
    void UnselectAnyObject();
    void UpdateObjectSelection();
    void SetSelectedObjectType(ObjectType objectType);
    void SetSelectedObjectId(uint64_t objectId);
    ObjectType GetSelectedObjectType();
    uint64_t GetSelectedObjectId();

    // Gizmo shit
    void UpdateObjectGizmoInteraction();

    // Axis constraint
    void ResetAxisConstraint();
    void SetAxisConstraint(Axis axis);
    Axis GetAxisConstraint();

    // Height Map Editor
    void InitHeightMapEditor();
    void OpenHeightMapEditor();
    void LoadHeightMap(const std::string& heightMapName);
    void UpdateHeightMapEditor();
    void ShowNewHeightMapWindow();
    void ShowOpenHeightMapWindow();
    void CloseAllHeightMapEditorWindows();
    void CreateHeigthMapEditorImGuiElements();

    int GetHeightMapPaintMode();
    float GetHeightMapBrushSize();
    float GetHeightMapBrushStrength();
    float GetHeightMapNoiseStrength();
    float GetHeightMapMinPaintHeight();
    float GetHeightMapMaxPaintHeight();
     
    // House Editor
    void InitHouseEditor();
    void OpenHouseEditor();
    void UpdateHouseEditor();
    void ShowNewHouseWindow();
    void ShowOpenHouseWindow();
    void CloseAllHouseEditorWindows();
    void CreateHouseEditorImGuiElements();
    void EnterWallPlacementState();

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
    //int GetSelectedObjectIndex();
    //int GetHoveredObjectIndex();
    bool IsOpen();
    bool IsClosed();
    bool IsViewportOrthographic(uint32_t viewportIndex);
    bool EditorIsIdle();
    bool EditorWasIdleLastFrame();
    float GetVerticalDividerXPos();
    float GetHorizontalDividerYPos();
    glm::vec3 GetMouseRayOriginByViewportIndex(int32_t viewportIndex);
    glm::vec3 GetMouseRayDirectionByViewportIndex(int32_t viewportIndex);
    glm::mat4 GetViewportViewMatrix(int32_t viewportIndex);
    float GetEditorOrthoSize(int32_t viewportIndex);
    Viewport* GetActiveViewport();
    ShadingMode GetViewportModeByIndex(uint32_t index);
    CameraView GetCameraViewByIndex(uint32_t index);
    //ObjectType GetSelectedObjectType();
    //ObjectType GetHoveredObjectType();
    EditorState GetEditorState();
    EditorViewportSplitMode GetEditorViewportSplitMode();
    SelectionRectangleState& GetSelectionRectangleState();
    EditorMode& GetEditorMode();
    Axis GetAxisConstraint();

    std::string GetCurrentHeightMapName();

    // Dividers
    bool IsVerticalDividerHovered();
    bool IsHorizontalDividerHovered();
}