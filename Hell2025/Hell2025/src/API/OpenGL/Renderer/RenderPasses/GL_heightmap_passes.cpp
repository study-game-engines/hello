 #include "../GL_renderer.h" 
#include "../../GL_backend.h"
#include "API/OpenGL/Types/GL_heightmap_mesh.h"
#include "Config/Config.h"
#include "AssetManagement/AssetManager.h"
#include "Viewport/ViewportManager.h"
#include "Renderer/RenderDataManager.h"

#include "API/OpenGL/Types/GL_texture_readback.h"
#include "BackEnd/BackEnd.h"
#include "Core/Game.h"
#include "Editor/Editor.h"
#include "Editor/Gizmo.h"
#include "Imgui/ImguiBackEnd.h"
#include "Input/Input.h"
#include "World/HeightMapManager.h"
#include "Tools/ImageTools.h"
#include "Util/Util.h"
#include "World/MapManager.h"
#include "World/SectorManager.h"
#include "World/World.h"


#include "Game/AStarMap.h"
#include "lodepng/lodepng.h"

#include "Audio/Audio.h"

#include "Physics/Physics.h"

namespace OpenGLRenderer {

    void BlitHeightMapWorld();
    void GenerateHeightMapImageData();
    void GenerateHeightMapVertexData();
    void GeneratePhysXTextures();
    void DrawHeightMap();
    void SaveHeightMap();


    void RecalculateAllHeightMapData() {
        BlitHeightMapWorld();
        //GenerateHeightMapImageData();
        GenerateHeightMapVertexData();
        GeneratePhysXTextures();
        AStarMap::UpdateDebugMeshesFromHeightField();
    }

    void HeightMapPass() {

        DrawHeightMap();

        if (Editor::IsOpen() && Editor::GetEditorMode() == EditorMode::HEIGHTMAP_EDITOR) {

            //if (Input::KeyPressed(HELL_KEY_L)) {                
            //    HeightMapData heightMapData = File::LoadHeightMap("TEST.heightmap");
            //    OpenGLFrameBuffer* heightmapFBO = GetFrameBuffer("HeightMap");
            //    GLuint textureHandle = heightmapFBO->GetColorAttachmentHandleByName("Color");
            //}
            //if (Input::KeyPressed(HELL_KEY_S)) {
            //    SaveHeightMap();
            //}
        }

        if (Input::KeyPressed(HELL_KEY_U)) {
            if (Util::RenameFile("res/shit.txt", "res/fuck.txt")) {
                std::cout << "rename successful\n";
            }
        }
    }

    void BlitHeightMapWorld() {

        OpenGLFrameBuffer* worldFramebuffer = GetFrameBuffer("World");
        OpenGLFrameBuffer* heightMapBlitBuffer = GetFrameBuffer("HeightMapBlitBuffer");
        OpenGLTextureArray& heightmapTextureArray = HeightMapManager::GetGLTextureArray();
        //MapCreateInfo* mapCreateInfo = World::GetCurrentMapCreateInfo();

        int mapWidth = World::GetMapWidth();
        int mapDepth = World::GetMapDepth();
        int textureWidth = mapWidth * 256 + 1;
        int textureHeight = mapDepth * 256 + 1;

        // Hack! write better error checking and handling:
        textureWidth = std::max(textureWidth, 257);
        textureHeight = std::max(textureHeight, 257);

        // If invalid map, clear to black an empty 257*257 heightmap world
        //if (!mapCreateInfo) {
        //    if (worldFramebuffer->GetWidth() != textureWidth || worldFramebuffer->GetHeight() != textureHeight) {
        //        worldFramebuffer->Resize(textureWidth, textureHeight);
        //        worldFramebuffer->ClearAttachment("HeightMap", 0.0);
        //    }
        //    return;
        //}

        if (worldFramebuffer->GetWidth() != textureWidth || worldFramebuffer->GetHeight() != textureHeight) {
            worldFramebuffer->Resize(textureWidth, textureHeight);
        }

        worldFramebuffer->ClearAttachment("HeightMap", 0.0);

        glBindFramebuffer(GL_READ_FRAMEBUFFER, heightMapBlitBuffer->GetHandle());
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, worldFramebuffer->GetHandle());

        for (int x = 0; x < 1; x++) {
            for (int z = 0; z < 1; z++) {

                // Skip if there is no heightmap at this map cell
                const std::string& heightMapName = World::GetHeightMapNameAtLocation(x, z);
                if (heightMapName == "") continue;

                //if (!MapManager::IsHeightMapAtLocation(mapCreateInfo, x, z)) continue;
                
                // Skip if the height map there is is invalid
                int32_t layerIndex = MapManager::GetHeightMapIndexByHeightMapName(heightMapName);
                if (layerIndex == -1) continue;

                // Otherwise blit it into the world heightmap
                int srcX0 = 0;
                int srcY0 = 0;
                int srcX1 = HEIGHT_MAP_SIZE;
                int srcY1 = HEIGHT_MAP_SIZE; 
                int dstX0 = x * HEIGHT_MAP_SIZE;
                int dstY0 = z * HEIGHT_MAP_SIZE;
                int dstX1 = dstX0 + HEIGHT_MAP_SIZE;
                int dstY1 = dstY0 + HEIGHT_MAP_SIZE;
                glFramebufferTextureLayer(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, heightmapTextureArray.GetHandle(), 0, layerIndex);
                glDrawBuffer(worldFramebuffer->GetColorAttachmentSlotByName("HeightMap"));
                glBlitFramebuffer(srcX0, srcY0, srcX1, srcY1, dstX0, dstY0, dstX1, dstY1, GL_COLOR_BUFFER_BIT, GL_LINEAR);
            }
        }
        glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    }


    void GenerateHeightMapImageData() {
        static bool runOnce = true;
        if (!runOnce) return;
        runOnce = false;
        return;

        OpenGLTextureArray& heightmapTextureArray = HeightMapManager::GetGLTextureArray();
        OpenGLShader* shader = GetShader("HeightMapImageGeneration");

        shader->Bind();

        glBindImageTexture(0, heightmapTextureArray.GetHandle(), 0, GL_TRUE, 0, GL_READ_WRITE, GL_R16F);
        glDispatchCompute(HEIGHT_MAP_SIZE / 16, HEIGHT_MAP_SIZE / 16, 1);
        glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
    }

    void PaintHeightMap() {
        if (!IsMouseRayWorldPositionReadBackReady()) return;
        if (!Editor::IsOpen()) return;
        if (Editor::GetEditorMode() != EditorMode::HEIGHTMAP_EDITOR) return;
        if (ImGuiBackEnd::OwnsMouse()) return;

        OpenGLFrameBuffer* worldFramebuffer = GetFrameBuffer("World");
        OpenGLFrameBuffer* gBuffer = GetFrameBuffer("GBuffer");
        OpenGLShader* shader = GetShader("HeightMapPaint");

        if (Input::LeftMouseDown() || Input::RightMouseDown()) {
            shader->Bind();
            shader->SetInt("u_paintX", static_cast<int>(GetMouseRayWorldPostion().x / (float)worldFramebuffer->GetWidth()));
            shader->SetInt("u_paintY", static_cast<int>(GetMouseRayWorldPostion().z / (float)worldFramebuffer->GetHeight()));
            shader->SetFloat("u_brushSize", Editor::GetHeightMapBrushSize());
            shader->SetFloat("u_brushStrength", Editor::GetHeightMapBrushStrength() * (Input::RightMouseDown() ? -1.0f : 1.0f));
            shader->SetFloat("u_noiseStrength", Editor::GetHeightMapNoiseStrength());
            shader->SetFloat("u_noiseScale", Editor::GetHeightMapNoiseScale());
            shader->SetFloat("u_minPaintHeight", Editor::GetHeightMapMinPaintHeight());
            shader->SetFloat("u_maxPaintHeight", Editor::GetHeightMapMaxPaintHeight());

            glMemoryBarrier(GL_ALL_BARRIER_BITS);
            glBindImageTexture(0, worldFramebuffer->GetColorAttachmentHandleByName("HeightMap"), 0, GL_FALSE, 0, GL_READ_WRITE, GL_R16F);
            glBindTextureUnit(1, gBuffer->GetColorAttachmentHandleByName("WorldPosition"));
            glDispatchCompute(worldFramebuffer->GetWidth() / 32, worldFramebuffer->GetHeight() / 32, 1);

            GenerateHeightMapVertexData();
        }
    }

    void GenerateHeightMapVertexData() {
        OpenGLFrameBuffer* worldFramebuffer = GetFrameBuffer("World");
        OpenGLHeightMapMesh& heightMapMesh = OpenGLBackEnd::GetHeightMapMesh();
        OpenGLShader* shader = GetShader("HeightMapVertexGeneration");

        int heightMapCount = World::GetHeightMapCount();
        int heightMapWidth = 256;//worldFramebuffer->GetWidth();
        int heightMapDepth = 256;//worldFramebuffer->GetHeight();

        heightMapMesh.AllocateMemory(heightMapCount);
        
        shader->Bind();
        shader->SetInt("u_heightMapWidth", heightMapWidth);
        shader->SetInt("u_heightMapDepth", heightMapDepth);

        glBindImageTexture(0, worldFramebuffer->GetColorAttachmentHandleByName("HeightMap"), 0, GL_FALSE, 0, GL_READ_ONLY, GL_R16F);
        
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, heightMapMesh.GetVBO());
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 6, heightMapMesh.GetEBO());
        //glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT | GL_VERTEX_ATTRIB_ARRAY_BARRIER_BIT);
       
        glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT | GL_SHADER_STORAGE_BARRIER_BIT | GL_VERTEX_ATTRIB_ARRAY_BARRIER_BIT);


        std::vector<HeightMapChunk>& chunks = World::GetHeightMapChunks();
        for (HeightMapChunk& chunk : chunks) {
            shader->SetInt("u_baseIndex", chunk.baseIndex);
            shader->SetInt("u_baseVertex", chunk.baseVertex);
            shader->SetInt("u_chunkX", chunk.coord.x);
            shader->SetInt("u_chunkZ", chunk.coord.z);
            int chunkSize = HEIGHT_MAP_SIZE / 8;
            int chunkWidth = chunkSize + 1;
            int chunkDepth = chunkSize + 1;
            int groupSizeX = (chunkWidth + 16 - 1) / 16;
            int groupSizeY = (chunkDepth + 16 - 1) / 16;
            glDispatchCompute(groupSizeX, groupSizeY, 1);
        }

        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT | GL_VERTEX_ATTRIB_ARRAY_BARRIER_BIT);

    }

    void GeneratePhysXTextures() {
        OpenGLFrameBuffer* worldFramebuffer = GetFrameBuffer("World");

        GLuint handle = worldFramebuffer->GetColorAttachmentHandleByName("HeightMap");
        GLint level = 0;
        GLint zOffset = 0;
        GLsizei width = 33;
        GLsizei height = 33;
        GLsizei depth = 1;
        GLenum format = GL_RED;
        GLenum type = GL_FLOAT;
        GLsizei numPixels = width * height * depth;
        GLsizei dataSize = numPixels * sizeof(float);
        std::vector<float> pixels(numPixels);

        std::cout << "WorldFrameBufferSize: " << worldFramebuffer->GetWidth() << " x " << worldFramebuffer->GetHeight() << "\n";

        glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);

        struct ChunkReadBackData {
            float vertices[VERTICES_PER_CHUNK];
        };

        int chunkCount = World::GetChunkCount();
        std::vector<ChunkReadBackData> chunkReadBackDataSet(chunkCount);

        // Readback height chunk data from gpu
        std::vector<HeightMapChunk>& chunks = World::GetHeightMapChunks();
        for (int i = 0; i < chunkCount; i++) {
            HeightMapChunk& chunk = chunks[i];
            GLint xOffset = chunk.coord.x * 32;
            GLint yOffset = chunk.coord.z * 32;

            if (xOffset + width > worldFramebuffer->GetWidth() ||
                yOffset + height > worldFramebuffer->GetHeight()) {
                std::cout << "YOU HAVE PROBLEMS: \n";
                std::cout << " - worldFramebuffer->GetWidth(): " << worldFramebuffer->GetWidth() << "\n";
                std::cout << " - worldFramebuffer->GetHeight(): " << worldFramebuffer->GetHeight() << "\n";
                std::cout << " - xOffset: " << xOffset << "\n";
                std::cout << " - yOffset: " << yOffset << "\n";
                std::cout << " - width: " << width << "\n";
                std::cout << " - height: " << height << "\n";
            }

            glGetTextureSubImage(handle, level, xOffset, yOffset, zOffset, width, height, depth, GL_RED, GL_FLOAT, dataSize, chunkReadBackDataSet[i].vertices);
        }

        Physics::MarkAllHeightFieldsForRemoval();

        // For each chunk determine the AABB
        for (int i = 0; i < chunkCount; i++) {
            HeightMapChunk& chunk = chunks[i];
            glm::vec3 aabbMin(std::numeric_limits<float>::max());
            glm::vec3 aabbMax(std::numeric_limits<float>::lowest());

            for (size_t j = 0; j < VERTICES_PER_CHUNK; j++) {
                float x = ((j % 33) + (chunk.coord.x * 32)) * HEIGHTMAP_SCALE_XZ;
                float y = chunkReadBackDataSet[i].vertices[j] * HEIGHTMAP_SCALE_Y;
                float z = ((j / 33) + (chunk.coord.z * 32)) * HEIGHTMAP_SCALE_XZ;

                glm::vec3 position(x, y, z);
                aabbMin = glm::min(aabbMin, position);
                aabbMax = glm::max(aabbMax, position);
            }
            chunk.aabbMin = aabbMin;
            chunk.aabbMax = aabbMax;

            vecXZ worldSpaceOffest = vecXZ(chunk.coord.x * CHUNK_SIZE_WORLDSPACE, chunk.coord.z * CHUNK_SIZE_WORLDSPACE);
            Physics::CreateHeightField(worldSpaceOffest, chunkReadBackDataSet[i].vertices);
       }
    }

    void DrawHeightMap() {
        OpenGLFrameBuffer* gBuffer = GetFrameBuffer("GBuffer");
        OpenGLHeightMapMesh& heightMapMesh = OpenGLBackEnd::GetHeightMapMesh();
        OpenGLShader* shader = GetShader("HeightMapColor");

        Transform transform;
        transform.scale = glm::vec3(HEIGHTMAP_SCALE_XZ, HEIGHTMAP_SCALE_Y, HEIGHTMAP_SCALE_XZ);
        glm::mat4 modelMatrix = transform.to_mat4();
        glm::mat4 inverseModelMatrix = glm::inverse(modelMatrix);

        gBuffer->Bind();
        gBuffer->DrawBuffers({ "BaseColor", "Normal", "RMA", "WorldPosition", "Emissive" });

        shader->Bind();
        shader->SetMat4("modelMatrix", modelMatrix);
        shader->SetMat4("inverseModelMatrix", inverseModelMatrix);
        shader->SetFloat("u_textureScaling", 1);

        SetRasterizerState("GeometryPass_NonBlended");

        Material* material = AssetManager::GetDefaultMaterial();
        int materialIndex = AssetManager::GetMaterialIndexByName("Ground_MudVeg");
        material = AssetManager::GetMaterialByIndex(materialIndex);

        Material* dirtRoadMaterial = AssetManager::GetMaterialByName("DirtRoad");

        if (Editor::IsOpen() && Editor::GetEditorMode() == EditorMode::HEIGHTMAP_EDITOR) {
            material = AssetManager::GetDefaultMaterial();
            shader->SetFloat("u_textureScaling", 0.1);
        }


        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, AssetManager::GetTextureByIndex(material->m_basecolor)->GetGLTexture().GetHandle());
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, AssetManager::GetTextureByIndex(material->m_normal)->GetGLTexture().GetHandle());
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, AssetManager::GetTextureByIndex(material->m_rma)->GetGLTexture().GetHandle());
        glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_2D, AssetManager::GetTextureByIndex(dirtRoadMaterial->m_basecolor)->GetGLTexture().GetHandle());
        glActiveTexture(GL_TEXTURE4);
        glBindTexture(GL_TEXTURE_2D, AssetManager::GetTextureByIndex(dirtRoadMaterial->m_normal)->GetGLTexture().GetHandle());
        glActiveTexture(GL_TEXTURE5);
        glBindTexture(GL_TEXTURE_2D, AssetManager::GetTextureByIndex(dirtRoadMaterial->m_rma)->GetGLTexture().GetHandle());
        glBindTextureUnit(6, AssetManager::GetTextureByName("RoadMask")->GetGLTexture().GetHandle());

        //int indexCount = (HEIGHT_MAP_SIZE - 1) * (HEIGHT_MAP_SIZE - 1) * 6;
        //int vertexCount = HEIGHT_MAP_SIZE * HEIGHT_MAP_SIZE;
        glBindVertexArray(heightMapMesh.GetVAO());

        int verticesPerChunk = 33 * 33;
        int verticesPerHeightMap = verticesPerChunk * 8 * 8;
        int indicesPerChunk = 32 * 32 * 6;
        int indicesPerHeightMap = indicesPerChunk * 8 * 8;

        int heightMapCount = World::GetHeightMapCount();

        int culled = 0;

        for (int i = 0; i < 4; i++) {
            Viewport* viewport = ViewportManager::GetViewportByIndex(i);
            Frustum& frustum = viewport->GetFrustum();

            int test = 0;
            if (viewport->IsVisible()) {
                OpenGLRenderer::SetViewport(gBuffer, viewport);
                std::vector<HeightMapChunk>& chunks = World::GetHeightMapChunks();

                //std::cout << "chunks.size(): " << chunks.size() << "\n";
                
                for (HeightMapChunk& chunk : chunks) {

                    if (Editor::IsClosed()) {
                        if (!frustum.IntersectsAABBFast(AABB(chunk.aabbMin, chunk.aabbMax))) {
                            culled++;
                            continue;
                        }
                    }
                   // std::cout << "hi\n";

                    shader->SetInt("u_test", test++);

                    int indexCount = INDICES_PER_CHUNK;
                    int baseVertex = 0;
                    int baseIndex = chunk.baseIndex;
                    void* indexOffset = (GLvoid*)(baseIndex * sizeof(GLuint));
                    int instanceCount = 1;
                    int viewportIndex = i;
                    glDrawElementsInstancedBaseVertexBaseInstance(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, indexOffset, instanceCount, baseVertex, viewportIndex);
                    // Draw chunk vertices as points
                    // glDrawArraysInstancedBaseInstance(GL_POINTS, chunk.baseVertex, VERTICES_PER_CHUNK, instanceCount, viewportIndex); 
                }
            }
        }
        glBindVertexArray(0);
        //std::cout << "Culled: " << culled << "\n";
    }

    void SaveHeightMap() {
        Audio::PlayAudio(AUDIO_SELECT, 1.00f);

        OpenGLTextureArray& heightmapTextureArray = HeightMapManager::GetGLTextureArray();

        GLuint textureHandle = heightmapTextureArray.GetHandle();
        GLuint width = heightmapTextureArray.GetWidth();
        GLuint height = heightmapTextureArray.GetHeight();
        int layerIndex = 0;
        size_t dataSize = width * height * sizeof(float);

        HeightMapData heightMapData;
        heightMapData.name = "TEST";
        heightMapData.width = width;
        heightMapData.height = height;
        heightMapData.data.resize(heightMapData.width * heightMapData.height);

        // Readback
        glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
        glGetTextureSubImage(textureHandle, 0, 0, 0, layerIndex, width, height, 1, GL_RED, GL_FLOAT, dataSize, heightMapData.data.data());

        // Write file
        File::SaveHeightMap(heightMapData);
    }

    void SaveHeightMaps() {
        std::cout << "Renderer::SaveHeightMaps()\n";

        OpenGLFrameBuffer* worldFramebuffer = GetFrameBuffer("World");
        GLuint textureHandle = worldFramebuffer->GetColorAttachmentHandleByName("HeightMap");
        GLuint width = HEIGHT_MAP_SIZE;
        GLuint height = HEIGHT_MAP_SIZE;
        size_t dataSize = width * height * sizeof(float);

        HeightMapData heightMapData;
        heightMapData.width = width;
        heightMapData.height = height;
        heightMapData.data.resize(heightMapData.width * heightMapData.height);

        MapCreateInfo* mapCreateInfo = MapManager::GetHeightMapEditorMapCreateInfo();
        if (mapCreateInfo) {
            int mapWidth = World::GetMapWidth();
            int mapDepth = World::GetMapDepth();

            // Iterate the map, save any sectors height map that isn't none
            for (int x = 0; x < mapWidth; x++) {
                for (int z = 0; z < mapDepth; z++) {
                    const std::string& heightMapName = World::GetHeightMapNameAtLocation(x, z);
                    if (heightMapName != "") {
                        int xOffset = x * HEIGHT_MAP_SIZE;
                        int yOffset = z * HEIGHT_MAP_SIZE;
                        glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
                        glGetTextureSubImage(textureHandle, 0, xOffset, yOffset, 0, width, height, 1, GL_RED, GL_FLOAT, dataSize, heightMapData.data.data());
                        heightMapData.name = heightMapName;
                        File::SaveHeightMap(heightMapData);
                    }
                }
            }           
        }

    }
}