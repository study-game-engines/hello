#include "HouseManager.h"
#include "Core/JSON.h"
#include "Util.h"

#include <map>


namespace HouseManager {

    std::map<std::string, HouseCreateInfo> g_houses;

    void Init() {
        LoadAllHouseFilesFromDisk();
    }

    void LoadAllHouseFilesFromDisk() {
        g_houses.clear();
        for (FileInfo& fileInfo : Util::IterateDirectory("res/houses/", { "json" })) {
            g_houses[fileInfo.name] = JSON::LoadHouse(fileInfo.path);
        }
    }

    HouseCreateInfo* GetHouseCreateInfoByFilename(const std::string& filename) {
        auto it = g_houses.find(filename);
        if (it == g_houses.end()) {
            return nullptr;
        }
        else {
            return &(it->second);
        }
    }

    /*
    HouseCreateInfo GetTestHouseCreateInfo() {

        HouseCreateInfo houseCreateInfo; 
        
        DoorCreateInfo& doorCreateInfo = houseCreateInfo.doors.emplace_back();
        doorCreateInfo.position = glm::vec3(0.05f, 0.0f, 0.0f);
        doorCreateInfo.rotation = glm::vec3(0.0f, 0.0f, 0.0f);

        DoorCreateInfo& doorCreateInfo2 = houseCreateInfo.doors.emplace_back();
        doorCreateInfo2.position = glm::vec3(2.2f, 0.0f, -2.95f);
        doorCreateInfo2.rotation = glm::vec3(0.0f, HELL_PI * 0.5f, 0.0f);

        WindowCreateInfo& windowCreateInfo = houseCreateInfo.windows.emplace_back();
        windowCreateInfo.position = glm::vec3(0.05f, 0.0f, -1.5f);
        windowCreateInfo.rotation = glm::vec3(0.0f, 0.0f, 0.0f);

        WindowCreateInfo& windowCreateInfo2 = houseCreateInfo.windows.emplace_back();
        windowCreateInfo2.position = glm::vec3(0.05f, 0.0f, 1.5f);
        windowCreateInfo2.rotation = glm::vec3(0.0f, 0.0f, 0.0f);

        WindowCreateInfo& windowCreateInfo3 = houseCreateInfo.windows.emplace_back();
        windowCreateInfo3.position = glm::vec3(2.2f, 0.0f, 2.95f);
        windowCreateInfo3.rotation = glm::vec3(0.0f, HELL_PI * 0.5f, 0.0f);

        WallCreateInfo& wallCreateInfo = houseCreateInfo.walls.emplace_back();
        wallCreateInfo.points.push_back(glm::vec3(0.0f, 0.0f, -3.0f));
        wallCreateInfo.points.push_back(glm::vec3(0.0f, 0.0f, 6.1f));
        wallCreateInfo.points.push_back(glm::vec3(4.5f, 0.0f, 6.1f));
        wallCreateInfo.points.push_back(glm::vec3(4.5f, 0.0f, -3.0f));
        wallCreateInfo.points.push_back(glm::vec3(0.0f, 0.0f, -3.0f));
        wallCreateInfo.materialName = "Ceiling2";
        wallCreateInfo.textureOffsetU = 0.0f;
        wallCreateInfo.textureOffsetV = -1.0f;
        wallCreateInfo.textureScale = 1.0f;
        wallCreateInfo.height = 2.4f;

        WallCreateInfo& wallCreateInfo2 = houseCreateInfo.walls.emplace_back();
        wallCreateInfo2.points.push_back(glm::vec3(0.1f, 0.0f, -2.9f));
        wallCreateInfo2.points.push_back(glm::vec3(4.4f, 0.0f, -2.9f));
        wallCreateInfo2.points.push_back(glm::vec3(4.4f, 0.0f, 2.9f));
        wallCreateInfo2.points.push_back(glm::vec3(0.1f, 0.0f, 2.9f));
        wallCreateInfo2.points.push_back(glm::vec3(0.1f, 0.0f, -2.9f));
        wallCreateInfo2.materialName = "WallPaper";
        wallCreateInfo2.materialName = "Ceiling2";
        wallCreateInfo2.textureOffsetU = 0.0f;
        wallCreateInfo2.textureOffsetV = -1.4f;
        wallCreateInfo2.textureScale = 1 / 2.4f;
        wallCreateInfo2.height = 2.4f;
        wallCreateInfo2.ceilingTrimType = TrimType::TIMBER;
        wallCreateInfo2.floorTrimType = TrimType::TIMBER;
       
        WallCreateInfo& wallCreateInfo3 = houseCreateInfo.walls.emplace_back();
        wallCreateInfo3.points.push_back(glm::vec3(0.1f, 0.0f, 3.0f));
        wallCreateInfo3.points.push_back(glm::vec3(4.4f, 0.0f, 3.0f));
        wallCreateInfo3.points.push_back(glm::vec3(4.4f, 0.0f, 6.0f));
        wallCreateInfo3.points.push_back(glm::vec3(0.1f, 0.0f, 6.0f));
        wallCreateInfo3.points.push_back(glm::vec3(0.1f, 0.0f, 3.0f));
        wallCreateInfo3.materialName = "WallPaper";
        wallCreateInfo3.materialName = "Ceiling2";
        wallCreateInfo3.textureOffsetU = 0.0f;
        wallCreateInfo3.textureOffsetV = -1.4f;
        wallCreateInfo3.textureScale = 1 / 2.4f;
        wallCreateInfo3.height = 2.4f;
        wallCreateInfo3.ceilingTrimType = TrimType::TIMBER;
        wallCreateInfo3.floorTrimType = TrimType::TIMBER;

        PlaneCreateInfo& housePlaneCreateInfo = houseCreateInfo.planes.emplace_back();
        housePlaneCreateInfo.p0 = glm::vec3(0.0f, 0.0f, -3.0f);
        housePlaneCreateInfo.p1 = glm::vec3(0.0f, 0.0f, 6.1f);
        housePlaneCreateInfo.p2 = glm::vec3(4.4f, 0.0f, 6.1f);
        housePlaneCreateInfo.p3 = glm::vec3(4.4f, 0.0f, -3.0f);
        housePlaneCreateInfo.textureScale = 0.4f;
        housePlaneCreateInfo.materialName = "FloorBoards";

        PlaneCreateInfo& housePlaneCreateInfo2 = houseCreateInfo.planes.emplace_back();
        housePlaneCreateInfo2.p3 = glm::vec3(0.0f, 2.4f, -2.9f);
        housePlaneCreateInfo2.p2 = glm::vec3(0.0f, 2.4f, 6.1f);
        housePlaneCreateInfo2.p1 = glm::vec3(4.4f, 2.4f, 6.1f);
        housePlaneCreateInfo2.p0 = glm::vec3(4.4f, 2.4f, -2.9f);
        housePlaneCreateInfo2.textureScale = 1.0f;
        housePlaneCreateInfo2.materialName = "Ceiling2";

        PianoCreateInfo& pianoCreateInfo = houseCreateInfo.pianos.emplace_back();
        pianoCreateInfo.position = glm::vec3(4.35f, 0.0f, 1.0f);
        pianoCreateInfo.rotation.y = HELL_PI * -0.5f;

        PianoCreateInfo& pianoCreateInfo2 = houseCreateInfo.pianos.emplace_back();
        pianoCreateInfo2.position = glm::vec3(3.00f, 0.0f, -2.14f);
        pianoCreateInfo2.rotation.y = -0.75f;

        LightCreateInfo& lightCreateInfo = houseCreateInfo.lights.emplace_back();
        lightCreateInfo.position = glm::vec3(2.2f, 2.175f, 0.0f);
        lightCreateInfo.color = DEFAULT_LIGHT_COLOR;
        lightCreateInfo.type = LightType::HANGING_LIGHT;
        lightCreateInfo.radius = 8.5f;
        lightCreateInfo.strength = 1.25f;

        return houseCreateInfo;
    }*/
}