#include "World.h"
#include "Audio/Audio.h"
#include "Core/Game.h"
#include "Input/Input.h"
#include "Renderer/RenderDataManager.h"
#include "Renderer/Renderer.h"
#include "Viewport/ViewportManager.h"

namespace World {

    void LazyDebugSpawns();
    void CalculateGPULights();

    // REMOVE ME!
    uint64_t g_rooAnimatedGameObject = 0;
    AnimatedGameObject* GetRooTest() {
        return GetAnimatedGameObjectByObjectId(g_rooAnimatedGameObject);
    }
    // 

    void Update(float deltaTime) {

        if (g_rooAnimatedGameObject == 0) {
            g_rooAnimatedGameObject = CreateAnimatedGameObject();
            AnimatedGameObject* roo = GetAnimatedGameObjectByObjectId(g_rooAnimatedGameObject);
            roo->SetSkinnedModel("Kangaroo");
            roo->SetRotationY(HELL_PI);
            roo->SetAnimationModeToBindPose();
            roo->SetName("Roo");
            roo->SetAllMeshMaterials("CheckerBoard");
            roo->SetAllMeshMaterials("Leopard");
            roo->SetScale(0.9f);
            roo->PrintMeshNames();
            roo->SetRagdoll("Kangaroo", 1500.0f);

            Ragdoll* ragdoll = Physics::GetRagdollById(roo->GetRagdollId());
            if (ragdoll) {
                ragdoll->SetPhysicsData(roo->GetRagdollId(), ObjectType::RAGDOLL_ENEMY);
            }

            AnimationPlaybackParams params;
            params.animationSpeed = 1.0f;
            roo->PlayAndLoopAnimation("Kangaroo_Hop2", params);
        }
        if (Input::KeyPressed(HELL_KEY_SLASH)) {
            AnimatedGameObject* roo = GetAnimatedGameObjectByObjectId(g_rooAnimatedGameObject);
            roo->SetAnimationModeToRagdoll();
        }
        if (Input::KeyPressed(HELL_KEY_COMMA)) {
            AnimatedGameObject* roo = GetAnimatedGameObjectByObjectId(g_rooAnimatedGameObject);
            AnimationPlaybackParams params;
            params.animationSpeed = 1.00f;
            roo->PlayAndLoopAnimation("Kangaroo_Idle", params);
        }
        if (Input::KeyPressed(HELL_KEY_PERIOD)) {
            AnimatedGameObject* roo = GetAnimatedGameObjectByObjectId(g_rooAnimatedGameObject);
            AnimationPlaybackParams params;
            params.animationSpeed = 1.0f;
            roo->PlayAndLoopAnimation("Kangaroo_Hop2", params);
        }
        // REMOVE ME

        ProcessBullets();
        LazyDebugSpawns();

        std::vector<AnimatedGameObject>& animatedGameObjects = GetAnimatedGameObjects();
        std::vector<BulletCasing>& bulletCasings = GetBulletCasings();
        std::vector<Bullet>& bullets = GetBullets();
        std::vector<Door>& doors = GetDoors();
        std::vector<GameObject>& gameObjects = GetGameObjects();
        std::vector<Light>& lights = GetLights();
        std::vector<Piano>& pianos = GetPianos();
        std::vector<PickUp>& pickUps = GetPickUps();
        std::vector<Tree>& trees = GetTrees();
        std::vector<Wall>& walls = GetWalls();
        std::vector<Window>& windows = GetWindows();

        for (AnimatedGameObject& animatedGameObject : animatedGameObjects) {
            animatedGameObject.Update(deltaTime);
        }

        for (BulletCasing& bulletCasing : bulletCasings) {
            bulletCasing.Update(deltaTime);
        }

        for (Door& door : doors) {
            door.Update(deltaTime);
        }

        for (GameObject& gameObject : gameObjects) {
            gameObject.Update(deltaTime);
        }

        for (Mermaid& mermaid : GetMermaids()) {
            mermaid.Update(deltaTime);
        }

        for (PickUp& pickUp : pickUps) {
            pickUp.Update(deltaTime);
        }

        for (Piano& piano : pianos) {
            piano.Update(deltaTime);
        }

        for (Tree& tree : trees) {
            tree.Update(deltaTime);
        }

        for (Shark& shark : GetSharks()) {
            shark.Update(deltaTime);
        }

        for (Window& window : windows) {
            window.Update(deltaTime);
        }

        for (Wall& wall : walls) {
            // Nothing as of yet. Probably ever.
        }

        // Update lights last. That way their dirty state reflects the state of all 
        // objects whom may have potentially moved within their radius
        for (Light& light : lights) {
            light.Update(deltaTime);
        }

        CalculateGPULights();

        AnimatedGameObject* roo = GetAnimatedGameObjectByObjectId(g_rooAnimatedGameObject);
        if (roo) {
            roo->SetPosition(glm::vec3(31, 30.4f, 36));
            roo->SetPosition(glm::vec3(49.5, 30.4f, 39));
            roo->SetRotationY(HELL_PI * -0.5);
            //roo->SetPosition(glm::vec3(29, 30.4f, 39));
            //roo->SetRotationY(HELL_PI * -0.85);
            roo->SetName("Roo");
            roo->SetAllMeshMaterials("Kangaroo");
            roo->SetMeshMaterialByMeshName("LeftEye_Iris", "KangarooIris");
            roo->SetMeshMaterialByMeshName("RightEye_Iris", "KangarooIris");
            roo->DisableDrawingForMeshByMeshName("LeftEye_Sclera");
            roo->DisableDrawingForMeshByMeshName("RightEye_Sclera");
            roo->SetScale(1.0f);
            roo->SetScale(1.0f);
        }

        // Volumetric blood
        std::vector<VolumetricBloodSplatter>& volumetricBloodSplatters = GetVolumetricBloodSplatters();
        for (int i = 0; i < volumetricBloodSplatters.size(); i++) {
            VolumetricBloodSplatter& volumetricBloodSplatter = volumetricBloodSplatters[i];

            if (volumetricBloodSplatter.GetLifeTime() < 0.9f) {
                volumetricBloodSplatter.Update(deltaTime);
            }
            else {
                volumetricBloodSplatters.erase(volumetricBloodSplatters.begin() + i);
                i--;
            }
        }
    }

    void LazyDebugSpawns() {
        // AKs
        if (Input::KeyPressed(HELL_KEY_BACKSPACE)) {
            PickUpCreateInfo createInfo;
            createInfo.position = Game::GetLocalPlayerByIndex(0)->GetCameraPosition();
            createInfo.position += Game::GetLocalPlayerByIndex(0)->GetCameraForward();
            createInfo.rotation.x = Util::RandomFloat(-HELL_PI, HELL_PI);
            createInfo.rotation.y = Util::RandomFloat(-HELL_PI, HELL_PI);
            createInfo.rotation.z = Util::RandomFloat(-HELL_PI, HELL_PI);
            createInfo.pickUpType = Util::PickUpTypeToString(PickUpType::AKS74U);
            AddPickUp(createInfo);
        }

        // Remingtons
        if (Input::KeyPressed(HELL_KEY_INSERT)) {
            PickUpCreateInfo createInfo;
            createInfo.position = Game::GetLocalPlayerByIndex(0)->GetCameraPosition();
            createInfo.position += Game::GetLocalPlayerByIndex(0)->GetCameraForward();
            createInfo.rotation.x = Util::RandomFloat(-HELL_PI, HELL_PI);
            createInfo.rotation.y = Util::RandomFloat(-HELL_PI, HELL_PI);
            createInfo.rotation.z = Util::RandomFloat(-HELL_PI, HELL_PI);
            createInfo.pickUpType = Util::PickUpTypeToString(PickUpType::REMINGTON_870);
            AddPickUp(createInfo);
        }
    }

    void UpdateDoorAndWindowCubeTransforms() {
        std::vector<Transform>& transforms = GetDoorAndWindowCubeTransforms();
        std::vector<Door>& doors = GetDoors();
        std::vector<Window>& windows = GetWindows();

        transforms.clear();
        transforms.reserve(doors.size() + windows.size());

        for (Door& door : doors) {
            Transform& transform = transforms.emplace_back();
            transform.position = door.GetPosition();
            transform.position.y += DOOR_HEIGHT / 2;
            transform.rotation.y = door.GetRotation().y;
            transform.scale.x = 0.2f;
            transform.scale.y = DOOR_HEIGHT * 1.0f;
            transform.scale.z = 1.02f;
        }

        for (Window& window : windows) {
            float windowMidPointFromGround = 1.5f;

            Transform& transform = transforms.emplace_back();
            transform.position = window.GetPosition();
            transform.position.y += windowMidPointFromGround;
            transform.rotation.y = window.GetRotation().y;
            transform.scale.x = 0.2f;
            transform.scale.y = 1.12f;
            transform.scale.z = 0.946f;
        }
    }

    void CalculateGPULights() {
        for (int i = 0; i < GetLights().size(); i++) {
            RenderDataManager::SubmitGPULightHighRes(i);
        }
    }
}