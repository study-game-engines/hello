#include "API/OpenGL/GL_backend.h"
#include "API/OpenGL/Renderer/GL_renderer.h"
#include "AssetManagement/AssetManager.h"
#include "Core/Game.h"
#include "Renderer/RenderDataManager.h"
#include "Viewport/ViewportManager.h"
#include "World/World.h"

namespace OpenGLRenderer {

    void RenderFlashLightShadowMaps();
    void RenderPointLightShadowMaps();

    void RenderShadowMaps() {
        RenderFlashLightShadowMaps();
        RenderPointLightShadowMaps();
    }

    void RenderFlashLightShadowMaps() {
        OpenGLShader* shader = GetShader("ShadowMap");
        OpenGLShadowMap* shadowMapsFBO = GetShadowMap("FlashlightShadowMaps");
        OpenGLHeightMapMesh& heightMapMesh = OpenGLBackEnd::GetHeightMapMesh();
        const DrawCommandsSet& drawInfoSet = RenderDataManager::GetDrawInfoSet();
        const FlashLightShadowMapDrawInfo& flashLightShadowMapDrawInfo = RenderDataManager::GetFlashLightShadowMapDrawInfo();
        
        glm::mat4 heightMapModelMatrix = glm::scale(glm::mat4(1.0f), glm::vec3(HEIGHTMAP_SCALE_XZ, HEIGHTMAP_SCALE_Y, HEIGHTMAP_SCALE_XZ)); // move to heightmap manager

        glEnable(GL_DEPTH_TEST);
        glDrawBuffer(GL_NONE);
        glReadBuffer(GL_NONE);
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);

        shadowMapsFBO->Bind();
        shadowMapsFBO->SetViewport();

        shader->Use();

        for (int i = 0; i < Game::GetLocalPlayerCount(); i++) {
            shadowMapsFBO->BindLayer(i);
            shadowMapsFBO->ClearLayer(i);

            glm::mat4 lightProjectionView = Game::GetLocalPlayerByIndex(i)->GetFlashlightProjectionView();
            shader->SetMat4("u_projectionView", lightProjectionView);

            Frustum frustum;
            frustum.Update(lightProjectionView);

            // Scene geometry
            shader->SetBool("u_useInstanceData", true);
            glCullFace(GL_FRONT);
            glBindVertexArray(OpenGLBackEnd::GetVertexDataVAO());

            MultiDrawIndirect(flashLightShadowMapDrawInfo.flashlightShadowMapGeometry[i]);

            // Heightfield chunks
            std::vector<HeightMapChunk>& chunks = World::GetHeightMapChunks();
            OpenGLHeightMapMesh& heightMapMesh = OpenGLBackEnd::GetHeightMapMesh();
            glBindVertexArray(heightMapMesh.GetVAO());
            shader->SetMat4("u_modelMatrix", heightMapModelMatrix);
            shader->SetBool("u_useInstanceData", false);

            for (uint32_t chunkIndex : flashLightShadowMapDrawInfo.heightMapChunkIndices[i]) {
                HeightMapChunk& chunk = chunks[chunkIndex];
                int indexCount = INDICES_PER_CHUNK;
                int baseVertex = 0;
                int baseIndex = chunk.baseIndex;
                void* indexOffset = (GLvoid*)(baseIndex * sizeof(GLuint));
                int instanceCount = 1;
                int viewportIndex = i;
                if (indexCount > 0) {
                    glDrawElementsInstancedBaseVertexBaseInstance(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, indexOffset, instanceCount, baseVertex, viewportIndex);
                }
            }

            // House render items
            OpenGLMeshBuffer& houseMeshBuffer = World::GetHouseMeshBuffer().GetGLMeshBuffer();
            glBindVertexArray(houseMeshBuffer.GetVAO());
            shader->SetMat4("u_modelMatrix", glm::mat4(1.0f));

            const std::vector<HouseRenderItem>& renderItems = flashLightShadowMapDrawInfo.houseMeshRenderItems[i];
            for (const HouseRenderItem& renderItem : renderItems) {
                int indexCount = renderItem.indexCount;
                int baseVertex = renderItem.baseVertex;
                int baseIndex = renderItem.baseIndex;
                glDrawElementsBaseVertex(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, (void*)(sizeof(unsigned int) * baseIndex), baseVertex);
            }
        }

        glBindVertexArray(0);
        glCullFace(GL_BACK);
    }

    void RenderPointLightShadowMaps() {

        OpenGLShader* shader = GetShader("ShadowCubeMap");
        OpenGLShadowCubeMapArray* hiResShadowMaps = GetShadowMapArray("HiRes");
        const DrawCommandsSet& drawInfoSet = RenderDataManager::GetDrawInfoSet();

        if (!shader) return;
        if (!hiResShadowMaps) return;

        shader->Use();
        shader->SetBool("u_useInstanceData", true);

        GLuint shadowMapTextureID = hiResShadowMaps->GetDepthTexture();
        if (shadowMapTextureID != 0) {
            float clearDepthValue = 1.0f;
            glClearTexImage(shadowMapTextureID, 0, GL_DEPTH_COMPONENT, GL_FLOAT, &clearDepthValue);
        }
        else {
            std::cout << "Error: Invalid shadow map texture handle for clearing.\n";
        }

        glDepthMask(true);
        glDisable(GL_BLEND);
        glDisable(GL_CULL_FACE);
        glEnable(GL_DEPTH_TEST);
        glViewport(0, 0, hiResShadowMaps->GetSize(), hiResShadowMaps->GetSize());
        glBindFramebuffer(GL_FRAMEBUFFER, hiResShadowMaps->GetHandle());

        glCullFace(GL_FRONT);
        glBindVertexArray(OpenGLBackEnd::GetVertexDataVAO());

        const std::vector<GPULight>& gpuLightsHighRes = RenderDataManager::GetGPULightsHighRes();

        for (int i = 0; i < gpuLightsHighRes.size(); i++) {
            const GPULight& gpuLight = gpuLightsHighRes[i];

            Light* light = World::GetLightByIndex(gpuLight.lightIndex);
            if (!light) continue;

            shader->SetFloat("farPlane", light->GetRadius());
            shader->SetVec3("lightPosition", light->GetPosition());
            shader->SetMat4("shadowMatrices[0]", light->GetProjectionView(0));
            shader->SetMat4("shadowMatrices[1]", light->GetProjectionView(1));
            shader->SetMat4("shadowMatrices[2]", light->GetProjectionView(2));
            shader->SetMat4("shadowMatrices[3]", light->GetProjectionView(3));
            shader->SetMat4("shadowMatrices[4]", light->GetProjectionView(4));
            shader->SetMat4("shadowMatrices[5]", light->GetProjectionView(5));

            for (int face = 0; face < 6; ++face) {
                GLuint layer = i * 6 + face;
                shader->SetInt("faceIndex", face);
                glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, hiResShadowMaps->GetDepthTexture(), 0, layer);
                MultiDrawIndirect(drawInfoSet.shadowMapHiRes[i][face]);
            }
        }

        size_t maxLights = 4;
        size_t lightCount = std::min(maxLights, World::GetLights().size());

        OpenGLMeshBuffer& houseMeshBuffer = World::GetHouseMeshBuffer().GetGLMeshBuffer();
        glBindVertexArray(houseMeshBuffer.GetVAO());
        shader->SetBool("u_useInstanceData", false);

        for (int i = 0; i < gpuLightsHighRes.size(); i++) {
            const GPULight& gpuLight = gpuLightsHighRes[i];

            Light* light = World::GetLightByIndex(gpuLight.lightIndex);
            if (!light) continue;

            shader->SetFloat("farPlane", light->GetRadius());
            shader->SetVec3("lightPosition", light->GetPosition());
            shader->SetMat4("shadowMatrices[0]", light->GetProjectionView(0));
            shader->SetMat4("shadowMatrices[1]", light->GetProjectionView(1));
            shader->SetMat4("shadowMatrices[2]", light->GetProjectionView(2));
            shader->SetMat4("shadowMatrices[3]", light->GetProjectionView(3));
            shader->SetMat4("shadowMatrices[4]", light->GetProjectionView(4));
            shader->SetMat4("shadowMatrices[5]", light->GetProjectionView(5));

            for (int face = 0; face < 6; ++face) {
                shader->SetInt("faceIndex", face);
                int shadowMapIndex = i;
                GLuint layer = shadowMapIndex * 6 + face;

                glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, hiResShadowMaps->GetDepthTexture(), 0, layer);
                
                Frustum* frustum = light->GetFrustumByFaceIndex(face);
                if (!frustum) return;

                const std::vector<HouseRenderItem>& renderItems = RenderDataManager::GetHouseRenderItems();
                for (const HouseRenderItem& renderItem : renderItems) {

                    if (!frustum->IntersectsAABBFast(renderItem)) continue;

                    int indexCount = renderItem.indexCount;
                    int baseVertex = renderItem.baseVertex;
                    int baseIndex = renderItem.baseIndex; 
                    glDrawElementsBaseVertex(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, (void*)(sizeof(unsigned int) * baseIndex), baseVertex);
                }
            }
        }

        glCullFace(GL_BACK);
    }
}