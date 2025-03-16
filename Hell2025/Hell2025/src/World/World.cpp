#include "World.h"
#include "CreateInfo.h"
#include "HellDefines.h"
#include "HellTypes.h"
#include "UniqueID.h"
#include "Util.h"

#include "Core/Audio.h"
#include "Core/Game.h"
#include "Input/Input.h"
#include "Renderer/Renderer.h"
#include "Renderer/RenderDataManager.h"
#include "World/MapManager.h"
#include "World/SectorManager.h"

namespace World {
    std::vector<Light> g_lights;
    std::vector<AnimatedGameObject> g_animatedGameObjects;
    std::vector<Bullet> g_bullets;
    std::vector<BulletCasing> g_bulletCasings;
    std::vector<Door> g_doors;
    std::vector<Decal> g_decals;
    std::vector<GameObject> g_gameObjects;
    std::vector<PickUp> g_pickUps;
    std::vector<Tree> g_trees;

    std::vector<RenderItem> g_renderItems;
    std::vector<RenderItem> g_renderItemsBlended;
    std::vector<RenderItem> g_renderItemsAlphaDiscarded;
    std::vector<RenderItem> g_renderItemsHairTopLayer;
    std::vector<RenderItem> g_renderItemsHairBottomLayer;
    std::vector<RenderItem> g_skinnedRenderItems;

    std::vector<ClippingCube> g_clippingCubes;

    std::vector<HeightMapChunk> g_heightMapChunks;
    std::map<ivecXZ, int> g_validChunks;

    std::vector<Floor> g_floors;
    std::vector<Wall> g_walls;
    std::vector<Window> g_windows;

    OpenGLDetachedMesh g_houseMesh;
    std::vector<Vertex> g_houseMeshVertices;
    std::vector<uint32_t> g_houseMeshIndices;

    // Map
    std::string g_mapName = "";
    uint32_t g_mapWidth = 0;
    uint32_t g_mapDepth = 0;
    std::string g_sectorNames[MAX_MAP_WIDTH][MAX_MAP_DEPTH];
    std::string g_heightMapNames[MAX_MAP_WIDTH][MAX_MAP_DEPTH];

    void RecreateHieghtMapChunks();
    void AddSectorAtLocation(SectorCreateInfo& sectorCreateInfo, SpawnOffset spawnOffset);
    void ProcessBullets();
    void UpdateHouseMeshVertexData();

    void Init() {
        LoadMap("TestMap");
    }

    void BeginFrame() {
        // Clear all bullets
        g_bullets.clear();

        for (GameObject& gameObject : g_gameObjects) {
            gameObject.BeginFrame();
        }
        for (Tree& tree : g_trees) {
            tree.BeginFrame();
        }
    }

    void UpdateHouseMeshVertexData() {
        g_houseMeshVertices.clear();
        g_houseMeshIndices.clear();

       for (Wall& wall : g_walls) {
           int baseVertex = g_houseMeshVertices.size();
           int baseIndex = g_houseMeshIndices.size();
           wall.UpdateRenderItems(baseVertex, baseIndex);
           std::vector<Vertex>& localVertices = wall.GetVertices();
           std::vector<uint32_t>& localIndices = wall.GetIndices();
           g_houseMeshVertices.insert(g_houseMeshVertices.end(), localVertices.begin(), localVertices.end());
           g_houseMeshIndices.insert(g_houseMeshIndices.end(), localIndices.begin(), localIndices.end());
       }

       
       
       for (Floor& floor : g_floors) {
           int baseVertex = g_houseMeshVertices.size();
           int baseIndex = g_houseMeshIndices.size();       
           floor.UpdateRenderItem(baseVertex, baseIndex);              
           std::vector<Vertex>& localVertices = floor.GetVertices();
           std::vector<uint32_t>& localIndices = floor.GetIndices();
           g_houseMeshVertices.insert(g_houseMeshVertices.end(), localVertices.begin(), localVertices.end());
           g_houseMeshIndices.insert(g_houseMeshIndices.end(), localIndices.begin(), localIndices.end());
       }


        g_houseMesh.UpdateBuffers(g_houseMeshVertices, g_houseMeshIndices);
    }

    void Update(float deltaTime) {
        
        //for (ClippingCube& clippingCube : g_clippingCubes) {
        //    clippingCube.DrawDebugCorners(OUTLINE_COLOR);
        //    clippingCube.DrawDebugEdges(WHITE);
        //}

        for (Decal& decal : g_decals) {
          //  Renderer::DrawPoint(decal.GetPosition(), GREEN);
            //Renderer::DrawLine(p1, p2, GREEN);

        }

        ProcessBullets();

        if (Input::KeyPressed(HELL_KEY_BACKSPACE)) {
            //MapCreateInfo* currentMapCreateInfo = GetCurrentMapCreateInfo();
            //MapManager::PrintMapCreateInfoDebugInfo(currentMapCreateInfo);

            PickUpCreateInfo createInfo;
            createInfo.position = Game::GetLocalPlayerByIndex(0)->GetCameraPosition();
            createInfo.position += Game::GetLocalPlayerByIndex(0)->GetCameraForward();
            createInfo.rotation.x = Util::RandomFloat(-HELL_PI, HELL_PI);
            createInfo.rotation.y = Util::RandomFloat(-HELL_PI, HELL_PI);
            createInfo.rotation.z = Util::RandomFloat(-HELL_PI, HELL_PI);
            createInfo.pickUpType = Util::PickUpTypeToString(PickUpType::AKS74U);
            AddPickUp(createInfo);            
        }

        if (Input::KeyPressed(HELL_KEY_INSERT)) {
            //MapCreateInfo* currentMapCreateInfo = GetCurrentMapCreateInfo();
            //MapManager::PrintMapCreateInfoDebugInfo(currentMapCreateInfo);

            PickUpCreateInfo createInfo;
            createInfo.position = Game::GetLocalPlayerByIndex(0)->GetCameraPosition();
            createInfo.position += Game::GetLocalPlayerByIndex(0)->GetCameraForward();
            createInfo.rotation.x = Util::RandomFloat(-HELL_PI, HELL_PI);
            createInfo.rotation.y = Util::RandomFloat(-HELL_PI, HELL_PI);
            createInfo.rotation.z = Util::RandomFloat(-HELL_PI, HELL_PI);
            createInfo.pickUpType = Util::PickUpTypeToString(PickUpType::REMINGTON_870);
            AddPickUp(createInfo);
        }



       //if (Input::KeyPressed(HELL_KEY_SPACE)) {
       //    if (OpenGLRenderer::IsMouseRayWorldPositionReadBackReady()) {
       //        TreeCreateInfo treeCreateInfo;
       //        treeCreateInfo.position = OpenGLRenderer::GetMouseRayWorldPostion();
       //        treeCreateInfo.type = (int)TreeType::TREE_LARGE_0;
       //        g_editorSector.trees.push_back(treeCreateInfo);
       //        AddTree(treeCreateInfo);
       //        Audio::PlayAudio(AUDIO_SELECT, 1.00f);
       //    }
       //}

        for (GameObject& gameObject : g_gameObjects) {
            gameObject.UpdateRenderItems();

            // Selected outline?
            if (gameObject.IsSelected()) {
                RenderDataManager::SubmitForOutlineRendering(gameObject.GetRenderItems());
                RenderDataManager::SubmitForOutlineRendering(gameObject.GetRenderItemsHairTopLayer());
                RenderDataManager::SubmitForOutlineRendering(gameObject.GetRenderItemsHairBottomLayer());
                break;
            }
        }
        for (AnimatedGameObject& animatedGameObject : g_animatedGameObjects) {
            animatedGameObject.Update(deltaTime);
        }

        // Clear global render item vectors
        g_renderItems.clear();
        g_skinnedRenderItems.clear();
        g_renderItemsBlended.clear();
        g_renderItemsAlphaDiscarded.clear();
        g_renderItemsHairTopLayer.clear();
        g_renderItemsHairBottomLayer.clear();

        int mousePickIndex = 0;
        for (PickUp& pickUp : g_pickUps) {
            pickUp.SetMousePickIndex(mousePickIndex++);
            pickUp.Update(deltaTime);
            g_renderItems.insert(g_renderItems.end(), pickUp.GetRenderItems().begin(), pickUp.GetRenderItems().end());
        }

        // Doors
        mousePickIndex = 0;
        for (Door& door : g_doors) {
            door.SetMousePickIndex(mousePickIndex++);
            door.Update(deltaTime);
            g_renderItems.insert(g_renderItems.end(), door.GetRenderItems().begin(), door.GetRenderItems().end());
        }

        // Window
        mousePickIndex = 0;
        for (Window& window : g_windows) {
            window.SetMousePickIndex(mousePickIndex++);
            window.Update(deltaTime);
            g_renderItems.insert(g_renderItems.end(), window.GetRenderItems().begin(), window.GetRenderItems().end());
        }

        // Trees
        mousePickIndex = 0;
        for (Tree& tree : g_trees) {
            tree.SetMousePickIndex(mousePickIndex++);
            tree.Update(deltaTime);
            g_renderItems.insert(g_renderItems.end(), tree.GetRenderItems().begin(), tree.GetRenderItems().end());
        
            // Selected outline?
            if (tree.IsSelected()) {
                RenderDataManager::SubmitForOutlineRendering(tree.GetRenderItems());
            }
        
        }

        // Update each GameObject and collect render items
        mousePickIndex = 0;
        for (GameObject& gameObject : g_gameObjects) {
            gameObject.SetMousePickIndex(mousePickIndex++);
            gameObject.UpdateRenderItems();
            // Merge render items into global vectors
            g_renderItems.insert(g_renderItems.end(), gameObject.GetRenderItems().begin(), gameObject.GetRenderItems().end());
            g_renderItemsBlended.insert(g_renderItemsBlended.end(), gameObject.GetRenderItemsBlended().begin(), gameObject.GetRenderItemsBlended().end());
            g_renderItemsAlphaDiscarded.insert(g_renderItemsAlphaDiscarded.end(), gameObject.GetRenderItemsAlphaDiscarded().begin(), gameObject.GetRenderItemsAlphaDiscarded().end());
            g_renderItemsHairTopLayer.insert(g_renderItemsHairTopLayer.end(), gameObject.GetRenderItemsHairTopLayer().begin(), gameObject.GetRenderItemsHairTopLayer().end());
            g_renderItemsHairBottomLayer.insert(g_renderItemsHairBottomLayer.end(), gameObject.GetRenderItemsHairBottomLayer().begin(), gameObject.GetRenderItemsHairBottomLayer().end());
        }

        // Lights
        mousePickIndex = 0;
        for (Light& light : g_lights) {
            light.UpdateRenderItems();
            light.SetMousePickIndex(mousePickIndex++);
            g_renderItems.insert(g_renderItems.end(), light.GetRenderItems().begin(), light.GetRenderItems().end());
        }

        for (BulletCasing& casing : g_bulletCasings) {
            casing.Update(deltaTime);

            RenderItem& renderItem = g_renderItems.emplace_back();
            renderItem.modelMatrix = casing.GetModelMatrix();
            renderItem.inverseModelMatrix = inverse(renderItem.modelMatrix);

            Material* material = AssetManager::GetMaterialByIndex(casing.GetMaterialIndex());
            renderItem.baseColorTextureIndex = material->m_basecolor;
            renderItem.rmaTextureIndex = material->m_rma;
            renderItem.normalMapTextureIndex = material->m_normal;
            renderItem.meshIndex = casing.GetMeshIndex();

            if (Input::KeyPressed(HELL_KEY_BACKSPACE)) {
                Physics::RemoveRigidDynamic(casing.GetrigidDynamicId());
            }
        }


        RenderDataManager::ResetBaseSkinnedVertex();
        for (AnimatedGameObject& animatedGameObject : g_animatedGameObjects) {
            animatedGameObject.UpdateRenderItems();
            animatedGameObject.SubmitForSkinning();
            //animatedGameObject.DrawBones(WHITE);
            //animatedGameObject.DrawBoneTangentVectors();
            g_skinnedRenderItems.insert(g_skinnedRenderItems.end(), animatedGameObject.GetRenderItems().begin(), animatedGameObject.GetRenderItems().end());
        }

        // Animated game objects
        for (int i = 0; i < Game::GetLocalPlayerCount(); i++) {
            Player* player = Game::GetLocalPlayerByIndex(i);
            AnimatedGameObject* viewWeapon = player->GetViewWeaponAnimatedGameObject();
            AnimatedGameObject* characterModel = player->GetCharacterModelAnimatedGameObject();
            viewWeapon->UpdateRenderItems();
            viewWeapon->SubmitForSkinning();
            characterModel->UpdateRenderItems();
            characterModel->SubmitForSkinning();
            g_skinnedRenderItems.insert(g_skinnedRenderItems.end(), viewWeapon->GetRenderItems().begin(), viewWeapon->GetRenderItems().end());
            g_skinnedRenderItems.insert(g_skinnedRenderItems.end(), characterModel->GetRenderItems().begin(), characterModel->GetRenderItems().end());
        }
    }

    void CreateGameObject() {
        g_gameObjects.emplace_back();
    }

    void CreateAnimatedGameObject() {
        g_animatedGameObjects.emplace_back();
    }

    AnimatedGameObject* GetAnimatedGameObjectByIndex(int32_t index) {
        if (index >= 0 && index < g_animatedGameObjects.size()) {
            return &g_animatedGameObjects[index];
        }
        else {
            return nullptr;
        }
    }

    GameObject* GetGameObjectByName(const std::string& name) {
        for (GameObject& gameObject : g_gameObjects) {
            if (gameObject.m_name == name) {
                return &gameObject;
            }
        }
        return nullptr;
    }

    GameObject* GetGameObjectByIndex(int32_t index) {
        if (index >= 0 && index < g_gameObjects.size()) {
            return &g_gameObjects[index];
        }
        else {
            return nullptr;
        }
    }

    Light* GetLightByIndex(int32_t index) {
        if (index >= 0 && index < g_lights.size()) {
            return &g_lights[index];
        }
        else {
            return nullptr;
        }
    }

    PickUp* GetPickUpByIndex(int32_t index) {
        if (index >= 0 && index < g_pickUps.size()) {
            return &g_pickUps[index];
        }
        else {
            return nullptr;
        }
    }
        
    Tree* GetTreeByIndex(int32_t index) {
        if (index >= 0 && index < g_trees.size()) {
            return &g_trees[index];
        }
        else {
            return nullptr;
        }
    }



    void NewCampainWorld() {
        ResetWorld();   
    }

    void LoadDeathMatchMap() {
        ResetWorld();
    }


    void RecreateHieghtMapChunks() {
        g_heightMapChunks.clear();
        g_validChunks.clear();

        // Init heightmap chunks
        int chunkCountX = g_mapWidth * 8;
        int chunkCountZ = g_mapDepth * 8;
        int baseVertex = 0;
        int baseIndex = 0;
        for (int x = 0; x < chunkCountX; x++) {
            for (int z = 0; z < chunkCountZ; z++) {
                int cellX = x / 8;
                int cellZ = z / 8;

                // Skip if map cell out of range somehow
                if (!IsMapCellInRange(cellX, cellZ)) continue;

                // Skip if there is no height map at this cell
                const std::string& heightMapName = GetHeightMapNameAtLocation(cellX, cellZ);
                //std::cout << x << " " << z << " heightMapName: " << heightMapName << "\n";
                if (heightMapName == "") continue;

                HeightMapChunk& chunk = g_heightMapChunks.emplace_back();
                chunk.coord.x = x;
                chunk.coord.z = z;
                chunk.baseVertex = baseVertex;
                chunk.baseIndex = baseIndex;
                baseVertex += VERTICES_PER_CHUNK;
                baseIndex += INDICES_PER_CHUNK;

                g_validChunks[chunk.coord] = g_heightMapChunks.size() - 1;
            }
        }

        Renderer::RecalculateAllHeightMapData();
    }

    void LoadMap(MapCreateInfo* mapCreateInfo) {
        // Handle failed map load
        if (!mapCreateInfo) {
            std::cout << "World::LoadMap() failed: mapCreateInfo was nullptr\n";
            return;
        }

        // It loaded successfully so reset the world...
        ResetWorld();

        g_mapName = mapCreateInfo->name;
        g_mapWidth = mapCreateInfo->width;
        g_mapDepth = mapCreateInfo->depth;

        // and iterate over all the sector locations...
        int mapWidth = World::GetMapWidth();
        int mapDepth = World::GetMapDepth();
        for (int x = 0; x < mapWidth; x++) {
            for (int z = 0; z < mapDepth; z++) {
                std::string sectorName = mapCreateInfo->m_sectorNames[x][z];
                SectorCreateInfo* sectorCreateInfo = SectorManager::GetSectorCreateInfoByName(sectorName);
                if (sectorCreateInfo) {
                    g_heightMapNames[x][z] = sectorCreateInfo->heightMapName;

                    SpawnOffset spawnOffset;
                    spawnOffset.positionX = x * SECTOR_SIZE_WORLD_SPACE;
                    spawnOffset.positionZ = z * SECTOR_SIZE_WORLD_SPACE;
                    AddSectorAtLocation(*sectorCreateInfo, spawnOffset);
                    //std::cout << " - [" << sectorLocation.x << "][" << sectorLocation.z << "] " << sectorName << "\n";
                }
            }
        }

        RecreateHieghtMapChunks();
    }

    void LoadMap(const std::string& mapName) {
        MapCreateInfo* mapCreateInfo = MapManager::GetMapCreateInfoByName(mapName);
        if (mapCreateInfo) {
            LoadMap(mapCreateInfo);
        }
    } 

    void LoadEmptyWorld() {
        ResetWorld();
        g_mapName = "EmptyWorld";
        g_mapWidth = 0;
        g_mapDepth = 0;
        RecreateHieghtMapChunks();
        std::cout << "Loaded empty world\n";
    }
    void LoadSingleSector(SectorCreateInfo* sectorCreateInfo) {
        if (!sectorCreateInfo) return;

        ResetWorld();

        g_mapName = "SingleSector";
        g_sectorNames[0][0] = sectorCreateInfo->sectorName;
        g_heightMapNames[0][0] = sectorCreateInfo->heightMapName;
        g_mapWidth = 1;
        g_mapDepth = 1;

        AddSectorAtLocation(*sectorCreateInfo, SpawnOffset()); 
        RecreateHieghtMapChunks();

        std::cout << "Loaded Single Sector: '" << g_sectorNames[0][0] << "' with height map '" << g_heightMapNames[0][0] << "'\n";
    }

    void AddSectorAtLocation(SectorCreateInfo& sectorCreateInfo, SpawnOffset spawnOffset) {
        for (LightCreateInfo& createInfo : sectorCreateInfo.lights) {
            AddLight(createInfo, spawnOffset);
        }
        for (GameObjectCreateInfo& createInfo : sectorCreateInfo.gameObjects) {
            AddGameObject(createInfo, spawnOffset);
        }
        for (PickUpCreateInfo& createInfo : sectorCreateInfo.pickUps) {
            AddPickUp(createInfo, spawnOffset);
        }
        for (TreeCreateInfo& createInfo : sectorCreateInfo.trees) {
            AddTree(createInfo, spawnOffset);
        }
    }

    void ProcessBullets() {
        for (Bullet& bullet : g_bullets) {

            // Cast PhysX ray
            glm::vec3 rayOrigin = bullet.GetOrigin();
            glm::vec3 rayDirection = bullet.GetDirection();
            float rayLength = 1000.0f;
            PxU32 collisionFlags = RaycastGroup::RAYCAST_ENABLED;
            PhysXRayResult rayResult = Physics::CastPhysXRay(rayOrigin, rayDirection, rayLength, collisionFlags);

            // On hit
            if (rayResult.hitFound) {
                if (rayResult.userData != nullptr) {
                    PhysicsUserData* physicsUserData = (PhysicsUserData*)rayResult.userData;
                    PxRigidDynamic* pxRigidDynamic = (PxRigidDynamic*)rayResult.hitActor;

                    float strength = 200.0f;
                    PxVec3 force = Physics::GlmVec3toPxVec3(bullet.GetDirection()) * strength;

                    // Apply force if object is dynamic
                    if (physicsUserData->physicsType == PhysicsType::RIGID_DYNAMIC) {
                        pxRigidDynamic->setRigidBodyFlag(PxRigidBodyFlag::eKINEMATIC, false);
                        pxRigidDynamic->addForce(force);
                    }
                    

                    // Apply force if object is dynamic
                    if (physicsUserData->objectType == ObjectType::WINDOW) {
                        std::cout << "Shot window " << Util::Vec3ToString(rayResult.hitPosition) << "\n";

                        glm::mat4 parentMatrix = Physics::GetRigidStaticGlobalPose(physicsUserData->physicsId);
                        glm::vec3 localPosition = glm::inverse(parentMatrix) * glm::vec4(rayResult.hitPosition + (rayResult.surfaceNormal * glm::vec3(0.001)), 1.0);
                        glm::vec3 localNormal = glm::inverse(parentMatrix) * glm::vec4(rayResult.surfaceNormal, 0.0);

                        DecalCreateInfo decalCreateInfo;
                        decalCreateInfo.position = rayResult.hitPosition;
                        decalCreateInfo.scale = glm::vec3(0.4);
                        decalCreateInfo.parentPhysicsId = physicsUserData->physicsId;
                        decalCreateInfo.parentPhysicsType = physicsUserData->physicsType;
                        decalCreateInfo.parentObjectId = physicsUserData->objectId;
                        decalCreateInfo.parentObjectType = physicsUserData->objectType;
                        decalCreateInfo.localPosition = localPosition;
                        decalCreateInfo.localNormal = localNormal;
                        decalCreateInfo.decalType = DecalType::GLASS;

                        AddDecal(decalCreateInfo);

                        localNormal = glm::inverse(parentMatrix) * glm::vec4(rayResult.surfaceNormal * glm::vec3(-1) - (rayResult.surfaceNormal * glm::vec3(0.001)), 0.0);
                        AddDecal(decalCreateInfo);

                        Audio::PlayAudio("GlassImpact.wav", 2.0f);
                    }

                    // 

                                        // Front glass bullet decal
                   //PxRigidBody* parent = actor;
                   //glm::mat4 parentMatrix = Util::PxMat44ToGlmMat4(actor->getGlobalPose());
                   //glm::vec3 localPosition = glm::inverse(parentMatrix) * glm::vec4(rayResult.hitPosition + (rayResult.surfaceNormal * glm::vec3(0.001)), 1.0);
                   //glm::vec3 localNormal = glm::inverse(parentMatrix) * glm::vec4(rayResult.surfaceNormal, 0.0);
                   //Scene::CreateBulletDecal(localPosition, localNormal, parent, BulletHoleDecalType::GLASS);
                   //
                   //// Back glass bullet decal
                   //localNormal = glm::inverse(parentMatrix) * glm::vec4(rayResult.surfaceNormal * glm::vec3(-1) - (rayResult.surfaceNormal * glm::vec3(0.001)), 0.0);
                   //Scene::CreateBulletDecal(localPosition, localNormal, parent, BulletHoleDecalType::GLASS);

                    //AddDecal(rayResult.hitPosition);
                }
            }
        }
    }

    PickUp* GetPickUpByObjectId(uint64_t objectID) {
        for (int i = 0; i < g_pickUps.size(); i++) {
            PickUp& pickUp = g_pickUps[i];
            if (pickUp.GetObjectId() == objectID) {
                return &g_pickUps[i];
            }
        }
    }

    Door* GetDoorByObjectId(uint64_t objectID) {
        for (int i = 0; i < g_doors.size(); i++) {
            Door& door = g_doors[i];
            if (door.GetObjectId() == objectID) {
                return &g_doors[i];
            }
        }
    }

    void RemovePickUp(uint64_t objectID) {  
        for (int i = 0; i < g_pickUps.size(); i++) {
            PickUp& pickUp = g_pickUps[i];
            if (pickUp.GetObjectId() == objectID) {
                pickUp.CleanUp();
                g_pickUps.erase(g_pickUps.begin() + i);
                i--;
            }
        }
    }

    void ResetWorld() {
        // Zero out all map names
        for (int x = 0; x < MAX_MAP_WIDTH; x++) {
            for (int z = 0; z < MAX_MAP_DEPTH; z++) {
                g_sectorNames[x][z] = "";
                g_heightMapNames[x][z] = "";
            }
        }
        // Clear heightmap data
        g_heightMapChunks.clear();
        g_validChunks.clear();
        // TODO: probably clear heightmap data

        // Cleanup all objects
        for (Door& door : g_doors) {
            door.CleanUp();
        }
        for (PickUp& pickUp : g_pickUps) {
            pickUp.CleanUp();
        }
        for (Window& window : g_windows) {
            window.CleanUp();
        }

        // Clear all containers
        g_bulletCasings.clear();
        g_gameObjects.clear();
        g_decals.clear();
        g_lights.clear();
        g_trees.clear();
        g_heightMapChunks.clear();
        g_pickUps.clear();
        g_walls.clear();
        g_doors.clear();
        g_floors.clear();

        Door& door = g_doors.emplace_back();
        DoorCreateInfo doorCreateInfo;
        doorCreateInfo.position = glm::vec3(0.05f, 0.0f, 0.0f);
        doorCreateInfo.rotation = glm::vec3(0.0f, 0.0f, 0.0f);
        door.Init(doorCreateInfo);

        Door& door2 = g_doors.emplace_back();
        DoorCreateInfo doorCreateInfo2;
        doorCreateInfo2.position = glm::vec3(2.2f, 0.0f, -2.95f);
        doorCreateInfo2.rotation = glm::vec3(0.0f, HELL_PI * 0.5f, 0.0f);
        door2.Init(doorCreateInfo2);

        Window& window = g_windows.emplace_back();
        WindowCreateInfo windowCreateInfo;
        windowCreateInfo.position = glm::vec3(0.05f, 0.0f, -1.5f);
        windowCreateInfo.rotation = glm::vec3(0.0f, 0.0f, 0.0f);
        window.Init(windowCreateInfo);

        Window& window2 = g_windows.emplace_back();
        WindowCreateInfo windowCreateInfo2;
        windowCreateInfo2.position = glm::vec3(0.05f, 0.0f, 1.5f);
        windowCreateInfo2.rotation = glm::vec3(0.0f, 0.0f, 0.0f);
        window2.Init(windowCreateInfo2);

        Window& window3 = g_windows.emplace_back();
        WindowCreateInfo windowCreateInfo3;
        windowCreateInfo3.position = glm::vec3(2.2f, 0.0f, 2.95f);
        windowCreateInfo3.rotation = glm::vec3(0.0f, HELL_PI * 0.5f, 0.0f);
        window3.Init(windowCreateInfo3);

        // Make all clipping cubes
        g_clippingCubes.clear();
        for (Door& door : g_doors) {
            Transform transform;
            transform.position = door.GetPosition();
            transform.position.y += DOOR_HEIGHT * 0.5f;
            transform.rotation = door.GetRotation();
            transform.scale = glm::vec3(0.2f, DOOR_HEIGHT * 1.01f, DOOR_WIDTH);

            ClippingCube& cube = g_clippingCubes.emplace_back();
            cube.Update(transform);
        }

        for (Window& window : g_windows) {
            Transform transform;
            transform.position = window.GetPosition();
            transform.position.y += 1.5f;
            transform.rotation = window.GetRotation();
            transform.scale = glm::vec3(0.2f, 1.185074f, 0.74f);

            ClippingCube& cube = g_clippingCubes.emplace_back();
            cube.Update(transform);
        }


        // REDESIGN MEEEEEEEEEE
        //Transform transform;
        //
        //ClippingCube& cube = g_clippingCubes.emplace_back();
        //transform.position = glm::vec3(0.0f, 1.5f, 0.0f);
        //transform.rotation = glm::vec3(0.5f, 0.1f, 0.5f);
        //transform.scale = glm::vec3(1.95f, 1.25f, 1.25);
        //cube.Update(transform);
        //
        //ClippingCube& cube2 = g_clippingCubes.emplace_back();
        //transform.position = glm::vec3(0.0f, 1.5f, 2.0f);
        //transform.rotation = glm::vec3(0.0f, 0.0f, 0.0f);
        //transform.scale = glm::vec3(0.75f, 0.75f, 0.75);
        //cube2.Update(transform);
        // REDESIGN MEEEEEEEEEE






















        Wall& wall = g_walls.emplace_back();
        WallCreateInfo wallCreateInfo;
        wallCreateInfo.points.push_back(glm::vec3(0.0f, 0.0f, -3.0f));
        wallCreateInfo.points.push_back(glm::vec3(0.0f, 0.0f, 6.1f));
        wallCreateInfo.points.push_back(glm::vec3(4.5f, 0.0f, 6.1f));
        wallCreateInfo.points.push_back(glm::vec3(4.5f, 0.0f, -3.0f));
        wallCreateInfo.points.push_back(glm::vec3(0.0f, 0.0f, -3.0f));
        wallCreateInfo.materialName = "Ceiling2";
        wallCreateInfo.textureOffsetX = 0.0f;
        wallCreateInfo.textureOffsetY = -1.0f;
        wallCreateInfo.textureScale = 1.0f;
        wallCreateInfo.height = 2.4f;
        wall.Init(wallCreateInfo);

        Wall& wall2 = g_walls.emplace_back();
        WallCreateInfo wallCreateInfo2;
        wallCreateInfo2.points.push_back(glm::vec3(0.1f, 0.0f, -2.9f));
        wallCreateInfo2.points.push_back(glm::vec3(4.4f, 0.0f, -2.9f));
        wallCreateInfo2.points.push_back(glm::vec3(4.4f, 0.0f, 2.9f));
        wallCreateInfo2.points.push_back(glm::vec3(0.1f, 0.0f, 2.9f));
        wallCreateInfo2.points.push_back(glm::vec3(0.1f, 0.0f, -2.9f));
        wallCreateInfo2.materialName = "WallPaper";
        wallCreateInfo2.textureOffsetX = 0.0f;
        wallCreateInfo2.textureOffsetY = -1.4f;
        wallCreateInfo2.textureScale = 1 / 2.4f;
        wallCreateInfo2.height = 2.4f;
        wall2.Init(wallCreateInfo2);


        Wall& wall3 = g_walls.emplace_back();
        WallCreateInfo wallCreateInfo3;
        wallCreateInfo3.points.push_back(glm::vec3(0.1f, 0.0f, 3.0f));
        wallCreateInfo3.points.push_back(glm::vec3(4.4f, 0.0f, 3.0f));
        wallCreateInfo3.points.push_back(glm::vec3(4.4f, 0.0f, 6.0f));
        wallCreateInfo3.points.push_back(glm::vec3(0.1f, 0.0f, 6.0f));
        wallCreateInfo3.points.push_back(glm::vec3(0.1f, 0.0f, 3.0f));
        wallCreateInfo3.materialName = "WallPaper";
        wallCreateInfo3.textureOffsetX = 0.0f;
        wallCreateInfo3.textureOffsetY = -1.4f;
        wallCreateInfo3.textureScale = 1 / 2.4f;
        wallCreateInfo3.height = 2.4f;
        wall3.Init(wallCreateInfo3);



        Floor& floor = g_floors.emplace_back();
        glm::vec3 p0 = glm::vec3(0.0f, 0.0f, -3.0f);
        glm::vec3 p1 = glm::vec3(0.0f, 0.0f, 6.1f);
        glm::vec3 p2 = glm::vec3(4.4f, 0.0f, 6.1f);
        glm::vec3 p3 = glm::vec3(4.4f, 0.0f, -3.0f);
        floor.InitFromPoints(p0, p1, p2, p3);
        floor.SetMaterial("FloorBoards");

        Floor& floor2 = g_floors.emplace_back();
        p0 = glm::vec3(0.0f, 2.4f, -2.9f);
        p1 = glm::vec3(0.0f, 2.4f, 6.1f);
        p2 = glm::vec3(4.4f, 2.4f, 6.1f);
        p3 = glm::vec3(4.4f, 2.4f, -2.9f);
        floor2.InitFromPoints(p3, p2, p1, p0);
        floor2.SetMaterial("Ceiling2");

        LightCreateInfo lightCreateInfo;
        lightCreateInfo.position = glm::vec3(2.2f, 2.2f, 0.0f);
        lightCreateInfo.color = DEFAULT_LIGHT_COLOR;
        lightCreateInfo.type = "HANGING_LIGHT";
        lightCreateInfo.radius = 8.752f;
        lightCreateInfo.strength = 1.5f;

        g_lights.clear();
        AddLight(lightCreateInfo);

        UpdateHouseMeshVertexData();

    }

    void AddBullet(BulletCreateInfo createInfo) {
        g_bullets.push_back(Bullet(createInfo));
    }

    void AddBulletCasing(BulletCasingCreateInfo createInfo, SpawnOffset spawnOffset) {
        createInfo.position.x += spawnOffset.positionX;
        createInfo.position.z += spawnOffset.positionZ;
        g_bulletCasings.push_back(BulletCasing(createInfo));
    }

    void AddDecal(const DecalCreateInfo& createInfo) {
        Decal& decal = g_decals.emplace_back();
        decal.Init(createInfo);
    }

    void AddGameObject(GameObjectCreateInfo createInfo, SpawnOffset spawnOffset) {
        createInfo.position.x += spawnOffset.positionX;
        createInfo.position.z += spawnOffset.positionZ;
        g_gameObjects.push_back(GameObject(createInfo));
    }

    void AddLight(LightCreateInfo createInfo, SpawnOffset spawnOffset) {
        createInfo.position.x += spawnOffset.positionX;
        createInfo.position.z += spawnOffset.positionZ;
        g_lights.push_back(Light(createInfo));
    }

    void AddPickUp(PickUpCreateInfo createInfo, SpawnOffset spawnOffset) {
        createInfo.position.x += spawnOffset.positionX;
        createInfo.position.z += spawnOffset.positionZ;

        PickUp& pickUp = g_pickUps.emplace_back();
        pickUp.Init(createInfo);
    }

    void AddTree(TreeCreateInfo createInfo, SpawnOffset spawnOffset) {
        createInfo.position.x += spawnOffset.positionX;
        createInfo.position.z += spawnOffset.positionZ;
        g_trees.push_back(Tree(createInfo));
    }

    //MapCreateInfo* GetCurrentMapCreateInfo() {
    //    return MapManager::GetMapCreateInfoByName(g_mapName); 
    //}

    std::vector<HeightMapChunk>& GetHeightMapChunks() {
        return g_heightMapChunks;
    }

    const uint32_t GetChunkCountX() {
        return GetMapWidth() * CHUNK_COUNT_PER_MAP_CELL;
    }

    const uint32_t GetChunkCountZ() {
        return GetMapDepth() * CHUNK_COUNT_PER_MAP_CELL;
    }

    const uint32_t GetChunkCount() {
        return g_heightMapChunks.size();
    }

    bool ChunkExists(int x, int z) {
        return g_validChunks.contains(ivecXZ(x, z));
    }

    const HeightMapChunk* GetChunk(int x, int z) {
        if (!ChunkExists(x, z)) return nullptr;

        int index = g_validChunks[ivecXZ(x, z)];
        return &g_heightMapChunks[index];
    }

    const std::string& GetCurrentMapName() {
        return g_mapName;
    }


    const float GetWorldSpaceWidth() {
        return g_mapWidth * MAP_CELL_WORLDSPACE_SIZE;
    }

    const float GetWorldSpaceDepth() {
        return g_mapDepth * MAP_CELL_WORLDSPACE_SIZE;
    }

    const uint32_t GetMapWidth() {
        return g_mapWidth;
    }

    const uint32_t GetMapDepth() {
        return g_mapDepth;
    }

    const std::string& GetSectorNameAtLocation(int x, int z) {
        if (x < 0 || x >= g_mapWidth || z < 0 || z >= g_mapDepth) {
            std::cout << "World::GetSectorNameAtLocation() failed: [" << x << "][" << z << "] out of range of size [" << g_mapWidth << "][" << g_mapDepth << "]\n";
            static const std::string emptyStr = "";
            return emptyStr;
        }
        return g_sectorNames[x][z];
    }

    const std::string& GetHeightMapNameAtLocation(int x, int z) {
        if (x < 0 || x >= g_mapWidth || z < 0 || z >= g_mapDepth) {
            std::cout << "World::GetHeightMapNameAtLocation() failed: [" << x << "][" << z << "] out of range of size [" << g_mapWidth << "][" << g_mapDepth << "]\n";
            static const std::string emptyStr = "";
            return emptyStr;
        }
        return g_heightMapNames[x][z];
    }

    bool IsMapCellInRange(int x, int z) {
        return (x >= 0 && x < g_mapWidth && z >= 0 && z < g_mapDepth);
    }

    const uint32_t GetHeightMapCount() {
        int count = 0;
        for (int x = 0; x < g_mapWidth; x++) {
            for (int z = 0; z < g_mapDepth; z++) {
                if (g_heightMapNames[x][z] != "") {
                    count++;
                }
            }
        }
        return count;
    }

    OpenGLDetachedMesh& GetHouseMesh() {
        return g_houseMesh;
    }

    std::vector<Decal>& GetDecals()                             { return g_decals; }
    std::vector<Door>& GetDoors()                               { return g_doors; }
    std::vector<Floor>& GetFloors()                             { return g_floors; }
    std::vector<Wall>& GetWalls()                               { return g_walls; }
    std::vector<Window>& GetWindows()                           { return g_windows; }
    std::vector<ClippingCube>& GetClippingCubes()               { return g_clippingCubes; }
    std::vector<AnimatedGameObject>& GetAnimatedGameObjects()   { return g_animatedGameObjects; }
    std::vector<BulletCasing>& GetBulletCasings()               { return g_bulletCasings; };
    std::vector<Bullet>& GetBullets()                           { return g_bullets; };
    std::vector<GameObject>& GetGameObjects()                   { return g_gameObjects; }
    std::vector<Light>& GetLights()                             { return g_lights; };
    std::vector<PickUp>& GetPickUps()                           { return g_pickUps; };
    std::vector<Tree>& GetTrees()                               { return g_trees; };
    std::vector<RenderItem>& GetRenderItems()                   { return g_renderItems; }
    std::vector<RenderItem>& GetRenderItemsBlended()            { return g_renderItemsBlended; }
    std::vector<RenderItem>& GetRenderItemsAlphaDiscarded()     { return g_renderItemsAlphaDiscarded; }
    std::vector<RenderItem>& GetRenderItemsHairTopLayer()       { return g_renderItemsHairTopLayer; }
    std::vector<RenderItem>& GetRenderItemsHairBottomLayer()    { return g_renderItemsHairBottomLayer; }
    std::vector<RenderItem>& GetSkinnedRenderItems()            { return g_skinnedRenderItems; }

    void PrintMapCreateInfoDebugInfo() {
        std::cout << "Map: '" << g_mapName << "'\n";
        // Iterate the map, save any sectors height map that isn't none
        for (int x = 0; x < g_mapWidth; x++) {
            for (int z = 0; z < g_mapDepth; z++) {
                std::string sectorName = GetSectorNameAtLocation(x, z);
                std::string heightMapName = GetHeightMapNameAtLocation(x, z);
                std::cout << " - [" << x << "][" << z << "] Sector: '" << sectorName << "'";
                std::cout << " HeightMap: '" << heightMapName << "'";
                std::cout << "\n";
            }
        }
    }
}