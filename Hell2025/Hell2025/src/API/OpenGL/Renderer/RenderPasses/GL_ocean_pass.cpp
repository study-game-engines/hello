#include "API/OpenGL//Renderer/GL_renderer.h"
#include "AssetManagement/AssetManager.h"
#include "Editor/Editor.h"
#include "Core/Game.h"
#include "Ocean/Ocean.h"
#include "World/World.h"

#include "Renderer/RenderDataManager.h"
#include "Viewport/ViewportManager.h"

namespace OpenGLRenderer {

    void OceanPass() {
        if (!World::HasOcean()) {
            return;
        }

        const ViewportData& viewportData = RenderDataManager::GetViewportData()[0];

        glm::vec3 viewPos = viewportData.viewPos;

        OpenGLCubemapView* skyboxCubemapView = GetCubemapView("SkyboxNightSky");
        OpenGLFrameBuffer* gBuffer = GetFrameBuffer("GBuffer");
        OpenGLFrameBuffer* waterFrameBuffer = GetFrameBuffer("Water");
        OpenGLMeshPatch* oceanMeshPatch = GetOceanMeshPatch();
        OpenGLShader* shader = GetShader("OceanColor");
        OpenGLShader* compositeShader = GetShader("OceanComposite");
        OpenGLSSBO* oceanPatchTransformsSSBO = GetSSBO("OceanPatchTransforms");

        if (!oceanMeshPatch) return;
        if (!skyboxCubemapView) return;
        if (!gBuffer) return;
        if (!waterFrameBuffer) return;
        if (!shader) return;
        if (!compositeShader) return;
        if (!oceanPatchTransformsSSBO) return;

        OpenGLRenderer::BlitFrameBufferDepth(gBuffer, waterFrameBuffer);

        waterFrameBuffer->Bind();
        waterFrameBuffer->SetViewport();
        waterFrameBuffer->DrawBuffers({ "Diffuse", "Specular" });

        shader->Bind();
        shader->SetInt("environmentMap", 0);
        shader->SetVec3("eyePos", viewPos);

        oceanPatchTransformsSSBO->Bind(6);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxCubemapView->GetHandle());

        glBindVertexArray(oceanMeshPatch->GetVAO());

        glMemoryBarrier(GL_VERTEX_ATTRIB_ARRAY_BARRIER_BIT);

        Viewport* viewport = ViewportManager::GetViewportByIndex(0);
        Frustum& frustum = viewport->GetFrustum();

        // Render ocean geometry and thus diffuse and specular color
        for (int i = 0; i < 1; i++) {
            Viewport* viewport = ViewportManager::GetViewportByIndex(i);
            if (viewport->IsVisible()) {
                OpenGLRenderer::SetViewport(waterFrameBuffer, viewport);
                int instanceCount = RenderDataManager::GetOceanPatchTransforms().size();

                shader->SetFloat("u_normalModifier", 1.0f);
                glCullFace(GL_BACK);
                glDrawElementsInstanced(GL_TRIANGLE_STRIP, oceanMeshPatch->GetIndexCount(), GL_UNSIGNED_INT, nullptr, instanceCount);

                shader->SetFloat("u_normalModifier", -1.0f);
                glCullFace(GL_FRONT);
                glDrawElementsInstanced(GL_TRIANGLE_STRIP, oceanMeshPatch->GetIndexCount(), GL_UNSIGNED_INT, nullptr, instanceCount);
            }
        }

        // Cleanup
        glCullFace(GL_BACK);

        // Composite the water result atop the lighting texture
        compositeShader->Bind();
        glBindImageTexture(0, gBuffer->GetColorAttachmentHandleByName("FinalLighting"), 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA16F);
        glBindTextureUnit(0, waterFrameBuffer->GetColorAttachmentHandleByName("Diffuse"));
        glBindTextureUnit(1, waterFrameBuffer->GetColorAttachmentHandleByName("Specular"));
        glDispatchCompute(gBuffer->GetWidth() / TILE_SIZE, gBuffer->GetHeight() / TILE_SIZE, 1);
    }

}