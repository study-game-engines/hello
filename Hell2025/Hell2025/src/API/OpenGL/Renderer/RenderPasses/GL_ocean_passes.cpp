#include "API/OpenGL//Renderer/GL_renderer.h"
#include "AssetManagement/AssetManager.h"
#include "Editor/Editor.h"
#include "Core/Game.h"
#include "Ocean/Ocean.h"
#include "World/World.h"

#include "Renderer/RenderDataManager.h"
#include "Viewport/ViewportManager.h"


#include "Input/Input.h"



namespace OpenGLRenderer {

    void OceanGeometryPass() {
        if (!World::HasOcean()) {
            return;
        }

        OpenGLFrameBuffer* gBuffer = GetFrameBuffer("GBuffer");
        OpenGLFrameBuffer* waterFrameBuffer = GetFrameBuffer("Water");
        OpenGLFrameBuffer* fftFrameBuffer_band0 = GetFrameBuffer("FFT_band0");
        OpenGLFrameBuffer* fftFrameBuffer_band1 = GetFrameBuffer("FFT_band1");
        OpenGLCubemapView* skyboxCubemapView = GetCubemapView("SkyboxNightSky");
        OpenGLMeshPatch* oceanMeshPatch = GetOceanMeshPatch();
        OpenGLShader* shader = GetShader("OceanGeometry");

        if (!gBuffer) return;
        if (!waterFrameBuffer) return;
        if (!fftFrameBuffer_band0) return;
        if (!fftFrameBuffer_band1) return;
        if (!skyboxCubemapView) return;
        if (!oceanMeshPatch) return;
        if (!shader) return;
        
        static bool wireframe = false;
        static bool swap = false;
        static bool test = false;

        if (Input::KeyPressed(HELL_KEY_8)) {
            test = !test;
        }
        if (Input::KeyPressed(HELL_KEY_9)) {
            wireframe = !wireframe;
        }
        if (Input::KeyPressed(HELL_KEY_0)) {
            swap = !swap;
        }


        float scale = 0.05;

        int min = -20;
        int max = 20;
        float offset = (max - min) * Ocean::GetBaseFFTResolution().x * scale;

        if (test) {
            min = 0;
            max = 1;
            offset = Ocean::GetBaseFFTResolution().x * scale;
        }

        const ViewportData& viewportData = RenderDataManager::GetViewportData()[0];
        glm::mat4 projectionMatrix = viewportData.projection;
        glm::mat4 viewMatrix = viewportData.view;
        glm::vec3 viewPos = viewportData.viewPos;
        glm::mat4 projectionView = viewportData.projectionView;

        float patchOffset = Ocean::GetBaseFFTResolution().y * scale;


        //DrawPoint(glm::vec3(0, -0.65f, 0), WHITE);
        //DrawPoint(glm::vec3(patchOffset, -0.65f, 0), WHITE);

        Transform tesseleationTransform;
        tesseleationTransform.scale = glm::vec3(scale);

        OpenGLRenderer::BlitFrameBufferDepth(gBuffer, waterFrameBuffer);

        waterFrameBuffer->Bind();
        waterFrameBuffer->SetViewport();
        waterFrameBuffer->DrawBuffers({ "Color", "UnderwaterMask", "WorldPosition" });

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, fftFrameBuffer_band0->GetColorAttachmentHandleByName("Displacement"));
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, fftFrameBuffer_band0->GetColorAttachmentHandleByName("Normals"));
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, fftFrameBuffer_band1->GetColorAttachmentHandleByName("Displacement"));
        glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_2D, fftFrameBuffer_band1->GetColorAttachmentHandleByName("Normals"));
        glActiveTexture(GL_TEXTURE4);
        glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxCubemapView->GetHandle());

        glActiveTexture(GL_TEXTURE5);
        glBindTexture(GL_TEXTURE_2D, gBuffer->GetColorAttachmentHandleByName("WorldSpacePosition"));

        glPolygonMode(GL_FRONT_AND_BACK, wireframe ? GL_LINE : GL_FILL);
        glMemoryBarrier(GL_VERTEX_ATTRIB_ARRAY_BARRIER_BIT);

        glEnable(GL_CULL_FACE);
        glEnable(GL_DEPTH_TEST);

        // Tessellated ocean
        tesseleationTransform.position.x = -patchOffset;
        shader->Bind();
        shader->SetMat4("u_projectionView", projectionView);
        shader->SetVec3("u_wireframeColor", GREEN);
        shader->SetMat4("u_model", tesseleationTransform.to_mat4());
        shader->SetInt("u_mode", GetFftDisplayMode());
        shader->SetVec3("u_viewPos", viewPos);
        shader->SetVec2("u_fftGridSize", Ocean::GetBaseFFTResolution());
        shader->SetBool("u_wireframe", wireframe);
        shader->SetFloat("u_meshSubdivisionFactor", Ocean::GetMeshSubdivisionFactor());

        glGenerateTextureMipmap(fftFrameBuffer_band0->GetColorAttachmentHandleByName("Normals"));
        glGenerateTextureMipmap(fftFrameBuffer_band1->GetColorAttachmentHandleByName("Normals"));

        glBindVertexArray(oceanMeshPatch->GetVAO());
        glPatchParameteri(GL_PATCH_VERTICES, 4);

        // Surface
        glCullFace(GL_BACK);
        shader->SetInt("u_normalMultipler", 1);
        for (int x = min; x < max; x++) {
            for (int z = min; z < max; z++) {
                tesseleationTransform.position = glm::vec3(patchOffset * x, Ocean::GetOceanOriginY(), patchOffset * z);
                if (swap) {
                    tesseleationTransform.position += glm::vec3(offset, 0.0f, 0.0f);
                }
                shader->SetMat4("u_model", tesseleationTransform.to_mat4());
                glDrawElements(GL_PATCHES, oceanMeshPatch->GetIndexCount(), GL_UNSIGNED_INT, nullptr);
            }
        }

        // Inverted surface
        glCullFace(GL_FRONT);
        shader->SetInt("u_normalMultipler", -1);
        for (int x = min; x < max; x++) {
            for (int z = min; z < max; z++) {
                tesseleationTransform.position = glm::vec3(patchOffset * x, Ocean::GetOceanOriginY(), patchOffset * z);
                if (swap) {
                    tesseleationTransform.position += glm::vec3(offset, 0.0f, 0.0f);
                }
                shader->SetMat4("u_model", tesseleationTransform.to_mat4());
                glDrawElements(GL_PATCHES, oceanMeshPatch->GetIndexCount(), GL_UNSIGNED_INT, nullptr);
            }
        }

        // Cleanup
        shader->SetBool("u_wireframe", false);
        glEnable(GL_DEPTH_TEST);
        glCullFace(GL_BACK);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    }

    void OceanSurfaceCompositePass() {
        if (!World::HasOcean()) {
            return;
        }
      
        OpenGLFrameBuffer* gBuffer = GetFrameBuffer("GBuffer");
        OpenGLFrameBuffer* waterFrameBuffer = GetFrameBuffer("Water");
        OpenGLFrameBuffer* quaterSizeFrameBuffer = GetFrameBuffer("QuarterSize");
        OpenGLShader* shader = GetShader("OceanSurfaceComposite");

        if (!gBuffer) return;
        if (!shader) return;
        if (!waterFrameBuffer) return;
        if (!quaterSizeFrameBuffer) return;

        // Down sample the final lighting to 25%
        BlitFrameBuffer(gBuffer, quaterSizeFrameBuffer, "FinalLighting", "DownsampledFinalLighting", GL_COLOR_BUFFER_BIT, GL_LINEAR);

        const ViewportData& viewportData = RenderDataManager::GetViewportData()[0];
        glm::mat4 projectionMatrix = viewportData.projection;
        glm::mat4 viewMatrix = viewportData.view;
        glm::vec3 viewPos = viewportData.viewPos;
        glm::mat4 projectionView = viewportData.projectionView;

        // Water surface composite
        glm::mat4 inverseProjectionView = glm::inverse(projectionView);
        glm::vec2 resolution = glm::vec2(gBuffer->GetWidth(), gBuffer->GetHeight());
        shader->Bind();
        shader->SetFloat("u_time", Game::GetTotalTime());
        shader->SetVec3("u_viewPos", viewPos);
        shader->SetVec2("u_resolution", resolution);
        shader->SetMat4("u_inverseProjectionView", inverseProjectionView);
        shader->SetFloat("u_oceanYOrigin", Ocean::GetOceanOriginY());

        glBindImageTexture(0, gBuffer->GetColorAttachmentHandleByName("FinalLighting"), 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA16F);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, waterFrameBuffer->GetColorAttachmentHandleByName("Color"));
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, AssetManager::GetTextureByName("WaterNormals")->GetGLTexture().GetHandle());
        glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_2D, AssetManager::GetTextureByName("WaterDUDV")->GetGLTexture().GetHandle());
        glActiveTexture(GL_TEXTURE4);
        glBindTexture(GL_TEXTURE_2D, waterFrameBuffer->GetColorAttachmentHandleByName("UnderwaterMask"));
        glActiveTexture(GL_TEXTURE5);
        glBindTexture(GL_TEXTURE_2D, quaterSizeFrameBuffer->GetColorAttachmentHandleByName("DownsampledFinalLighting"));

        glDispatchCompute((gBuffer->GetWidth() + 7) / 8, (gBuffer->GetHeight() + 7) / 8, 1);

    }


    void OceanUnderwaterCompositePass() {
        if (!World::HasOcean()) {
            return;
        }

    }
     

}