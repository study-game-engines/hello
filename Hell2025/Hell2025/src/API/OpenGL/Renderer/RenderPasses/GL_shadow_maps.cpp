#include "API/OpenGL/GL_backend.h"
#include "API/OpenGL/Renderer/GL_renderer.h"
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
        OpenGLShader* shader = GetShader("ShadowMapGeometry");
        OpenGLFrameBuffer* frameBuffer = GetFrameBuffer("FlashlightShadowMap");
        OpenGLHeightMapMesh& heightMapMesh = OpenGLBackEnd::GetHeightMapMesh();

        glEnable(GL_DEPTH_TEST);
        glDrawBuffer(GL_NONE);
        glReadBuffer(GL_NONE);
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);

        frameBuffer->Bind();
        frameBuffer->SetViewport();
        frameBuffer->ClearDepthAttachment();

        glm::mat4 lightProjectionView = Game::GetLocalPlayerByIndex(0)->GetFlashlightProjectionView();

        shader->Use();
        shader->SetMat4("u_projectionView", lightProjectionView);

        // Heightmap
        // Scene geometry
        glCullFace(GL_FRONT);
        glBindVertexArray(OpenGLBackEnd::GetVertexDataVAO());
        for (RenderItem& renderItem : World::GetRenderItems()) {
            shader->SetMat4("u_modelMatrix", renderItem.modelMatrix);
            Mesh* mesh = AssetManager::GetMeshByIndex(renderItem.meshIndex);
            glDrawElementsBaseVertex(GL_TRIANGLES, mesh->indexCount, GL_UNSIGNED_INT, (void*)(sizeof(unsigned int) * mesh->baseIndex), mesh->baseVertex);
        }



        for (int i = 0; i < 4; i++) {
            Viewport* viewport = ViewportManager::GetViewportByIndex(i);

            Frustum frustum;
            frustum.Update(lightProjectionView);

            Transform transform;
            transform.scale = glm::vec3(HEIGHTMAP_SCALE_XZ, HEIGHTMAP_SCALE_Y, HEIGHTMAP_SCALE_XZ);
            glm::mat4 modelMatrix = transform.to_mat4();
            glm::mat4 inverseModelMatrix = glm::inverse(modelMatrix);
            shader->SetMat4("u_modelMatrix", modelMatrix);
            int indexCount = (HEIGHT_MAP_SIZE - 1) * (HEIGHT_MAP_SIZE - 1) * 6;
            int vertexCount = HEIGHT_MAP_SIZE * HEIGHT_MAP_SIZE;

            // Heightfield chunks
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
            break;
        }
        glBindVertexArray(0);

        //// TODO: find out what the fuck is going on here!
        //
        //int i = 0; // viewport index !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
        //glBindVertexArray(heightMapMesh.GetVAO());
        //glDrawElementsInstancedBaseVertexBaseInstance(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, (void*)(sizeof(unsigned int) * 0), 1, 0, i);
        
        glCullFace(GL_BACK);
    }
}