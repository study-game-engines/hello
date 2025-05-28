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
    void SetSelectedObjectType(ObjectType editorObjectType);
    void SetHoveredObjectType(ObjectType editorObjectType);
    void SetSplitX(float value);
    void SetSplitY(float value);
    void SetViewportView(uint32_t viewportIndex, glm::vec3 viewportOrigin, CameraView targetView);
    void SetEditorState(EditorState editorState);
    void SetViewportOrthographicState(uint32_t viewportIndex, bool state);
    void SetViewportOrthoSize(uint32_t viewportIndex, float size);
    void SetEditorViewportSplitMode(EditorViewportSplitMode mode);

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

    float GetHeightMapNoiseScale();
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
    void CreateSectorEditorImGuiElements();
    void ShowNewSectorWindow();
    void ShowOpenSectorWindow();
    void CloseAllSectorEditorWindows();

    void CloseAllEditorWindows();

    // Object placement
    void UpdateDoorPlacement();
    void UpdatePictureFramePlacement();
    void UpdateTreePlacement();
    void UpdateWallPlacement();
    void UpdateWindowPlacement();
    void UpdateObjectPlacement();
    void ExitObjectPlacement();
    void SetPlacementObjectId(uint64_t objectId);
    uint64_t GetPlacementObjectId();

    // Ray intersections
    glm::vec3 GetMouseRayPlaneIntersectionPoint(glm::vec3 planeOrigin, glm::vec3 planeNormal);
    PhysXRayResult GetEditorPhysXMouseRayHit();

    int GetActiveViewportIndex();
    int GetHoveredViewportIndex();
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
    EditorState GetEditorState();
    EditorViewportSplitMode GetEditorViewportSplitMode();
    SelectionRectangleState& GetSelectionRectangleState();
    EditorMode& GetEditorMode();
    Axis GetAxisConstraint();

    const std::string& GetSectorName();
    const std::string& GetHeightMapName();

    // Saving/Loading
    void SaveSector();
    void LoadSectorFromDisk(const std::string& sectorName);

    // Dividers
    bool IsVerticalDividerHovered();
    bool IsHorizontalDividerHovered();
}