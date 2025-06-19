#include "Renderer.h"
#include "API/OpenGL/Renderer/GL_renderer.h"
#include "API/Vulkan/Renderer/VK_renderer.h"
#include "BackEnd/BackEnd.h"
#include "Editor/Editor.h"
#include "Timer.hpp"

namespace Renderer {

    struct RendererSettingsSet {
        RendererSettings game;
        RendererSettings heightMapEditor;
        RendererSettings houseEditor;
        RendererSettings mapEditor;
        RendererSettings sectorEditor;
    } g_rendererSettingsSet;

    void InitMain() {
        if (BackEnd::GetAPI() == API::OPENGL) {
            OpenGLRenderer::InitMain();
        }
        else if (BackEnd::GetAPI() == API::VULKAN) {
            // TODO: VulkanRenderer::InitMain();
        }

        g_rendererSettingsSet.houseEditor.rendererOverrideState = RendererOverrideState::CAMERA_NDOTL;
        g_rendererSettingsSet.houseEditor.drawGrass = false;

        g_rendererSettingsSet.mapEditor.rendererOverrideState = RendererOverrideState::NORMALS;
        g_rendererSettingsSet.mapEditor.drawGrass = false;

        g_rendererSettingsSet.heightMapEditor.rendererOverrideState = RendererOverrideState::CAMERA_NDOTL;
        g_rendererSettingsSet.heightMapEditor.drawGrass = false;

        g_rendererSettingsSet.sectorEditor.rendererOverrideState = RendererOverrideState::BASE_COLOR;
        g_rendererSettingsSet.sectorEditor.drawGrass = true;
    }

    void RenderLoadingScreen() {
        if (BackEnd::GetAPI() == API::OPENGL) {
            OpenGLRenderer::RenderLoadingScreen();
        }
        else if (BackEnd::GetAPI() == API::VULKAN) {
            VulkanRenderer::RenderLoadingScreen();
        }
    }

    void PreGameLogicComputePasses() {
        if (BackEnd::GetAPI() == API::OPENGL) {
            OpenGLRenderer::PreGameLogicComputePasses();
        }
        else if (BackEnd::GetAPI() == API::VULKAN) {
            // TODO: VulkanRenderer::PreGameLogicComputePasses();
        }
    }

    void RenderGame() {
        if (BackEnd::GetAPI() == API::OPENGL) {
            OpenGLRenderer::RenderGame();
        }
        else if (BackEnd::GetAPI() == API::VULKAN) {
            // TODO: VulkanRenderer::RenderLoadingScreen();
        }
    }

    void HotloadShaders() {
        if (BackEnd::GetAPI() == API::OPENGL) {
            OpenGLRenderer::HotloadShaders();
        }
        else if (BackEnd::GetAPI() == API::VULKAN) {
            //VulkanRenderer::HotloadShaders();
        }
    }

    void RecalculateAllHeightMapData() {
        if (BackEnd::GetAPI() == API::OPENGL) {
            OpenGLRenderer::RecalculateAllHeightMapData();
        }
        else if (BackEnd::GetAPI() == API::VULKAN) {
            // TODO
        }
    }

    void DrawPoint(glm::vec3 position, glm::vec3 color, bool obeyDepth, int exclusiveViewportIndex) {
        if (BackEnd::GetAPI() == API::OPENGL) {
            OpenGLRenderer::DrawPoint(position, color, obeyDepth, exclusiveViewportIndex);
        }
        else if (BackEnd::GetAPI() == API::VULKAN) {
            // TODO
        }
    }

    void DrawLine(glm::vec3 begin, glm::vec3 end, glm::vec3 color, bool obeyDepth, int exclusiveViewportIndex, int ignoredViewportIndex) {
        if (BackEnd::GetAPI() == API::OPENGL) {
            OpenGLRenderer::DrawLine(begin, end, color, obeyDepth, exclusiveViewportIndex, ignoredViewportIndex);
        }
        else if (BackEnd::GetAPI() == API::VULKAN) {
            // TODO
        }
    }

    void DrawAABB(const AABB& aabb, const glm::vec3& color) {
        if (BackEnd::GetAPI() == API::OPENGL) {
            OpenGLRenderer::DrawAABB(aabb, color);
        }
        else if (BackEnd::GetAPI() == API::VULKAN) {
            // TODO
        }
    }

    void DrawAABB(const AABB& aabb, const glm::vec3& color, const glm::mat4& worldTransform) {
        if (BackEnd::GetAPI() == API::OPENGL) {
            OpenGLRenderer::DrawAABB(aabb, color, worldTransform);
        }
        else if (BackEnd::GetAPI() == API::VULKAN) {
            // TODO
        }
    }

    RendererSettings& GetCurrentRendererSettings() {
        if (Editor::IsOpen()) {
            switch (Editor::GetEditorMode()) {
                case EditorMode::HEIGHTMAP_EDITOR: return g_rendererSettingsSet.heightMapEditor;
                case EditorMode::HOUSE_EDITOR:     return g_rendererSettingsSet.houseEditor;
                case EditorMode::SECTOR_EDITOR:    return g_rendererSettingsSet.sectorEditor;
                case EditorMode::MAP_EDITOR:       return g_rendererSettingsSet.mapEditor;
            }
        }
        return g_rendererSettingsSet.game;
    }

    void NextRendererOverrideState() {
        RendererSettings& rendererSettings = GetCurrentRendererSettings();
        int i = static_cast<int>(rendererSettings.rendererOverrideState);
        i = (i + 1) % static_cast<int>(RendererOverrideState::STATE_COUNT);
        rendererSettings.rendererOverrideState = static_cast<RendererOverrideState>(i);
    }
}