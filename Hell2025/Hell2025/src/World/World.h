#pragma once
#include "HellTypes.h"
#include "Core/Debug.h"
#include "Types/Game/AnimatedGameObject.h"
#include "Types/Game/Bullet.h"
#include "Types/Game/BulletCasing.h"
#include "Types/Game/Decal.h"
#include "Types/Game/GameObject.h"
#include "Types/Game/Light.h"
#include "Types/Game/PickUp.h"
#include "Types/House/Door.h"
#include "Types/House/HousePlane.h"
#include "Types/House/Wall.h"
#include "Types/House/Window.h"
#include "Types/Misc/Piano.h"
#include "Types/Exterior/Tree.h"
#include "Util/Util.h"
#include "glm/gtx/intersect.hpp"
#include <vector>
#include "Modelling/Clipping.h"

#include "API/OpenGL/Types/GL_detachedMesh.hpp"

struct ViewportBvhData {
    std::vector<PrimitiveInstance> instances;
    uint64_t sceneBvhId;
    RayTraversalResult closestHit;
};

namespace World {
    void Init();
    void BeginFrame();
    void Update(float deltaTime);
    void SubmitRenderItems();

    void LoadMap(const std::string& mapName);
    void LoadMap(MapCreateInfo* mapCreateInfo);
    void LoadSingleSector(SectorCreateInfo* sectorCreateInfo);
    void LoadDeathMatchMap();

    void ResetWorld();
    void LoadEmptyWorld();
    void NewCampainWorld();

    bool ChunkExists(int x, int z);
    const uint32_t GetChunkCountX();
    const uint32_t GetChunkCountZ();
    const uint32_t GetChunkCount(); 
    const HeightMapChunk* GetChunk(int x, int z);

    void AddBullet(BulletCreateInfo createInfo);
    void AddDoorBasic(BasicDoorCreateInfo createInfo);
    void AddBulletCasing(BulletCasingCreateInfo createInfo, SpawnOffset spawnOffset = SpawnOffset());
    void AddDecal(const DecalCreateInfo& createInfo);
    void AddGameObject(GameObjectCreateInfo createInfo, SpawnOffset spawnOffset = SpawnOffset());
    void AddLight(LightCreateInfo createInfo, SpawnOffset spawnOffset = SpawnOffset());
    void AddPickUp(PickUpCreateInfo createInfo, SpawnOffset spawnOffset = SpawnOffset());
    void AddTree(TreeCreateInfo createInfo, SpawnOffset spawnOffset = SpawnOffset());

    // Creation
    void CreateGameObject();
    void CreateAnimatedGameObject();

    // Piano
    Piano* GetPianoByPianoId(uint64_t objectId);

    // Getters

    // Removal
    void RemovePickUp(uint64_t objectID);
    
    // BVH
    void UpdateSceneBvh();
    void TestBvh();
    RayTraversalResult ClosestHit(glm::vec3 rayOrigin, glm::vec3 rayDir, float maxRayDistance, int viewportIndex);

    const float GetWorldSpaceWidth();
    const float GetWorldSpaceDepth();
    const uint32_t GetMapWidth();
    const uint32_t GetMapDepth();
    const uint32_t GetHeightMapCount();
    const std::string& GetSectorNameAtLocation(int x, int z);
    const std::string& GetHeightMapNameAtLocation(int x, int z);
    bool IsMapCellInRange(int x, int z);
    void PrintMapCreateInfoDebugInfo();

    // Map
    const std::string& GetCurrentMapName();

    void UpdateHouseMeshVertexDataAndRenderItems();

    OpenGLDetachedMesh& GetHouseMesh();

    Door* GetDoorByObjectId(uint64_t objectID);
    PickUp* GetPickUpByObjectId(uint64_t objectID);
    GameObject* GetGameObjectByIndex(int32_t index);
    GameObject* GetGameObjectByName(const std::string& name);
    Light* GetLightByIndex(int32_t index);
    PickUp* GetPickUpByIndex(int32_t index);
    //Piano* GetPianos(int32_t index);
    Tree* GetTreeByIndex(int32_t index);

    std::vector<AnimatedGameObject>& GetAnimatedGameObjects();
    std::vector<Bullet>& GetBullets();
    std::vector<BulletCasing>& GetBulletCasings();
    std::vector<ClippingCube>& GetClippingCubes();
    std::vector<Decal>& GetDecals();
    std::vector<Door>& GetDoors();
    std::vector<GameObject>& GetGameObjects();
    std::vector<HeightMapChunk>& GetHeightMapChunks();
    std::vector<HousePlane>& GetHousePlanes();
    std::vector<HouseRenderItem>& GetHouseRenderItems();
    std::vector<Light>& GetLights();
    std::vector<Piano>& GetPianos();
    std::vector<PickUp>& GetPickUps();
    std::vector<Transform>& GetDoorAndWindowCubeTransforms();
    std::vector<Tree>& GetTrees();
    std::vector<Wall>& GetWalls();
    std::vector<Window>& GetWindows();

    std::vector<RenderItem>& GetRenderItems();
    std::vector<RenderItem>& GetRenderItemsBlended();
    std::vector<RenderItem>& GetRenderItemsAlphaDiscarded();
    std::vector<RenderItem>& GetRenderItemsHairTopLayer();
    std::vector<RenderItem>& GetRenderItemsHairBottomLayer();
    std::vector<RenderItem>& GetSkinnedRenderItems();
}

/*
    Notes
    
    * Sectors
      - Each sector can or can not have a house
      - There is always a house in the first sector you DON'T spawn in
      - Each sector has a 50% chance of having a house
      - There can never be 2 sectors WITHOUT a house in a row
      - If a sector has no house, then it may be a mermaid shop sector
      - If a sector has no house, then it may be a dense woods sector, aka trees and full of enemies

    * Witches
      - they are a rarer shop than the mermaid shop
      - they sell better items
      - they hover above a fire 

*/