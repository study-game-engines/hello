#include "World.h"
#include "Audio/Audio.h"
#include "Core/Game.h"
#include "Input/Input.h"
#include "Renderer/RenderDataManager.h"

namespace World {

    void LazyDebugSpawns();
    void ProcessBullets();
    void UpdateDoorAndWindowCubeTransforms();

    void Update(float deltaTime) {
        ProcessBullets();
        LazyDebugSpawns();
        UpdateDoorAndWindowCubeTransforms();


        //static uint32_t audioId_a = 0;
        //static uint32_t audioId_b = 0;
        //static uint32_t audioId_c = 0;
        //static uint32_t audioId_d = 0;
        //static uint32_t audioId_e = 0;
        //
        //if (Input::KeyPressed(HELL_KEY_G)) {
        //    Audio::StopAudio(audioId_a);
        //    audioId_a = Audio::PlayAudio("piano/a3.wav", 1.0f);
        //}
        //if (!Input::KeyDown(HELL_KEY_G)) {
        //    Audio::FadeOut(audioId_a, 0.75f);
        //}
        //
        //if (Input::KeyPressed(HELL_KEY_H)) {
        //    Audio::StopAudio(audioId_b);
        //    audioId_b = Audio::PlayAudio("piano/b3.wav", 1.0f);
        //}
        //if (!Input::KeyDown(HELL_KEY_H)) {
        //    Audio::FadeOut(audioId_b, 0.75f);
        //}
        //
        //if (Input::KeyPressed(HELL_KEY_J)) {
        //    Audio::StopAudio(audioId_c);
        //    audioId_c = Audio::PlayAudio("piano/c3_sharp.wav", 1.0f);
        //}
        //if (!Input::KeyDown(HELL_KEY_J)) {
        //    Audio::FadeOut(audioId_c, 0.75f);
        //}
        //
        //if (Input::KeyPressed(HELL_KEY_K)) {
        //    Audio::StopAudio(audioId_d);
        //    audioId_d = Audio::PlayAudio("piano/d3.wav", 1.0f);
        //}
        //if (!Input::KeyDown(HELL_KEY_K)) {
        //    Audio::FadeOut(audioId_d, 0.75f);
        //}
        //
        //if (Input::KeyPressed(HELL_KEY_L)) {
        //    Audio::StopAudio(audioId_e);
        //    audioId_e = Audio::PlayAudio("piano/e3.wav", 1.0f);
        //}
        //if (!Input::KeyDown(HELL_KEY_L)) {
        //    Audio::FadeOut(audioId_e, 0.75f);
        //}


       //bool isPlaying = false;
       //if (channel.fmodChannel) {
       //    channel.fmodChannel->isPlaying(&isPlaying);
       //}
       //
       //if (channel.fmodChannel && isPlaying) {
       //    float blendFactor = 1.0f - powf(0.01f, deltaTime / releaseDuration); // Use powf for floats
       //    channel.currentVolume = std::lerp(channel.currentVolume, 0.0f, blendFactor);
       //
       //    // Apply the volume
       //    FMOD_ERRCHECK(channel.fmodChannel->setVolume(channel.currentVolume));
       //
       //    // Stop when volume is negligible
       //    if (channel.currentVolume < 0.01f) {
       //        FMOD_ERRCHECK(channel.fmodChannel->stop());
       //        channel.fmodChannel = nullptr; // Mark as stopped
       //        // Remove from your 'releasingNotes' list here
       //    }
       //}
       //else {
       //  // Channel already stopped or became invalid, remove from 'releasingNotes'
       //    channel.fmodChannel = nullptr;
       //    // Remove from your 'releasingNotes' list here
       //}

        std::vector<AnimatedGameObject>& animatedGameObjects = GetAnimatedGameObjects();
        std::vector<BulletCasing>& bulletCasings = GetBulletCasings();
        std::vector<Bullet>& bullets = GetBullets();
        std::vector<Door>& doors = GetDoors();
        std::vector<GameObject>& gameObjects = GetGameObjects();
        std::vector<Light>& lights = GetLights();
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

        for (Tree& tree : trees) {
            tree.Update(deltaTime);
        }

        for (Window& window : windows) {
            window.Update(deltaTime);
        }

        for (Wall& wall : walls) {
            // Nothing as of yet. Probably ever.
        }

        UpdateSceneBvh();
        TestBvh();
    }

    void ProcessBullets() {
        std::vector<Bullet>& bullets = GetBullets();
        std::vector<Bullet> newBullets;
        bool glassWasHit = false;

        for (Bullet& bullet : bullets) {

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
                        objectType == ObjectType::HOUSE_PLANE ||
                        objectType == ObjectType::DOOR) {
                        decalCreateInfo.decalType = DecalType::PLASTER;
                        AddDecal(decalCreateInfo);
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
            transform.scale.y = DOOR_HEIGHT * 1.1f;
            transform.scale.z = 1.02f;
        }
    }
}