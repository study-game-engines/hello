#pragma once
#include "HellTypes.h"
#include "Math/AABB.h"

namespace Renderer {
    void InitMain();
    void RenderLoadingScreen();
    void RenderGame();
    void HotloadShaders();
    void NextRendererOverrideState();

    void RecalculateAllHeightMapData();

    void DrawPoint(glm::vec3 position, glm::vec3 color, bool obeyDepth = false, int exclusiveViewportIndex = -1);
    void DrawLine(glm::vec3 begin, glm::vec3 end, glm::vec3 color, bool obeyDepth = false, int exclusiveViewportIndex = -1);
    void DrawAABB(const AABB& aabb, const glm::vec3& color);
    void DrawAABB(const AABB& aabb, const glm::vec3& color, const glm::mat4& worldTransform);

    RendererSettings& GetCurrentRendererSettings();
}