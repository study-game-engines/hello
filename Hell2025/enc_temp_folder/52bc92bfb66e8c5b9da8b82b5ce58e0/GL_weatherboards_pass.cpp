#include "API/OpenGL/Renderer/GL_renderer.h"
#include "Types/House/WeatherBoards.h"
#include "AssetManagement/AssetManager.h"
#include "Input/Input.h"
#include "Viewport/ViewportManager.h"

#include "World/World.h"
#include "Util/Util.h"

namespace OpenGLRenderer {


    OpenGLMeshBuffer g_weathboardMeshBuffer;





    struct BoardVertexData {
        std::vector<Vertex> vertices;
        std::vector<uint32_t> indices;
    };

    BoardVertexData CreateBoardVertexData(glm::vec3 begin, glm::vec3 end, int yUVOffsetIndex, float xUVOffset) {

        BoardVertexData weatherBoardVertexData;

        Model* model = AssetManager::GetModelByName("WeatherBoard");
        uint32_t meshIndex = model->GetMeshIndices()[0];
        Mesh* mesh = AssetManager::GetMeshByIndex(meshIndex);

        std::span<Vertex> verticesSpan = AssetManager::GetMeshVerticesSpan(mesh);
        std::span<uint32_t> indicesSpan = AssetManager::GetMeshIndicesSpan(mesh);

        float boardWidth = glm::distance(begin, end);
        const float meshBoardWidth = 4.0f; // FULL WIDTH (MATHCES TEXUTRE STUFF)

        for (Vertex vertex : verticesSpan) {

            // Shift right most vertices
            if (vertex.position.z > 0.5f) {
                vertex.position.z = boardWidth;
                vertex.uv.x = boardWidth / meshBoardWidth;
            }

            // Finally shift to the origin
            vertex.position += begin;

            // Test next board up 
            float uvVerticalOffset = 1.0f / 16.0f;;
            vertex.uv.y -= uvVerticalOffset * yUVOffsetIndex;

            vertex.uv.x += xUVOffset;
            weatherBoardVertexData.vertices.push_back(vertex);
        }

        // Indices
        for (uint32_t& index : indicesSpan) {
            weatherBoardVertexData.indices.push_back(index);
        }

        return weatherBoardVertexData;
    }


    void InitTestBoardMesh2() {
      
        glm::vec3 origin = glm::vec3(-0.025f, 0.0f, -3.0f);

        int horizontalBoardCount = 20;

        float individialBoardHeight = 0.13f;
        float boardWidth = 9.1f;
        float meshBoardWidth = 4.0f;

        Model* model = AssetManager::GetModelByName("WeatherBoard");
        for (uint32_t meshIndex : model->GetMeshIndices()) {

            Mesh* mesh = AssetManager::GetMeshByIndex(meshIndex);
            std::span<Vertex> verticesSpan = AssetManager::GetMeshVerticesSpan(mesh);
            std::span<uint32_t> indicesSpan = AssetManager::GetMeshIndicesSpan(mesh);

            std::vector<Vertex> vertices;
            std::vector<uint32_t> indices;

            for (int i = 0; i < horizontalBoardCount; i++) {

                uint32_t baseVertex = vertices.size();


                for (Vertex vertex : verticesSpan) {

                    // Shift right most vertices
                    if (vertex.position.z > 0.5f) {
                        vertex.position.z = boardWidth;
                        vertex.uv.x = boardWidth / meshBoardWidth;
                    }

                    // Finally shift to the origin
                    vertex.position += origin;

                    // Extra finally add board vertical offset
                    vertex.position.y += i * individialBoardHeight;

                    // Test next board up 
                    float uvVerticalOffset = 1.0f / 16.0f;;
                    vertex.uv.y -= uvVerticalOffset * i;

                    if (i > 15) {
                        vertex.uv.y -= uvVerticalOffset * 12;
                        vertex.uv.x += 0.5f;
                    }
                    vertices.push_back(vertex);
                }

                // Indices
                for (uint32_t& index : indicesSpan) {
                    indices.push_back(index + baseVertex);
                }
            }


            g_weathboardMeshBuffer.ReleaseBuffers();
            g_weathboardMeshBuffer.UpdateBuffers(vertices, indices);
        }    
    }




    void InitTestBoardMesh() {

        glm::vec3 origin = glm::vec3(-0.025f, 0.0f, -3.0f);


        std::vector<BoardVertexData> boardVertexDataSet;
        float individialBoardHeight = 0.13f;

        for (int i = 0; i < 20; i++) {

            int yUVOffsetIndex = i;
            float xUVOffset = 0.0f;

            if (i > 15) {
                yUVOffsetIndex += 12;
            }

            glm::vec3 start = origin;
            glm::vec3 end = origin + glm::vec3(0, 0, 9.1);

            start.y += individialBoardHeight * i;
            end.y += individialBoardHeight * i;


            DrawPoint(start, RED);
            DrawPoint(end, GREEN);

            glm::vec3 rayOrigin = start;
            glm::vec3 rayDir = glm::normalize(end - start);
            float desiredBoardLength = glm::distance(start, end);

            CubeRayResult rayResult;
            do {
                rayResult = Util::CastCubeRay(rayOrigin, rayDir, World::GetDoorAndWindowCubeTransforms(), desiredBoardLength);
                if (rayResult.hitFound) {

                    glm::vec3 hitPos = rayOrigin + (rayDir * rayResult.distanceToHit);
                    DrawPoint(hitPos, YELLOW);

                    Transform transform;
                    transform.position = rayOrigin;
                    transform.rotation.y = Util::EulerYRotationBetweenTwoPoints(start, end);
                    transform.scale.x = rayResult.distanceToHit;

                    float dot = glm::dot(rayResult.hitNormal, rayDir);
                    if (dot <= 0.99f) {




                        glm::vec3 localStart = rayOrigin;
                        glm::vec3 localEnd = rayOrigin + (rayDir * rayResult.distanceToHit);

                        BoardVertexData boardVertexData = CreateBoardVertexData(localStart, localEnd, yUVOffsetIndex, xUVOffset);
                        boardVertexDataSet.emplace_back(boardVertexData);

                        //
                        //
                        //Trim& trim = m_trims.emplace_back();
                        //trim.Init(transform, "TrimFloor", "Trims");


                    }

                    // Start next ray just past the opposite face of the hit cube
                    rayOrigin = rayResult.hitPosition + (rayDir * 0.01f);
                }

            } while (rayResult.hitFound);

            BoardVertexData boardVertexData = CreateBoardVertexData(rayOrigin, end, yUVOffsetIndex, xUVOffset);
            boardVertexDataSet.emplace_back(boardVertexData);









        }

        


        // Build the mesh
        std::vector<Vertex> vertices;
        std::vector<uint32_t> indices;

        for (BoardVertexData& boardVertexData : boardVertexDataSet) {

            uint32_t baseVertex = vertices.size();

            vertices.insert(vertices.end(), boardVertexData.vertices.begin(), boardVertexData.vertices.end());

            for (uint32_t& index : boardVertexData.indices) {
                indices.push_back(index + baseVertex);
            }
        }

        g_weathboardMeshBuffer.ReleaseBuffers();
        g_weathboardMeshBuffer.UpdateBuffers(vertices, indices);
        
    }

    void WeatherBoardsPass() {

        static bool initilized = false;
        if (!initilized) {
            InitTestBoardMesh();

            initilized = true;
        }

        if (Input::KeyPressed(HELL_KEY_J)) {
            InitTestBoardMesh();
        }

        InitTestBoardMesh();

      // static WeatherBoards weatherboards;
      //
      // weatherboards.begin = glm::vec3(0.0f, 0.0f, -3.0f);
      // weatherboards.end = glm::vec3(0.0f, 0.0f, 6.1f);
      //
      // DrawPoint(weatherboards.begin, YELLOW);
      // DrawPoint(weatherboards.end, YELLOW);

        std::vector<Vertex> vertices;
        std::vector<uint32_t> indices;

        Model* model = AssetManager::GetModelByName("WeatherBoard");
        for (uint32_t meshIndex : model->GetMeshIndices()) {

            Mesh* mesh = AssetManager::GetMeshByIndex(meshIndex);
            std::span<Vertex> verticesSpan = AssetManager::GetMeshVerticesSpan(mesh);
            std::span<uint32_t> indicesSpan = AssetManager::GetMeshIndicesSpan(mesh);

            for (Vertex& vertex : verticesSpan) {
                vertices.push_back(vertex);
                //DrawPoint(vertex.position, RED);
            }

            for (uint32_t& index : indicesSpan) {
                indices.push_back(index);
            }
        }


        OpenGLFrameBuffer* gBuffer = GetFrameBuffer("GBuffer");
        //OpenGLShader* shader = GetShader("GBuffer");
        OpenGLShader* editorMeshShader = GetShader("EditorMesh");
        OpenGLShader* debugShader = GetShader("DebugTextured");

        if (!gBuffer) return;
        //if (!shader) return;
        if (!editorMeshShader) return;
        if (!debugShader) return;

        gBuffer->Bind();
        gBuffer->DrawBuffers({ "BaseColor", "Normal", "RMA", "WorldSpacePosition" });



        Transform transform;
        transform.position = glm::vec3(0.0, 0.0, 0.0f);
        glm::mat4 modelMatrix = transform.to_mat4();

        // Render house
        debugShader->Bind();
        debugShader->SetMat4("u_model", modelMatrix);

        glBindVertexArray(g_weathboardMeshBuffer.GetVAO());


        for (Vertex& vertex : vertices) {

            if (vertex.position.z > 0.5f) {
                //DrawPoint(vertex.position, RED);
            }
            else {
                //DrawPoint(vertex.position, YELLOW);
            }

        }


        Material* material = AssetManager::GetMaterialByName("WeatherBoards1");
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, AssetManager::GetTextureByIndex(material->m_basecolor)->GetGLTexture().GetHandle());
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, AssetManager::GetTextureByIndex(material->m_normal)->GetGLTexture().GetHandle());
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, AssetManager::GetTextureByIndex(material->m_rma)->GetGLTexture().GetHandle());


        for (int i = 0; i < 4; i++) {

            Viewport* viewport = ViewportManager::GetViewportByIndex(i);
            if (!viewport->IsVisible()) continue;
            if (g_weathboardMeshBuffer.GetIndexCount() <= 0) continue;

            OpenGLRenderer::SetViewport(gBuffer, viewport);
            debugShader->SetInt("u_viewportIndex", i);

            int indexCount = indices.size();
            int baseIndex = 0;
            int baseVertex = 0;
            glDrawElementsBaseVertex(GL_TRIANGLES, g_weathboardMeshBuffer.GetIndexCount(), GL_UNSIGNED_INT, (void*)(sizeof(unsigned int) * baseIndex), baseVertex);

        }


        glBindVertexArray(0);

        
    }
}
