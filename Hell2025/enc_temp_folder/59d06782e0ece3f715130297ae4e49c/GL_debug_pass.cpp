#include "../GL_renderer.h" 
#include "Editor/Editor.h"
#include "Viewport/ViewportManager.h"
#include "Renderer/RenderDataManager.h"
#include "Renderer/Renderer.h"
#include "Physics/Physics.h"
#include "World/World.h"

#include "API/OpenGL/Types/GL_debug_mesh.hpp"
#include "API/OpenGL/Types/GL_mesh_buffer.h"

#include "Game/AStarMap.h"

#include "Input/Input.h"

namespace OpenGLRenderer {

    OpenGLDebugMesh g_debugMeshPoints;
    OpenGLDebugMesh g_debugMeshLines;
    OpenGLDebugMesh g_debugMeshDepthAwarePoints;
    OpenGLDebugMesh g_debugMeshDepthAwareLines;

    void RenderAStarDebugMesh();

    void DebugPass() {
        OpenGLShader* shader = GetShader("DebugVertex");
        OpenGLFrameBuffer* gBuffer = GetFrameBuffer("GBuffer");

        if (!gBuffer) return;
        if (!shader) return;

        const std::vector<ViewportData>& viewportData = RenderDataManager::GetViewportData();

        gBuffer->Bind();
        gBuffer->DrawBuffer("FinalLighting");

        glDisable(GL_DEPTH_TEST);
        glDisable(GL_CULL_FACE);
        glDisable(GL_BLEND);
        glPointSize(8.0f);

        shader->Bind();

        UpdateDebugMesh();

        for (int i = 0; i < 4; i++) {
            Viewport* viewport = ViewportManager::GetViewportByIndex(i);
            if (!viewport->IsVisible()) continue;

            OpenGLRenderer::SetViewport(gBuffer, viewport);
            shader->SetInt("u_viewportIndex", i);
            shader->SetMat4("u_projectionView", viewportData[i].projectionView);

            glDisable(GL_DEPTH_TEST);
            if (g_debugMeshLines.GetVertexCount() > 0) {
                glBindVertexArray(g_debugMeshLines.GetVAO());
                glDrawArrays(GL_LINES, 0, g_debugMeshLines.GetVertexCount());
            }
            if (g_debugMeshPoints.GetVertexCount() > 0) {
                glBindVertexArray(g_debugMeshPoints.GetVAO());
                glDrawArrays(GL_POINTS, 0, g_debugMeshPoints.GetVertexCount());
            }

            glEnable(GL_DEPTH_TEST);
            if (g_debugMeshDepthAwareLines.GetVertexCount() > 0) {
                glBindVertexArray(g_debugMeshDepthAwareLines.GetVAO());
                glDrawArrays(GL_LINES, 0, g_debugMeshDepthAwareLines.GetVertexCount());
            }
            if (g_debugMeshDepthAwarePoints.GetVertexCount() > 0) {
                glBindVertexArray(g_debugMeshPoints.GetVAO());
                glDrawArrays(GL_POINTS, 0, g_debugMeshPoints.GetVertexCount());
            }


        }

        RenderAStarDebugMesh();
    }

    void RenderAStarDebugMesh() {
        const std::vector<ViewportData>& viewportData = RenderDataManager::GetViewportData();
        OpenGLMeshBuffer& debugGridMesh = AStarMap::GetDebugGridMeshBuffer().GetGLMeshBuffer();
        OpenGLMeshBuffer& debugSolidMesh = AStarMap::GetDebugSolidMeshBuffer().GetGLMeshBuffer();

        OpenGLShader* solidColorShader = GetShader("DebugSolidColor");
        if (!solidColorShader) return;

        solidColorShader->Bind();
        solidColorShader->SetMat4("u_model", glm::mat4(1));
        solidColorShader->SetVec3("u_color", WHITE);


        // Line mesh
        if (debugGridMesh.GetIndexCount() > 0) {
            glDisable(GL_DEPTH_TEST);
            glBindVertexArray(debugGridMesh.GetVAO());
            for (int i = 0; i < 4; i++) {
                Viewport* viewport = ViewportManager::GetViewportByIndex(i);
                if (!viewport->IsVisible()) continue;

                solidColorShader->SetMat4("u_projectionView", viewportData[i].projectionView);
                glDrawElements(GL_LINES, debugGridMesh.GetIndexCount(), GL_UNSIGNED_INT, 0);
            }
        }

        // Solid mesh
        if (debugSolidMesh.GetIndexCount() > 0) {
            glEnable(GL_DEPTH_TEST);
            Transform transform;
            transform.position.x = 11.0f;
            solidColorShader->SetMat4("u_model", transform.to_mat4());
            glBindVertexArray(debugSolidMesh.GetVAO());

            for (int i = 0; i < 4; i++) {
                Viewport* viewport = ViewportManager::GetViewportByIndex(i);
                if (!viewport->IsVisible()) continue;

                solidColorShader->SetMat4("u_projectionView", viewportData[i].projectionView);
                glDrawElements(GL_TRIANGLES, debugSolidMesh.GetIndexCount(), GL_UNSIGNED_INT, 0);
            }
        }
    }

    void DebugViewPass() {
        RendererSettings& rendererSettings = Renderer::GetCurrentRendererSettings();

        // Tile based deferred heat map
        if (rendererSettings.rendererOverrideState == RendererOverrideState::TILE_HEATMAP) {
            OpenGLFrameBuffer* finalImageFBO = GetFrameBuffer("FinalImage");
            OpenGLFrameBuffer* gBuffer = GetFrameBuffer("GBuffer");
            OpenGLShader* shader = GetShader("DebugTileView");

            if (!finalImageFBO) return;
            if (!gBuffer) return;
            if (!shader) return;

            shader->Bind();
            shader->SetFloat("u_viewportWidth", gBuffer->GetWidth());
            shader->SetFloat("u_viewportHeight", gBuffer->GetHeight());
            shader->SetInt("u_tileXCount", gBuffer->GetWidth() / TILE_SIZE);
            shader->SetInt("u_tileYCount", gBuffer->GetHeight() / TILE_SIZE);

            glBindImageTexture(0, gBuffer->GetColorAttachmentHandleByName("FinalLighting"), 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA16F);
            glBindTextureUnit(1, gBuffer->GetDepthAttachmentHandle());
            glBindTextureUnit(2, finalImageFBO->GetColorAttachmentHandleByName("ViewportIndex"));

            glDispatchCompute(gBuffer->GetWidth() / TILE_SIZE, gBuffer->GetHeight() / TILE_SIZE, 1);
        }
        // Other modes
        if (rendererSettings.rendererOverrideState == RendererOverrideState::BASE_COLOR ||
            rendererSettings.rendererOverrideState == RendererOverrideState::NORMALS ||
            rendererSettings.rendererOverrideState == RendererOverrideState::RMA ||
            rendererSettings.rendererOverrideState == RendererOverrideState::CAMERA_NDOTL) {

            OpenGLFrameBuffer* finalImageFBO = GetFrameBuffer("FinalImage");
            OpenGLFrameBuffer* gBuffer = GetFrameBuffer("GBuffer");
            OpenGLShader* shader = GetShader("DebugView");

            if (!finalImageFBO) return;
            if (!gBuffer) return;
            if (!shader) return;

            shader->Bind();
            shader->SetFloat("u_brushSize", Editor::GetHeightMapBrushSize());
            shader->SetBool("u_heightMapEditor", (Editor::GetEditorMode() == EditorMode::HEIGHTMAP_EDITOR) && Editor::IsOpen());
            glBindImageTexture(0, gBuffer->GetColorAttachmentHandleByName("FinalLighting"), 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA16F);

            glBindTextureUnit(1, gBuffer->GetColorAttachmentHandleByName("BaseColor"));
            glBindTextureUnit(2, gBuffer->GetColorAttachmentHandleByName("Normal"));
            glBindTextureUnit(3, gBuffer->GetColorAttachmentHandleByName("RMA"));
            glBindTextureUnit(4, gBuffer->GetColorAttachmentHandleByName("WorldPosition"));
            glBindTextureUnit(5, finalImageFBO->GetColorAttachmentHandleByName("ViewportIndex"));

            glDispatchCompute(gBuffer->GetWidth() / TILE_SIZE, gBuffer->GetHeight() / TILE_SIZE, 1);
        }
    }

    void DrawPoint(glm::vec3 position, glm::vec3 color, bool obeyDepth, int exclusiveViewportIndex) {
        if (obeyDepth) {
            g_pointsDepthAware.push_back(DebugVertex(position, color, glm::ivec2(0,0), exclusiveViewportIndex));
        }
        else {
            g_points.push_back(DebugVertex(position, color, glm::ivec2(0, 0), exclusiveViewportIndex));
        }
    }

    void DrawLine(glm::vec3 begin, glm::vec3 end, glm::vec3 color, bool obeyDepth, int exclusiveViewportIndex) {
        DebugVertex v0 = DebugVertex(begin, color, glm::ivec2(0, 0), exclusiveViewportIndex);
        DebugVertex v1 = DebugVertex(end, color, glm::ivec2(0, 0), exclusiveViewportIndex);
        if (obeyDepth) {
            g_linesDepthAware.push_back(v0);
            g_linesDepthAware.push_back(v1);
        }
        else {
            g_lines.push_back(v0);
            g_lines.push_back(v1);
        }
    }

    void DrawAABB(const AABB& aabb, const glm::vec3& color) {
        glm::vec3 FrontTopLeft = glm::vec3(aabb.GetBoundsMin().x, aabb.GetBoundsMax().y, aabb.GetBoundsMax().z);
        glm::vec3 FrontTopRight = glm::vec3(aabb.GetBoundsMax().x, aabb.GetBoundsMax().y, aabb.GetBoundsMax().z);
        glm::vec3 FrontBottomLeft = glm::vec3(aabb.GetBoundsMin().x, aabb.GetBoundsMin().y, aabb.GetBoundsMax().z);
        glm::vec3 FrontBottomRight = glm::vec3(aabb.GetBoundsMax().x, aabb.GetBoundsMin().y, aabb.GetBoundsMax().z);
        glm::vec3 BackTopLeft = glm::vec3(aabb.GetBoundsMin().x, aabb.GetBoundsMax().y, aabb.GetBoundsMin().z);
        glm::vec3 BackTopRight = glm::vec3(aabb.GetBoundsMax().x, aabb.GetBoundsMax().y, aabb.GetBoundsMin().z);
        glm::vec3 BackBottomLeft = glm::vec3(aabb.GetBoundsMin().x, aabb.GetBoundsMin().y, aabb.GetBoundsMin().z);
        glm::vec3 BackBottomRight = glm::vec3(aabb.GetBoundsMax().x, aabb.GetBoundsMin().y, aabb.GetBoundsMin().z);
        DrawLine(FrontTopLeft, FrontTopRight, color);
        DrawLine(FrontBottomLeft, FrontBottomRight, color);
        DrawLine(BackTopLeft, BackTopRight, color);
        DrawLine(BackBottomLeft, BackBottomRight, color);
        DrawLine(FrontTopLeft, FrontBottomLeft, color);
        DrawLine(FrontTopRight, FrontBottomRight, color);
        DrawLine(BackTopLeft, BackBottomLeft, color);
        DrawLine(BackTopRight, BackBottomRight, color);
        DrawLine(FrontTopLeft, BackTopLeft, color);
        DrawLine(FrontTopRight, BackTopRight, color);
        DrawLine(FrontBottomLeft, BackBottomLeft, color);
        DrawLine(FrontBottomRight, BackBottomRight, color);
    }

    void DrawAABB(const AABB& aabb, const glm::vec3& color, const glm::mat4& worldTransform) {
        glm::vec3 FrontTopLeft = worldTransform * glm::vec4(aabb.GetBoundsMin().x, aabb.GetBoundsMax().y, aabb.GetBoundsMax().z, 1.0f);
        glm::vec3 FrontTopRight = worldTransform * glm::vec4(aabb.GetBoundsMax().x, aabb.GetBoundsMax().y, aabb.GetBoundsMax().z, 1.0f);
        glm::vec3 FrontBottomLeft = worldTransform * glm::vec4(aabb.GetBoundsMin().x, aabb.GetBoundsMin().y, aabb.GetBoundsMax().z, 1.0f);
        glm::vec3 FrontBottomRight = worldTransform * glm::vec4(aabb.GetBoundsMax().x, aabb.GetBoundsMin().y, aabb.GetBoundsMax().z, 1.0f);
        glm::vec3 BackTopLeft = worldTransform * glm::vec4(aabb.GetBoundsMin().x, aabb.GetBoundsMax().y, aabb.GetBoundsMin().z, 1.0f);
        glm::vec3 BackTopRight = worldTransform * glm::vec4(aabb.GetBoundsMax().x, aabb.GetBoundsMax().y, aabb.GetBoundsMin().z, 1.0f);
        glm::vec3 BackBottomLeft = worldTransform * glm::vec4(aabb.GetBoundsMin().x, aabb.GetBoundsMin().y, aabb.GetBoundsMin().z, 1.0f);
        glm::vec3 BackBottomRight = worldTransform * glm::vec4(aabb.GetBoundsMax().x, aabb.GetBoundsMin().y, aabb.GetBoundsMin().z, 1.0f);
        DrawLine(FrontTopLeft, FrontTopRight, color);
        DrawLine(FrontBottomLeft, FrontBottomRight, color);
        DrawLine(BackTopLeft, BackTopRight, color);
        DrawLine(BackBottomLeft, BackBottomRight, color);
        DrawLine(FrontTopLeft, FrontBottomLeft, color);
        DrawLine(FrontTopRight, FrontBottomRight, color);
        DrawLine(BackTopLeft, BackBottomLeft, color);
        DrawLine(BackTopRight, BackBottomRight, color);
        DrawLine(FrontTopLeft, BackTopLeft, color);
        DrawLine(FrontTopRight, BackTopRight, color);
        DrawLine(FrontBottomLeft, BackBottomLeft, color);
        DrawLine(FrontBottomRight, BackBottomRight, color);
    }

    void UpdateDebugMesh() {
        g_debugMeshPoints.UpdateVertexData(g_points);
        g_debugMeshDepthAwarePoints.UpdateVertexData(g_pointsDepthAware);
        g_debugMeshLines.UpdateVertexData(g_lines);
        g_debugMeshDepthAwareLines.UpdateVertexData(g_linesDepthAware);

        g_points.clear();
        g_lines.clear();
        g_pointsDepthAware.clear();
        g_linesDepthAware.clear();
    }
}
