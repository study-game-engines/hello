#include "World.h"
#include "AssetManagement/AssetManager.h"
#include "Core/JSON.h"
#include "Renderer/GlobalIllumination.h"
#include "Renderer/RenderDataManager.h"
#include "World/HouseManager.h"

namespace World {

    MeshBuffer g_houseMeshBuffer;
    MeshBuffer g_weatherBoardMeshBuffer;
    

    void AddHouse(HouseCreateInfo houseCreateInfo, SpawnOffset spawnOffset) {
        for (DoorCreateInfo& createInfo : houseCreateInfo.doors) {
            AddDoor(createInfo, spawnOffset);
        }
        for (PlaneCreateInfo& createInfo : houseCreateInfo.planes) {
            AddHousePlane(createInfo, spawnOffset);
        }
        for (LightCreateInfo& createInfo : houseCreateInfo.lights) {
            AddLight(createInfo, spawnOffset);
        }
        for (PianoCreateInfo& createInfo : houseCreateInfo.pianos) {
            AddPiano(createInfo, spawnOffset);
        }
        for (PictureFrameCreateInfo& createInfo : houseCreateInfo.pictureFrames) {
            AddPictureFrame(createInfo, spawnOffset);
        }
        for (WindowCreateInfo& createInfo : houseCreateInfo.windows) {
            AddWindow(createInfo, spawnOffset);
        }

        // Update the clipping cubes, so the walls have correct cut outs for doors/windows
        UpdateClippingCubes();
        
        for (WallCreateInfo& createInfo : houseCreateInfo.walls) {
            AddWall(createInfo, spawnOffset);
        }

        UpdateHouseMeshBuffer();
        UpdateWeatherBoardMeshBuffer();

        //GlobalIllumination::CreatePointCloud();

        // Hack in a christmas tree
        ChristmasTreeCreateInfo christmasTreeCreateInfo;
        christmasTreeCreateInfo.position = glm::vec3(8.13f, 0.15f, 1.2f);
        christmasTreeCreateInfo.rotation.y = Util::RandomFloat(0, HELL_PI);
        AddChristmasTree(christmasTreeCreateInfo, spawnOffset);

        christmasTreeCreateInfo.position = glm::vec3(0.78f, 0.15f, 2.25f);
        christmasTreeCreateInfo.rotation.y = Util::RandomFloat(0, HELL_PI);
        AddChristmasTree(christmasTreeCreateInfo, spawnOffset);
    }

    void SaveHouse() {

        std::string filename = "TestHouse.json";

        HouseCreateInfo houseCreateInfo;

        for (Door& door : GetDoors()) {
            houseCreateInfo.doors.emplace_back(door.GetCreateInfo());
        }
        for (Light& light : GetLights()) {
            houseCreateInfo.lights.emplace_back(light.GetCreateInfo());
        }
        for (Piano& piano : GetPianos()) {
            houseCreateInfo.pianos.emplace_back(piano.GetCreateInfo());
        }
        for (Plane& plane : GetPlanes()) {
            houseCreateInfo.planes.emplace_back(plane.GetCreateInfo());
        }
        for (PictureFrame& pictureFrame : GetPictureFrames()) {
            houseCreateInfo.pictureFrames.emplace_back(pictureFrame.GetCreateInfo());
        }
        for (Window& window : GetWindows()) {
            houseCreateInfo.windows.emplace_back(window.GetCreateInfo());
        }
        for (Wall& wall: GetWalls()) {
            houseCreateInfo.walls.emplace_back(wall.GetCreateInfo());
        }

        JSON::SaveHouse("res/houses/" + filename, houseCreateInfo);
    }


    void UpdateHouseMeshBuffer() {
        g_houseMeshBuffer.Reset();

        for (Wall& wall : GetWalls()) {
            for (WallSegment& wallSegment : wall.GetWallSegments()) {
                uint32_t meshIndex = g_houseMeshBuffer.AddMesh(wallSegment.GetVertices(), wallSegment.GetIndices());
                wallSegment.SetMeshIndex(meshIndex);
            }
        }
        for (Plane& housePlane : GetPlanes()) {
            uint32_t meshIndex = g_houseMeshBuffer.AddMesh(housePlane.GetVertices(), housePlane.GetIndices());
            housePlane.SetMeshIndex(meshIndex);
        }

        g_houseMeshBuffer.UpdateBuffers();
    }

    void UpdateWeatherBoardMeshBuffer() {
        g_weatherBoardMeshBuffer.Reset();

        std::vector<Vertex> vertices;
        std::vector<uint32_t> indices;

        for (Wall& wall : GetWalls()) {

            for (BoardVertexData& boardVertexData : wall.m_boardVertexDataSet) {
                uint32_t baseVertex = vertices.size();
                vertices.insert(vertices.end(), boardVertexData.vertices.begin(), boardVertexData.vertices.end());

                for (uint32_t& index : boardVertexData.indices) {
                    indices.push_back(index + baseVertex);
                }
            }
        }

        g_weatherBoardMeshBuffer.GetGLMeshBuffer().ReleaseBuffers();
        g_weatherBoardMeshBuffer.GetGLMeshBuffer().UpdateBuffers(vertices, indices);
    }

    void ResetWeatherboardMeshBuffer() {
        g_weatherBoardMeshBuffer.Reset();
    }

    MeshBuffer& GetHouseMeshBuffer() {
        return g_houseMeshBuffer;
    }

    MeshBuffer& GetWeatherBoardMeshBuffer() {
        return g_weatherBoardMeshBuffer;
    }

    Mesh* GetHouseMeshByIndex(uint32_t meshIndex) {
        return g_houseMeshBuffer.GetMeshByIndex(meshIndex);
    }
}