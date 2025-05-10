#include "World.h"
#include "Audio/Audio.h"
#include "Core/Game.h"
#include "Input/Input.h"
#include "Renderer/RenderDataManager.h"
#include "Viewport/ViewportManager.h"

namespace World {

    void EvaluatePianoKeyBulletHit(Bullet& bullet);
    void LazyDebugSpawns();
    void ProcessBullets();
    void CalculateGPULights();

    void Update(float deltaTime) {
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

        for (Light& light : lights) {
            light.Update(deltaTime);
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

        CalculateGPULights();
    }

    void ProcessBullets() {
        std::vector<Bullet>& bullets = GetBullets();
        std::vector<Bullet> newBullets;
        bool glassWasHit = false;

        for (Bullet& bullet : bullets) {

            // Did it hit a piano key?
            EvaluatePianoKeyBulletHit(bullet);

            // Cast PhysX ray
            glm::vec3 rayOrigin = bullet.GetOrigin();
            glm::vec3 rayDirection = bullet.GetDirection();
            float rayLength = 1000.0f;
            PxU32 collisionFlags = RaycastGroup::RAYCAST_ENABLED;
            PhysXRayResult rayResult = Physics::CastPhysXRay(rayOrigin, rayDirection, rayLength, collisionFlags);

            // On hit
            if (rayResult.hitFound) {
                PhysicsType& physicsType = rayResult.userData.physicsType;
                ObjectType& objectType = rayResult.userData.objectType;
                uint64_t physicsId = rayResult.userData.physicsId;
                uint64_t objectId = rayResult.userData.objectId;

                // Apply force if object is dynamic
                if (physicsType == PhysicsType::RIGID_DYNAMIC) {
                    float strength = 200.0f;
                    glm::vec3 force = bullet.GetDirection() * strength;
                    Physics::AddFoceToRigidDynamic(physicsId, force);
                }

                // Add decals for rigid static
                if (physicsType == PhysicsType::RIGID_STATIC) {
                    DecalCreateInfo decalCreateInfo;
                    decalCreateInfo.parentPhysicsId = physicsId;
                    decalCreateInfo.parentPhysicsType = physicsType;
                    decalCreateInfo.parentObjectId = objectId;
                    decalCreateInfo.parentObjectType = objectType;
                    decalCreateInfo.surfaceHitPosition = rayResult.hitPosition;
                    decalCreateInfo.surfaceHitNormal = rayResult.surfaceNormal;

                    // Plaster decal
                    if (objectType == ObjectType::WALL_SEGMENT ||
                        objectType == ObjectType::PLANE ||
                        objectType == ObjectType::DOOR ||
                        objectType == ObjectType::PIANO) {
                        decalCreateInfo.decalType = DecalType::PLASTER;
                        AddDecal(decalCreateInfo);
                    }

                    // Piano note trigger
                    if (objectType == ObjectType::PIANO) {
                        Piano* piano = World::GetPianoByObjectId(objectId);
                        if (piano) {
                            piano->TriggerInternalNoteFromExternalBulletHit(rayResult.hitPosition);
                        }
                    }

                    // Glass decal
                    if (objectType == ObjectType::WINDOW) {
                        decalCreateInfo.decalType = DecalType::GLASS;
                        AddDecal(decalCreateInfo);

                        decalCreateInfo.surfaceHitNormal *= glm::vec3(-1.0f);
                        AddDecal(decalCreateInfo);

                        // Create new bullet
                        BulletCreateInfo bulletCreateInfo;
                        bulletCreateInfo.origin = rayResult.hitPosition + bullet.GetDirection() * glm::vec3(0.05f);
                        bulletCreateInfo.direction = bullet.GetDirection();
                        bulletCreateInfo.damage = bullet.GetDamage();
                        bulletCreateInfo.weaponIndex = bullet.GetWeaponIndex();
                        newBullets.emplace_back(Bullet(bulletCreateInfo));

                        glassWasHit = true;
                    }
                }
            }
        }

        if (glassWasHit) {
            Audio::PlayAudio("GlassImpact.wav", 2.0f);
        }

        // Wipe old bullets, and replace with any new ones that got spawned from glass hits
        bullets = newBullets;;
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

    void EvaluatePianoKeyBulletHit(Bullet& bullet) {

        for (int i = 0; i < 4; i++) {
            Viewport* viewport = ViewportManager::GetViewportByIndex(i);
            if (!viewport->IsVisible()) continue;

            glm::vec3 rayOrigin = bullet.GetOrigin();
            glm::vec3 rayDir = bullet.GetDirection();
            float maxRayDistance = 100.0f;

            BvhRayResult result = ClosestHit(rayOrigin, rayDir, maxRayDistance, i);
            if (result.hitFound) {
                if (result.objectType == ObjectType::PIANO_KEY) {
                    for (Piano& piano : World::GetPianos()) {
                        if (piano.PianoKeyExists(result.objectId)) {
                            PianoKey* pianoKey = piano.GetPianoKey(result.objectId);
                            if (pianoKey) {
                                pianoKey->PressKey();
                            }
                        }
                    }

                }
            }
        }
    }

    void CalculateGPULights() {
        for (int i = 0; i < GetLights().size(); i++) {
            RenderDataManager::SubmitGPULightHighRes(i);
        }
    }
}