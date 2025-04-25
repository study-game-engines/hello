#include "API/OpenGL/Renderer/GL_renderer.h"
#include "Types/House/WeatherBoards.h"
#include "AssetManagement/AssetManager.h"
#include "Input/Input.h"
#include "Viewport/ViewportManager.h"

namespace OpenGLRenderer {


    OpenGLMeshBuffer g_weathboardMeshBuffer;

    void InitTestBoardMesh() {
      
       //Vertex vertex0;
       //vertex0.position = glm::vec3((0.0197187, -6.32836e-17, 0.0196091));
       //vertex0.normal = glm::vec3((-0.359333, -0.93321, -1.53042e-07));
       //vertex0.tangent = glm::vec3((-5.99525e-08, -1.40911e-07, 1));
       //vertex0.uv = glm::vec2(0.00390625, 0.996094);
       //
       //Vertex vertex1;
       //vertex1.position = glm::vec3((0.0104523, 0.00356786, 1.02453));
       //vertex1.normal = glm::vec3((-0.721132, -0.692798, -1.40856e-07));
       //vertex1.tangent = glm::vec3((-5.99525e-08, -1.40911e-07, 1));
       //vertex1.uv = glm::vec2(0.998047, 0.992138);
       //
       //Vertex vertex2;
       //vertex2.position = glm::vec3((0.0104523, 0.003568, 0.0196091));
       //vertex2.normal = glm::vec3((-0.721132, -0.692797, -1.40856e-07));
       //vertex2.tangent = glm::vec3((-5.99525e-08, -1.40911e-07, 1));
       //vertex2.uv = glm::vec2(0.00390625, 0.992138);
       //
       //Vertex vertex3;
       //vertex3.position = glm::vec3((0.00849484, 0.00932995, 1.02453));
       //vertex3.normal = glm::vec3((-0.993389, -0.114797, -7.57323e-08));
       //vertex3.tangent = glm::vec3((-5.99525e-08, -1.40911e-07, 1));
       //vertex3.uv = glm::vec2(0.998047, 0.989713);
       //
       //Vertex vertex4;
       //vertex4.position = glm::vec3((0.0084949, 0.00933009, 0.0196091));
       //vertex4.normal = glm::vec3((-0.993389, -0.114797, -7.57323e-08));
       //vertex4.tangent = glm::vec3((-5.99525e-08, -1.40911e-07, 1));
       //vertex4.uv = glm::vec2(0.00390625, 0.989713);
       //
       //Vertex vertex5;
       //vertex5.position = glm::vec3((0.0202826, 0.13, 1.02454));
       //vertex5.normal = glm::vec3((-0.995263, 0.0972228, -4.59688e-08));
       //vertex5.tangent = glm::vec3((-5.99525e-08, -1.40911e-07, 1));
       //vertex5.uv = glm::vec2(0.998047, 0.941406);
       //
       //Vertex vertex6;
       //vertex6.position = glm::vec3((0.0202826, 0.13, 0.0196092));
       //vertex6.normal = glm::vec3((-0.995263, 0.0972229, -4.59688e-08));
       //vertex6.tangent = glm::vec3((-5.99525e-08, -1.40911e-07, 1));
       //vertex6.uv = glm::vec2(0.00390625, 0.941406);
       //
       //Vertex vertex7;
       //vertex7.position = glm::vec3((0.0197186, -1.41605e-07, 1.02453));
       //vertex7.normal = glm::vec3((-0.359333, -0.93321, -1.53042e-07));
       //vertex7.tangent = glm::vec3((-5.99525e-08, -1.40911e-07, 1));
       //vertex7.uv = glm::vec2(0.998047, 0.996094);
      // std::vector<Vertex> vertices = { vertex0, vertex1, vertex2, vertex3, vertex4, vertex5, vertex6, vertex7
      // std::vector<uint32_t> indices = { 0, 1, 2, 3, 2, 1, 4, 5, 6, 0, 7, 1, 3, 4, 2, 4, 3, 5 };

        float weatherboardWidth = 2.0f;

        Model* model = AssetManager::GetModelByName("WeatherBoard");
        for (uint32_t meshIndex : model->GetMeshIndices()) {

            Mesh* mesh = AssetManager::GetMeshByIndex(meshIndex);
            std::span<Vertex> verticesSpan = AssetManager::GetMeshVerticesSpan(mesh);
            std::span<uint32_t> indicesSpan = AssetManager::GetMeshIndicesSpan(mesh);

            std::vector<Vertex> vertices;
            for (Vertex vertex : verticesSpan) {

                // Shift right most vertices
                if (vertex.position.z > 0.5f) {
                    vertex.position.z = weatherboardWidth;
                }
                else {
                    // No change needed
                }
                vertices.push_back(vertex);
            }

            std::vector<uint32_t> indices;
            for (uint32_t& index : indicesSpan) {
                indices.push_back(index);
            }

            g_weathboardMeshBuffer.ReleaseBuffers();
            g_weathboardMeshBuffer.UpdateBuffers(vertices, indices);
        }    
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
                DrawPoint(vertex.position, RED);
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
        transform.position = glm::vec3(-1.0, 0.0, 0.0f);
        glm::mat4 modelMatrix = transform.to_mat4();

        // Render house
        debugShader->Bind();
        debugShader->SetMat4("u_model", modelMatrix);

        glBindVertexArray(g_weathboardMeshBuffer.GetVAO());


        for (Vertex& vertex : vertices) {

            if (vertex.position.z > 0.5f) {
                DrawPoint(vertex.position, RED);
            }
            else {
                DrawPoint(vertex.position, YELLOW);
            }

        }


        Material* material = AssetManager::GetMaterialByName("WeatherBoards0");
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
            glDrawElementsBaseVertex(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, (void*)(sizeof(unsigned int) * baseIndex), baseVertex);

        }


        glBindVertexArray(0);

        
    }
}
