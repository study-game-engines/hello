#include "World.h"
#include "Core/Game.h"
#include "Input/Input.h"
#include "Viewport/ViewportManager.h"

namespace World {
    void EvaluatePianoKeyBulletHit(Bullet& bullet);

    void ProcessBullets() {
        std::vector<Bullet>& bullets = GetBullets();
        std::vector<Bullet> newBullets;
        bool glassWasHit = false;
        bool rooDeath = false;

        // Debug shit to find out why some bullets miss
        //static int mode = 0;
        //if (Input::KeyPressed(HELL_KEY_ENTER)) {
        //    mode++;
        //    if (mode == 3) {
        //        mode = 0;
        //    }
        //    std::cout << "debug mode: " << mode << "\n";
        //}
        //if (Input::RightMousePressed()) {
        //    Player* player = Game::GetLocalPlayerByIndex(0);
        //
        //    auto actors = Physics::GetRagdollPxRigidActors(player->GetRadollId());
        //    for (auto* actor : actors) {
        //        PxTransform t = actor->getGlobalPose();
        //        PxMat44 m = PxMat44(t);
        //        glm::mat4 mm = Physics::PxMat44ToGlmMat4(m);
        //        std::cout << Util::Mat4ToString(mm) << "\n\n";
        //    }
        //
        //    auto rayOrigin = player->GetCameraPosition();
        //    auto rayDirection = player->GetCameraForward();
        //    std::vector<PxRigidActor*> ignoredActors;
        //    PhysXRayResult rayResult;
        //    
        //    if (mode == 0) {
        //        ignoredActors = Physics::GetRagdollPxRigidActors(player->GetRagdollId());
        //        rayResult = Physics::CastPhysXRay(rayOrigin, rayDirection, 1000, false, RaycastIgnoreFlags::PLAYER_CHARACTER_CONTROLLERS, ignoredActors);
        //    }
        //    if (mode == 1) {
        //        ignoredActors = Physics::GetRagdollPxRigidActors(player->GetRagdollId());
        //        rayResult = Physics::CastPhysXRay(rayOrigin, rayDirection, 1000, false, RaycastIgnoreFlags::PLAYER_CHARACTER_CONTROLLERS);
        //    }
        //    if (mode == 2) {
        //        rayOrigin += glm::vec3(0, 1, 0);
        //        rayResult = Physics::CastPhysXRay(rayOrigin, rayDirection, 1000, false, RaycastIgnoreFlags::NONE);
        //    }
        //
        //    if (rayResult.hitFound) {
        //        PhysicsType& physicsType = rayResult.userData.physicsType;
        //        ObjectType& objectType = rayResult.userData.objectType;
        //        uint64_t physicsId = rayResult.userData.physicsId;
        //        uint64_t objectId = rayResult.userData.objectId;
        //        std::cout << "Debug ray hit: " << Util::PhysicsTypeToString(physicsType) << " / " << Util::ObjectTypeToString(objectType) << "\n";
        //    }
        //}

        for (Bullet& bullet : bullets) {

            // Did it hit a piano key?
            EvaluatePianoKeyBulletHit(bullet);

            // Cast PhysX ray
            glm::vec3 rayOrigin = bullet.GetOrigin();
            glm::vec3 rayDirection = bullet.GetDirection();
            float rayLength = 1000.0f;

            std::vector<PxRigidActor*> ignoredActors;

            Player* player = Game::GetPlayerByPlayerId(bullet.GetOwnerObjectId());
            if (player) {
                auto ragdollActors = Physics::GetRagdollPxRigidActors(player->GetRagdollId());
                ignoredActors.insert(ignoredActors.end(), ragdollActors.begin(), ragdollActors.end());
            }

            PhysXRayResult rayResult = Physics::CastPhysXRay(rayOrigin, rayDirection, rayLength, false, RaycastIgnoreFlags::PLAYER_CHARACTER_CONTROLLERS, ignoredActors);

            // On hit
            if (rayResult.hitFound) {
                PhysicsType& physicsType = rayResult.userData.physicsType;
                ObjectType& objectType = rayResult.userData.objectType;
                uint64_t physicsId = rayResult.userData.physicsId;
                uint64_t objectId = rayResult.userData.objectId;

                //std::cout << "Bullet hit: " << Util::PhysicsTypeToString(physicsType) << " / " << Util::ObjectTypeToString(objectType) << "\n";

                // Shot a player ragdoll?
                if (objectType == ObjectType::RAGDOLL_PLAYER) {
                    Player* player = Game::GetPlayerByPlayerId(objectId);
                    if (player) {
                        player->Kill();
                    }
                }

                // Shot enemy ragdoll?
                if (objectType == ObjectType::RAGDOLL_ENEMY && !rooDeath) {
                    //std::cout << "Bullet hit RAGDOLL_ENEMY\n";
                    for (AnimatedGameObject& animatedGameObject : GetAnimatedGameObjects()) {
                        if (animatedGameObject.GetRagdollId() == objectId) {
                            //std::cout << "- enemy ragdoll found\n";
                            animatedGameObject.SetAnimationModeToRagdoll();
                            Audio::PlayAudio("Kangaroo_Death.wav", 1.0f);
                            rooDeath = true;
                        }
                    }
                }

                // Spawn volumetric blood
                glm::vec3 position = rayResult.hitPosition;
                glm::vec3 front = bullet.GetDirection() * glm::vec3(-1);
                World::AddVolumetricBlood(position, -bullet.GetDirection());
                //fleshWasHit = true;

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
                    decalCreateInfo.surfaceHitNormal = rayResult.hitNormal;

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

}