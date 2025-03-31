#include "API/OpenGL/GL_backend.h"
#include "API/OpenGL/Renderer/GL_renderer.h"
#include "AssetManagement/AssetManager.h"
#include "Core/Game.h"
#include "Renderer/RenderDataManager.h"
#include "Viewport/ViewportManager.h"

// Get me out of here
#include "World/World.h"
// Get me out of here

namespace OpenGLRenderer {

    void RenderFlashLightShadowMaps();

    void RenderShadowMaps() {
        RenderFlashLightShadowMaps();
    }

    void RenderFlashLightShadowMaps() {
        OpenGLShader* shader = GetShader("ShadowMap");
        OpenGLShadowMap* shadowMapsFBO = GetShadowMap("FlashlightShadowMaps");
        OpenGLHeightMapMesh& heightMapMesh = OpenGLBackEnd::GetHeightMapMesh();

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
            glCullFace(GL_FRONT);
            glBindVertexArray(OpenGLBackEnd::GetVertexDataVAO());
            for (RenderItem& renderItem : World::GetRenderItems()) {
                shader->SetMat4("u_modelMatrix", renderItem.modelMatrix);
                Mesh* mesh = AssetManager::GetMeshByIndex(renderItem.meshIndex);
                glDrawElementsBaseVertex(GL_TRIANGLES, mesh->indexCount, GL_UNSIGNED_INT, (void*)(sizeof(unsigned int) * mesh->baseIndex), mesh->baseVertex);
            }

            // Heightfield chunks
            Transform transform;
            transform.scale = glm::vec3(HEIGHTMAP_SCALE_XZ, HEIGHTMAP_SCALE_Y, HEIGHTMAP_SCALE_XZ);
            glm::mat4 modelMatrix = transform.to_mat4();
            glm::mat4 inverseModelMatrix = glm::inverse(modelMatrix);
            shader->SetMat4("u_modelMatrix", modelMatrix);

            std::vector<HeightMapChunk>& chunks = World::GetHeightMapChunks();
            for (HeightMapChunk& chunk : chunks) {
                if (!frustum.IntersectsAABB(AABB(chunk.aabbMin, chunk.aabbMax))) continue;
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
            OpenGLDetachedMesh& houseMesh = World::GetHouseMesh();
            glBindVertexArray(houseMesh.GetVAO());
            shader->SetMat4("u_modelMatrix", glm::mat4(1.0f));
            const std::vector<HouseRenderItem>& renderItems = RenderDataManager::GetHouseRenderItems();
            for (const HouseRenderItem& renderItem : renderItems) {
                int indexCount = renderItem.indexCount;
                int baseVertex = renderItem.baseVertex;
                int baseIndex = renderItem.baseIndex;glDrawElementsBaseVertex(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, (void*)(sizeof(unsigned int) * baseIndex), baseVertex);
            }

            //break;
        }

        glBindVertexArray(0);
        glCullFace(GL_BACK);
    }
}