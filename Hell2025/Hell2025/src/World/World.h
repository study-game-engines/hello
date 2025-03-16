#pragma once
#include "HellTypes.h"
#include "AssetManagement/AssetManager.h"
#include "Core/Debug.h"
#include "Types/AnimatedGameObject.h"
#include "Types/Bullet.h"
#include "Types/BulletCasing.h"
#include "Types/Decal.h"
#include "Types/Door.h"
#include "Types/Floor.h"
#include "Types/GameObject.h"
#include "Types/Light.h"
#include "Types/PickUp.h"
#include "Types/Tree.h"
#include "Types/Wall.h"
#include "Types/Window.h"
#include "Util/Util.h"
#include "glm/gtx/intersect.hpp"
#include <vector>
#include "Modelling/Clipping.h"

#include "API/OpenGL/Types/GL_detachedMesh.hpp"

namespace World {


    OpenGLDetachedMesh& GetHouseMesh();



    void NewCampainWorld();

    void Init();
    void LoadMap(const std::string& mapName);
    void LoadMap(MapCreateInfo* mapCreateInfo);
    //void LoadSingleSector(const std::string& sectorName);
    void LoadSingleSector(SectorCreateInfo* sectorCreateInfo);

    //void RevertToSavedSector();


    void LoadDeathMatchMap();

    //SectorCreateInfo& GetEditorSectorCreateInfo();

    //MapCreateInfo* GetCurrentMapCreateInfo();
    std::vector<HeightMapChunk>& GetHeightMapChunks();

    void BeginFrame();
    void Update(float deltaTime);
    void ResetWorld();
    void LoadEmptyWorld();

    bool ChunkExists(int x, int z);
    const uint32_t GetMapWorldSpaceWidth();
    const uint32_t GetMapWorldSpaceDepth();
    const uint32_t GetChunkCountX();
    const uint32_t GetChunkCountZ();
    const uint32_t GetChunkCount(); 
    const HeightMapChunk* GetChunk(int x, int z);

    std::vector<ClippingCube>& GetClippingCubes();

    // Pick Ups
    PickUp* GetPickUpByObjectId(uint64_t objectID);
    void RemovePickUp(uint64_t objectID);

    // Doors
    Door* GetDoorByObjectId(uint64_t objectID);
    
    void CreateGameObject();
    void CreateAnimatedGameObject();

    //AnimatedGameObject* GetAnimatedGameObjectByIndex(int32_t index);
    GameObject* GetGameObjectByIndex(int32_t index);
    GameObject* GetGameObjectByName(const std::string& name);
    Light* GetLightByIndex(int32_t index);
    PickUp* GetPickUpByIndex(int32_t index);
    Tree* GetTreeByIndex(int32_t index);

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

    std::vector<GameObject>& GetGameObjects();
    std::vector<Bullet>& GetBullets();
    std::vector<BulletCasing>& GetBulletCasings();
    std::vector<Decal>& GetDecals();
    std::vector<Door>& GetDoors();
    std::vector<Floor>& GetFloors();
    std::vector<Light>& GetLights();
    std::vector<PickUp>& GetPickUps();
    std::vector<Tree>& GetTrees();
    std::vector<Wall>& GetWalls();
    std::vector<Window>& GetWindows();

    std::vector<RenderItem>& GetRenderItems();
    std::vector<RenderItem>& GetRenderItemsBlended();
    std::vector<RenderItem>& GetRenderItemsAlphaDiscarded();
    std::vector<RenderItem>& GetRenderItemsHairTopLayer();
    std::vector<RenderItem>& GetRenderItemsHairBottomLayer();
    std::vector<RenderItem>& GetSkinnedRenderItems();

    void AddBullet(BulletCreateInfo createInfo);
    void AddBulletCasing(BulletCasingCreateInfo createInfo, SpawnOffset spawnOffset = SpawnOffset());
    void AddDecal(const DecalCreateInfo& createInfo);
    void AddGameObject(GameObjectCreateInfo createInfo, SpawnOffset spawnOffset = SpawnOffset());
    void AddLight(LightCreateInfo createInfo, SpawnOffset spawnOffset = SpawnOffset());
    void AddPickUp(PickUpCreateInfo createInfo, SpawnOffset spawnOffset = SpawnOffset());
    void AddTree(TreeCreateInfo createInfo, SpawnOffset spawnOffset = SpawnOffset());
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