#include "World.h"
#include "Core/Game.h"
#include "Input/Input.h"
#include "Viewport/ViewportManager.h"
#include "Renderer/RenderDataManager.h"

namespace World {
    void EvaluatePianoKeyBulletHit(Bullet& bullet);

    void ProcessBullets() {
        std::vector<Bullet>& bullets = GetBullets();
        std::vector<Bullet> newBullets;
        bool glassWasHit = false;
        bool rooDeath = false;

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

                // Shot a player ragdoll?
                if (objectType == ObjectType::RAGDOLL_PLAYER) {
                    Player* player = Game::GetPlayerByPlayerId(objectId);
                    if (player) {
                        player->Kill();
                    }
                }

                if (objectType == ObjectType::SHARK) {
                    Shark* shark = World::GetSharkByObjectId(objectId);
                    if (shark) {
                        shark->GiveDamage(bullet.GetOwnerObjectId(), bullet.GetDamage());
                    }
                }

                // Shot enemy ragdoll?
                if (objectType == ObjectType::RAGDOLL_ENEMY && !rooDeath) {

                    // Find screenspace blood decal spawn position
                    glm::vec3 rayOrigin = rayResult.hitPosition;
                    glm::vec3 rayDirection = glm::vec3(0.0f, -1.0f, 0.0f);
                    float rayLength = 100;
                    PhysXRayResult downwardRayResult = Physics::CastPhysXRayStaticEnviroment(rayOrigin, rayDirection, rayLength);
                    
                    if (downwardRayResult.hitFound) {
                        ScreenSpaceBloodDecalCreateInfo decalCreateInfo;
                        decalCreateInfo.position = downwardRayResult.hitPosition;
                        World::AddScreenSpaceBloodDecal(decalCreateInfo);
                    }

                    // Give damage to enemy
                    for (AnimatedGameObject& animatedGameObject : GetAnimatedGameObjects()) {
                        if (animatedGameObject.GetRagdollId() == objectId) {

                            DecalPaintingInfo decalPaintingInfo;
                            decalPaintingInfo.rayOrigin = bullet.GetOrigin();
                            decalPaintingInfo.rayDirection = bullet.GetDirection();
                            RenderDataManager::SubmitDecalPaintingInfo(decalPaintingInfo);
                            
                            for (Kangaroo& kangaroo : GetKangaroos()) {
                                if (kangaroo.GetAnimatedGameObject() == &animatedGameObject) {
                                    kangaroo.GiveDamage(bullet.GetDamage());
                                }
                            }
                        }
                    }

                    // Spawn volumetric blood
                    glm::vec3 position = rayResult.hitPosition;
                    glm::vec3 front = bullet.GetDirection() * glm::vec3(-1);
                    World::AddVolumetricBlood(position, -bullet.GetDirection());
                }


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
                        bulletCreateInfo.ownerObjectId = bullet.GetOwnerObjectId();
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