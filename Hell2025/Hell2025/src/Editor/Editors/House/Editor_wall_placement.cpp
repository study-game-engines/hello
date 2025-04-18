#include "Editor/Editor.h"
#include "Audio/Audio.h"
#include "Input/Input.h"
#include "Renderer/Renderer.h"
#include "Viewport/ViewportManager.h"
#include "World/World.h"

#include <glm/gtx/intersect.hpp>

namespace Editor {

    //WallCreateInfo g_wallCreateInfo;
    uint64_t g_wallId = 0;

    glm::vec3 GetMouseRayHitPosition();

    void EnterWallPlacementState() {
        SetEditorState(EditorState::WALL_PLACEMENT);
        //g_wallCreateInfo.points.clear();
    }

    void BeginWall() {
        UnselectAnyObject();
        WallCreateInfo createInfo;
        createInfo.points.push_back(GetMouseRayHitPosition());
        createInfo.points.push_back(GetMouseRayHitPosition() + glm::vec3(0.5f, 0.0, 0.5f));
        createInfo.materialName = "WallPaper";
        createInfo.materialName = "Ceiling2";
        createInfo.textureOffsetU = 0.0f;
        createInfo.textureOffsetV = -1.4f;
        createInfo.textureScale = 1 / 2.4f;
        createInfo.height = 2.4f;
        createInfo.ceilingTrimType = TrimType::TIMBER;
        createInfo.floorTrimType = TrimType::TIMBER;
        createInfo.useReversePointOrder = false;
        g_wallId = World::AddWall(createInfo);
        World::UpdateHouseMeshBuffer();
        Audio::PlayAudio(AUDIO_SELECT, 1.0f);
        std::cout << "Beginning wall!\n";
    }

    void CancelWallPlacement() {
        //World::RemoveWallByObjectId(g_wallId);
        g_wallId = 0;
        //g_wallCreateInfo.points.clear();
        SetEditorState(EditorState::IDLE);
    }

    void UpdateWallPlacement() {
        glm::vec3 rayHitPosition = GetMouseRayHitPosition();
        bool cusrsorAtWallOrigin = false;

        // Begin wall
        if (Input::LeftMousePressed() && g_wallId == 0) {
            BeginWall();
            return;
        }

        Wall* wall = World::GetWallByObjectId(g_wallId);
        if (!wall) return;

        std::cout << "wall point count: " << wall->GetPointCount() << "\n";

        // Apply axis constraint
        if (wall->GetPointCount() > 1) {
            int previousPointIndex = wall->GetPointCount() - 2;
            glm::vec3 previousPosition = wall->GetPointByIndex(previousPointIndex);

            if (GetAxisConstraint() == Axis::X) {
                rayHitPosition.y = previousPosition.y;
                rayHitPosition.z = previousPosition.z;
            }
            if (GetAxisConstraint() == Axis::Z) {
                rayHitPosition.x = previousPosition.x;
                rayHitPosition.y = previousPosition.y;
            }
        }

        // Snap to the first point if within range
        if (wall->GetPointCount() > 2) {
            float threshold = 0.25f;
            glm::vec3 firstPoint = wall->GetPointByIndex(0);
            if (glm::distance(rayHitPosition, firstPoint) < threshold) {
                rayHitPosition = firstPoint;
                cusrsorAtWallOrigin = true;
            }
        }

        // Complete wall
        if (Input::LeftMousePressed() && cusrsorAtWallOrigin) {
            Audio::PlayAudio(AUDIO_SELECT, 1.0f);
            SetEditorState(EditorState::IDLE);
            g_wallId = 0;
            return;
        }

        // Add wall segment
        if (Input::LeftMousePressed() && wall->GetPointCount() > 0) {
            glm::vec3 endPoint = wall->GetPointByIndex(wall->GetPointCount() -1);
            glm::vec3 newPoint = endPoint + glm::vec3(0.5f, 0.0, 0.5f);
            if (wall->AddPointToEnd(newPoint)) {
                World::UpdateHouseMeshBuffer();
                Audio::PlayAudio(AUDIO_SELECT, 1.0f);
                return;
            }
        }

        // Flip faces
        if (Input::KeyPressed(HELL_KEY_F)) {
            Audio::PlayAudio(AUDIO_SELECT, 1.0f);
            wall->FlipFaces();
            World::UpdateHouseMeshBuffer();
        }

        if (wall->GetPointCount() > 1) {
            // Move last point to mouse cursor
            int pointIndex = wall->GetPointCount() - 1;
            if (wall->UpdatePointPosition(pointIndex, rayHitPosition)) {
                World::UpdateHouseMeshBuffer();
            }
            //glm::vec3& lastSegmentPosition = g_wallCreateInfo.points.back();
            //if (glm::distance(rayHitPosition, lastSegmentPosition) > 0.1) {
            //    lastSegmentPosition = rayHitPosition;
            //    wall->UpdateSegmentsAndVertexData(g_wallCreateInfo);
            //    World::UpdateHouseMeshBuffer();
            //}

            // Draw lines and points
            glm::vec4 pointColor = cusrsorAtWallOrigin ? WHITE : OUTLINE_COLOR;
            wall->DrawSegmentVertices(pointColor);
            wall->DrawSegmentLines(WHITE);
        }
        
        // Bail
        if (Input::RightMouseDown()) {
            CancelWallPlacement();
        }
    }


    glm::vec3 GetMouseRayHitPosition() {
         // Cast ray into ground plane
        int viewportIndex = Editor::GetHoveredViewportIndex();
        const Viewport* viewport = ViewportManager::GetViewportByIndex(viewportIndex);
        const glm::vec3 rayOrigin = Editor::GetMouseRayOriginByViewportIndex(viewportIndex);
        const glm::vec3 rayDir = Editor::GetMouseRayDirectionByViewportIndex(viewportIndex);
        glm::vec3 planeNormal = glm::vec3(0.0f, 1.0f, 0.0f);
        glm::vec3 planeOrigin = glm::vec3(0.0f);
        float distanceToHit = 0;
        bool hitFound = glm::intersectRayPlane(rayOrigin, rayDir, planeOrigin, planeNormal, distanceToHit);
        
        if (!hitFound) {
            std::cout << "Edtor::UpdateWallPlacement() failed to find mouse ray\n";
            return glm::vec3(0.0f);
        }
        else {
            glm::vec3 hitPosition = rayOrigin + (rayDir * distanceToHit);

            // Snap to grid
            hitPosition = glm::round(hitPosition * 10.0f) / 10.0f;

            return hitPosition;
        }
    }
}