#include "World.h"
#include "Core/Audio.h"
#include "Core/Game.h"
#include "Input/Input.h"
#include "Renderer/RenderDataManager.h"

namespace World {

    void LazyDebugSpawns();
    void ProcessBullets();

    void Update(float deltaTime) {
        ProcessBullets();
        LazyDebugSpawns();

        std::vector<AnimatedGameObject>& animatedGameObjects = GetAnimatedGameObjects();
        std::vector<BulletCasing>& bulletCasings = GetBulletCasings();
        std::vector<Bullet>& bullets = GetBullets();
        std::vector<Door>& doors = GetDoors();
        std::vector<GameObject>& gameObjects = GetGameObjects();
        std::vector<Light>& lights = GetLights();
        std::vector<PickUp>& pickUps = GetPickUps();
        std::vector<Tree>& trees = GetTrees();
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
            if (rayResult.hitFound && rayResult.userData != nullptr) {
                PhysicsUserData* physicsUserData = (PhysicsUserData*)rayResult.userData;

                // Apply force if object is dynamic
                float strength = 200.0f;
                PxVec3 force = Physics::GlmVec3toPxVec3(bullet.GetDirection()) * strength;
                PxRigidDynamic* pxRigidDynamic = (PxRigidDynamic*)rayResult.hitActor;
                if (physicsUserData->physicsType == PhysicsType::RIGID_DYNAMIC) {
                    pxRigidDynamic->setRigidBodyFlag(PxRigidBodyFlag::eKINEMATIC, false);
                    pxRigidDynamic->addForce(force);
                }

                DecalCreateInfo decalCreateInfo;
                decalCreateInfo.parentPhysicsId = physicsUserData->physicsId;
                decalCreateInfo.parentPhysicsType = physicsUserData->physicsType;
                decalCreateInfo.parentObjectId = physicsUserData->objectId;
                decalCreateInfo.parentObjectType = physicsUserData->objectType;
                decalCreateInfo.surfaceHitPosition = rayResult.hitPosition;
                decalCreateInfo.surfaceHitNormal = rayResult.surfaceNormal;

                // Plaster decal
                if (physicsUserData->objectType == ObjectType::WALL_SEGMENT ||
                    physicsUserData->objectType == ObjectType::HOUSE_PLANE ||
                    physicsUserData->objectType == ObjectType::DOOR) {
                    decalCreateInfo.decalType = DecalType::PLASTER;
                    AddDecal(decalCreateInfo);
                }

                // Glass decal
                if (physicsUserData->objectType == ObjectType::WINDOW) {
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
}