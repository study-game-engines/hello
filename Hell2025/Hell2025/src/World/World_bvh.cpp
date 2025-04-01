#include "World.h"
#include "AssetManagement/AssetManager.h"
#include "Core/Game.h"
#include "Math/Bvh.h";
#include "Viewport/ViewportManager.h";

namespace World {

    std::vector<PrimitiveInstance> g_instances;
    uint64_t g_sceneBvhId;

    //ObjectInstanceData CreateObjectInstanceDataFromRenderItem(const RenderItem& renderItem, uint64_t objectId) {
    //    ObjectInstanceData instance;
    //    instance.objectId = objectId;
    //    instance.objectType = (ObjectType)renderItem.objectType;
    //    instance.worldTransform = renderItem.modelMatrix;
    //    instance.worldAabbBoundsMin = renderItem.aabbMin;
    //    instance.worldAabbBoundsMax = renderItem.aabbMax;
    //    instance.worldAabbCenter = (renderItem.aabbMin + renderItem.aabbMax) * 0.5f;
    //    instance.triangleMeshBvhId = AssetManager::GetMeshByIndex(renderItem.meshIndex)->triangleMeshBvhId;
    //    return instance;
    //}

    void CreateObjectInstanceDataFromRenderItem(const RenderItem& renderItem, uint64_t objectId, Frustum& frustum, std::vector<PrimitiveInstance>& container) {
        if (frustum.IntersectsAABB(renderItem)) {
            PrimitiveInstance& instance = container.emplace_back();
            instance.objectId = objectId;
            instance.objectType = (ObjectType)renderItem.objectType;
            instance.worldTransform = renderItem.modelMatrix;
            instance.worldAabbBoundsMin = renderItem.aabbMin;
            instance.worldAabbBoundsMax = renderItem.aabbMax;
            instance.worldAabbCenter = (renderItem.aabbMin + renderItem.aabbMax) * 0.5f;
            instance.meshBvhId = AssetManager::GetMeshByIndex(renderItem.meshIndex)->triangleMeshBvhId;
        }
    }

    void UpdateSceneBvh() {

        g_instances.clear();

        Viewport* viewport = ViewportManager::GetViewportByIndex(0);
        //Frustum& frustum = viewport->GetFrustum();

        Player* player = Game::GetLocalPlayerByIndex(0);
        Frustum& frustum = player->GetFlashlightFrustum();

        //for (Door& door : GetDoors()) {
        //    const std::vector<RenderItem>& renderItems = door.GetRenderItems();
        //    for (const RenderItem& renderItem : renderItems) {
        //        g_instances.push_back(CreateObjectInstanceDataFromRenderItem(renderItem, door.GetObjectId()));
        //    }
        //}

        for (GameObject& gameObject : GetGameObjects()) {
            const std::vector<RenderItem>& renderItems = gameObject.GetRenderItems();
            for (const RenderItem& renderItem : renderItems) {
                CreateObjectInstanceDataFromRenderItem(renderItem, gameObject.GetObjectId(), frustum, g_instances);
            }
        }

        for (Light& light : GetLights()) {
            const std::vector<RenderItem>& renderItems = light.GetRenderItems();
            for (const RenderItem& renderItem : renderItems) {
                CreateObjectInstanceDataFromRenderItem(renderItem, light.GetObjectId(), frustum, g_instances);
            }
        }

        for (PickUp& pickUp : GetPickUps()) {
            const std::vector<RenderItem>& renderItems = pickUp.GetRenderItems();
            for (const RenderItem& renderItem : renderItems) {
                //PrimitiveInstance& instance = g_instances.emplace_back();
                CreateObjectInstanceDataFromRenderItem(renderItem, pickUp.GetObjectId(), frustum, g_instances);
            }
        }

        for (Tree& tree : GetTrees()) {
            const std::vector<RenderItem>& renderItems = tree.GetRenderItems();
            for (const RenderItem& renderItem : renderItems) {
                CreateObjectInstanceDataFromRenderItem(renderItem, tree.GetObjectId(), frustum, g_instances);
            }
        }

        //for (Window& window : GetWindows()) {
        //    const std::vector<RenderItem>& renderItems = window.GetRenderItems();
        //    for (const RenderItem& renderItem : renderItems) {
        //        g_instances.push_back(CreateObjectInstanceDataFromRenderItem(renderItem, window.GetObjectId()));
        //    }
        //    const std::vector<RenderItem>& glassRenderItems = window.GetGlassRenderItems();
        //    for (const RenderItem& renderItem : glassRenderItems) {
        //        g_instances.push_back(CreateObjectInstanceDataFromRenderItem(renderItem, window.GetObjectId()));
        //    }
        //}

        
        // Rebuild TLAS
        if (g_sceneBvhId == 0) {
            g_sceneBvhId = BVH::CreateNewSceneBvh();
        }
        BVH::UpdateSceneBvh(g_sceneBvhId, g_instances);

        //for (int i = 0; i < g_instances.size(); i++) {
        //    ObjectInstanceData& instance = g_instances[i];
        //    AABB aabb = AABB(instance.worldAabbBoundsMin, instance.worldAabbBoundsMax);
        //    if (!frustum.IntersectsAABB(aabb)) {
        // //       g_instances.erase(g_instances.begin() + i);
        //   //     i--;
        //    }
        //}
        //
        //BVH::DestroySceneBvh(g_sceneBvhId);
        //g_sceneBvhId = BVH::CreateSceneBvh(g_instances);

        // Debug draw
        //BVH::RenderSceneBvh(g_sceneBvhId, GREY);

        glm::vec3 rayOrigin = player->GetCameraPosition();
        glm::vec3 rayDir = player->GetCameraForward();
        float maxRayDistance = 1000.0f;

        if (Util::IsNan(rayDir)) {
            return;
        }

        BVH::RenderSceneBvh(g_sceneBvhId, GREEN);

        RayTraversalResult result = BVH::ClosestHit(g_sceneBvhId, rayOrigin, rayDir, maxRayDistance);
        if (result.hitFound) {
            PrimitiveInstance& instanceData = g_instances[result.primtiviveId];
            //Entity& entity = entities[instanceData.entityId];
            //BVH::RenderTriangleMeshTriangle(instanceData.triangleMeshBvhId, result.primtiviveId, GREEN, instanceData.worldTransform);
            BVH::RenderRayResultTriangle(result, GREEN);
            //std::cout << "hit found\n";
        } 
        //else {
        //    //std::cout << "hit not found\n";
        //}
    }

    void TestBvh() {

    }
}