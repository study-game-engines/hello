#include "API/OpenGL//Renderer/GL_renderer.h"
#include "AssetManagement/AssetManager.h"
#include "Editor/Editor.h"
#include "Ocean/Ocean.h"
#include "World/World.h"

#include "Renderer/RenderDataManager.h"
#include "Viewport/ViewportManager.h"

namespace OpenGLRenderer {

    void OceanPass() {

        const ViewportData& viewportData = RenderDataManager::GetViewportData()[0];

        glm::mat4 projectionMatrix = viewportData.projection;
        glm::mat4 viewMatrix = viewportData.view;
        glm::vec3 viewPos = viewportData.viewPos;

        OpenGLCubemapView* skyboxCubemapView = GetCubemapView("SkyboxNightSky");
        OpenGLFrameBuffer* gBuffer = GetFrameBuffer("GBuffer");
        OpenGLFrameBuffer* waterFrameBuffer = GetFrameBuffer("Water");
        OpenGLMeshPatch* oceanMeshPatch = GetOceanMeshPatch();
        OpenGLShader* shader = GetShader("OceanColor");
        OpenGLShader* compositeShader = GetShader("OceanComposite");

        if (!oceanMeshPatch) return;
        if (!skyboxCubemapView) return;
        if (!gBuffer) return;
        if (!waterFrameBuffer) return;
        if (!shader) return;
        if (!compositeShader) return;

        const float waterHeight = Ocean::GetWaterHeight();
        int patchCount = 16;
        //float scale = Ocean::GetOceanSize().x / 32.0f;
        float scale = 0.03125f;
        float patchOffset = Ocean::GetOceanSize().y * scale;

        Transform patchTransform;
        patchTransform.scale = glm::vec3(scale);

        OpenGLRenderer::BlitFrameBufferDepth(gBuffer, waterFrameBuffer);

        waterFrameBuffer->Bind();
        waterFrameBuffer->SetViewport();
        waterFrameBuffer->DrawBuffers({ "Diffuse", "Specular" });

        shader->Bind();
        shader->SetInt("environmentMap", 0);
        shader->SetVec3("eyePos", viewPos);
        shader->SetMat4("view", viewMatrix);
        //shader->SetMat4("projection", projectionMatrix);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxCubemapView->GetHandle());

        glBindVertexArray(oceanMeshPatch->GetVAO());

        glMemoryBarrier(GL_VERTEX_ATTRIB_ARRAY_BARRIER_BIT);

        //shader->SetMat4("model", glm::mat4(1.0f));
        //glDrawElements(GL_TRIANGLE_STRIP, oceanMeshPatch->GetIndexCount(), GL_UNSIGNED_INT, nullptr);
        //glDrawElements(GL_POINTS, oceanMeshPatch->GetIndexCount(), GL_UNSIGNED_INT, nullptr);

        glm::vec3 originOffset = glm::vec3(0.0f);

        // Offset water origin when in heightmap editor
        if (Editor::IsOpen() && Editor::GetEditorMode() == EditorMode::HEIGHTMAP_EDITOR) {
            originOffset = glm::vec3(64.0f, 0.0f, 64.0f);
        }

        for (int i = 0; i < 1; i++) {
            Viewport* viewport = ViewportManager::GetViewportByIndex(i);
            if (viewport->IsVisible()) {
                OpenGLRenderer::SetViewport(waterFrameBuffer, viewport);

                for (int x = 0; x < patchCount; x++) {
                    for (int z = 0; z < patchCount; z++) {
                        patchTransform.position = glm::vec3(patchOffset * x, waterHeight, patchOffset * z);
                        patchTransform.position += originOffset;
                        shader->SetMat4("u_model", patchTransform.to_mat4());
                        glDrawElements(GL_TRIANGLE_STRIP, oceanMeshPatch->GetIndexCount(), GL_UNSIGNED_INT, nullptr);
                    }
                }

            }
        }

        glm::vec3 bot = glm::vec3(0, 0, 0);
        glm::vec3 top = glm::vec3(0, waterHeight, 0);

        DrawPoint(bot, RED);
        DrawPoint(top, RED);
        DrawLine(bot, top, RED);

        // Composite the water result atop the lighting texture
        compositeShader->Bind();
        glBindImageTexture(0, gBuffer->GetColorAttachmentHandleByName("FinalLighting"), 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA16F);
        glBindTextureUnit(0, waterFrameBuffer->GetColorAttachmentHandleByName("Diffuse"));
        glBindTextureUnit(1, waterFrameBuffer->GetColorAttachmentHandleByName("Specular"));
        glDispatchCompute(gBuffer->GetWidth() / TILE_SIZE, gBuffer->GetHeight() / TILE_SIZE, 1);
    }

}