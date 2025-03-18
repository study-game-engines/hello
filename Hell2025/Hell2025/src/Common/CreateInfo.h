#pragma once
#include "HellEnums.h"
#include "HellTypes.h"
#include "HellDefines.h"
#include <map>

struct DecalCreateInfo {
    DecalType decalType;
    PhysicsType parentPhysicsType = PhysicsType::UNDEFINED;
    ObjectType parentObjectType = ObjectType::UNDEFINED;
    glm::vec3 surfaceHitPosition = glm::vec3(0.0f);
    glm::vec3 surfaceHitNormal = glm::vec3(0.0f);
    uint64_t parentPhysicsId = 0;
    uint64_t parentObjectId = 0;
};

struct DoorCreateInfo {
    glm::vec3 position;
    glm::vec3 rotation;
};

struct WindowCreateInfo {
    glm::vec3 position;
    glm::vec3 rotation;
};

struct WallCreateInfo {
    std::vector<glm::vec3> points;
    std::string materialName = "";
    float height;
    float textureScale;
    float textureOffsetX;
    float textureOffsetY;
};

struct LightCreateInfo {
    glm::vec3 position = glm::vec3(0.0f);
    glm::vec3 color = glm::vec3(1, 0.7799999713897705, 0.5289999842643738);
    float radius = 6.0f;
    float strength = 1.0f;
    std::string type = "LAMP_POST";
};

struct BulletCreateInfo {
    glm::vec3 origin = glm::vec3(0);
    glm::vec3 direction = glm::vec3(0);
    int32_t weaponIndex = 0;
    uint32_t damage = 0;
};

struct PickUpCreateInfo {
    glm::vec3 position = glm::vec3(0.0f);
    glm::vec3 rotation = glm::vec3(0.0f);
    std::string pickUpType = "";
};

struct SpriteSheetObjectCreateInfo {
    glm::vec3 position = glm::vec3(0.0f);
    glm::vec3 rotation = glm::vec3(0.0f);
    glm::vec3 scale = glm::vec3(1.0f);
    bool loop = false;
    bool billboard = true;
    bool renderingEnabled = true;
    float animationSpeed = 1.0f;
    std::string textureName = "";
};

struct BulletCasingCreateInfo {
    glm::vec3 position = glm::vec3(0.0f);
    glm::vec3 rotation = glm::vec3(0.0f);
    glm::vec3 force = glm::vec3(0.0f);
    float mass = 0.0f;
    uint32_t modelIndex = 0;
    uint32_t materialIndex = 0;
};

struct GameObjectCreateInfo {
    glm::vec3 position = glm::vec3(0.0f);
    glm::vec3 rotation = glm::vec3(0.0f);
    glm::vec3 scale = glm::vec3(1.0f);
    std::string modelName = "";
    std::vector<MeshRenderingInfo> meshRenderingInfoSet;
};

struct TreeCreateInfo {
    glm::vec3 position = glm::vec3(0.0f);
    glm::vec3 rotation = glm::vec3(0.0f);
    glm::vec3 scale = glm::vec3(1.0f);
    int type = 0;
};

struct SectorCreateInfo {
    std::string sectorName;
    std::string heightMapName;
    std::vector<GameObjectCreateInfo> gameObjects;
    std::vector<LightCreateInfo> lights;
    std::vector<PickUpCreateInfo> pickUps;
    std::vector<TreeCreateInfo> trees;

    // TO DO:
    //std::vector<InteractTextCreateInfo> interactTexts;
    //std::vector<HouseLocation> houseLocations;
    //glm::vec3 mermaidsLocation;
};

struct MapCreateInfo {
    std::string name;
    uint32_t width = 4;
    uint32_t depth = 4;
    ivecXZ spawnCoords = ivecXZ(0, 0);
    std::string m_sectorNames[MAX_MAP_WIDTH][MAX_MAP_DEPTH];
};