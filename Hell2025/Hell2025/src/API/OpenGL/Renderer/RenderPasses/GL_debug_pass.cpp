#include "../GL_renderer.h" 
#include "Editor/Editor.h"
#include "Viewport/ViewportManager.h"
#include "Renderer/RenderDataManager.h"
#include "Renderer/Renderer.h"
#include "Physics/Physics.h"
#include "World/World.h"

#include "API/OpenGL/Types/GL_debug_mesh.hpp"
#include "API/OpenGL/Types/GL_mesh_buffer.h"

namespace OpenGLRenderer {

    OpenGLDebugMesh g_debugMeshPoints;
    OpenGLDebugMesh g_debugMeshLines;
    OpenGLDebugMesh g_debugMeshDepthAwarePoints;
    OpenGLDebugMesh g_debugMeshDepthAwareLines;

    OpenGLMeshBuffer g_debugGrid;

    void InitDebugGrid() {


        int width = 10;
        int height = 10;

        int n = width;
        int m = height;

        std::vector<Vertex> vertices;
        std::vector<unsigned int> indices;

        vertices.resize(static_cast<size_t>(width) * height);

        // Positions
        float N_half = (static_cast<float>(n) - 1.0f) * 0.5f;
        float M_half = (static_cast<float>(m) - 1.0f) * 0.5f;

        for (int i = 0; i < m; ++i) {
            for (int j = 0; j < n; ++j) {
                vertices[i * n + j].position = glm::vec3(
                    static_cast<float>(j) - N_half,
                    0.0f,
                    static_cast<float>(i) - M_half
                );
                // Default normals
                vertices[i * n + j].normal = glm::vec3(0.0f, 1.0f, 0.0f);
            }
        }


        // Reserve an upper bound (approx)
        indices.reserve((n * (m - 1) * 2) + 2 * (m - 2));

        for (int i = 0; i < m - 1; ++i) {
            bool even = (i % 2) == 0;

            // build the vertical ladder for this row
            if (even) {
                for (int j = 0; j < n; ++j) {
                    indices.push_back(i * n + j);
                    indices.push_back((i + 1) * n + j);
                }
            }
            else {
                for (int j = n - 1; j >= 0; --j) {
                    indices.push_back((i + 1) * n + j);
                    indices.push_back(i * n + j);
                }
            }

            // insert two degenerates to restart without flipping
            if (i < m - 2) {

                // last vertex of this strip
                unsigned int last = indices.back();

                // first vertex of next strip
                unsigned int nextFirst = even
                    ? ((i + 1) * n + (n - 1))   // even ended at bottom right
                    : ((i + 1) * n + 0);        // odd  ended at top   left

                indices.push_back(last);
                indices.push_back(nextFirst);
            }
        }
        //indexCount = 2 * n * (m - 1) + 2 * (m - 2);
        //std::vector<glm::vec3>;
        //for (int i = 0;
        //g_debugGrid

        g_debugGrid.UpdateBuffers(vertices, indices);
    }

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

        //InitDebugGrid();
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
