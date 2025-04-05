#include "World.h"
#include "AssetManagement/AssetManager.h"
#include "Core/Game.h"
#include "Bvh/Bvh.h";
#include "Viewport/ViewportManager.h";

#include "Renderer/Renderer.h"
#include "Input/Input.h"

namespace World {
          
    ViewportBvhData g_viewportBvhData[4];

    void CreateObjectInstanceDataFromRenderItem(const RenderItem& renderItem, Frustum& frustum, std::vector<PrimitiveInstance>& container) {
        if (frustum.IntersectsAABB(renderItem)) {
            PrimitiveInstance& instance = container.emplace_back();
            instance.objectType = Util::IntToEnum(renderItem.objectType);
            instance.worldTransform = renderItem.modelMatrix;
            instance.worldAabbBoundsMin = renderItem.aabbMin;
            instance.worldAabbBoundsMax = renderItem.aabbMax;
            instance.worldAabbCenter = (renderItem.aabbMin + renderItem.aabbMax) * 0.5f;
            instance.meshBvhId = AssetManager::GetMeshByIndex(renderItem.meshIndex)->meshBvhId;
            Util::UnpackUint64(renderItem.objectIdLowerBit, renderItem.objectIdUpperBit, instance.objectId);
        }
    }


    void UpdateSceneBvh() {

        for (int i = 0; i < 4; i++) {
            Viewport* viewport = ViewportManager::GetViewportByIndex(i);
            if (!viewport->IsVisible()) continue;

            ViewportBvhData& viewportBvhData = g_viewportBvhData[i];
            viewportBvhData.instances.clear();

            Player* player = Game::GetLocalPlayerByIndex(i);
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
                    CreateObjectInstanceDataFromRenderItem(renderItem, frustum, viewportBvhData.instances);
                }
            }

            for (Light& light : GetLights()) {
                const std::vector<RenderItem>& renderItems = light.GetRenderItems();
                for (const RenderItem& renderItem : renderItems) {
                    CreateObjectInstanceDataFromRenderItem(renderItem, frustum, viewportBvhData.instances);
                }
            }

            for (PickUp& pickUp : GetPickUps()) {
                const std::vector<RenderItem>& renderItems = pickUp.GetRenderItems();
                for (const RenderItem& renderItem : renderItems) {
                    CreateObjectInstanceDataFromRenderItem(renderItem, frustum, viewportBvhData.instances);
                }
            }

            for (Piano& piano : GetPianos()) {
                const std::vector<RenderItem>& renderItems = piano.GetRenderItems();
                for (const RenderItem& renderItem : renderItems) {
                    CreateObjectInstanceDataFromRenderItem(renderItem, frustum, viewportBvhData.instances);
                }
            }

            for (Tree& tree : GetTrees()) {
                const std::vector<RenderItem>& renderItems = tree.GetRenderItems();
                for (const RenderItem& renderItem : renderItems) {
                    CreateObjectInstanceDataFromRenderItem(renderItem, frustum, viewportBvhData.instances);
                }
            }

            // Rebuild TLAS
            if (viewportBvhData.sceneBvhId == 0) {
                viewportBvhData.sceneBvhId = BVH::CreateNewSceneBvh();
            }
            BVH::UpdateSceneBvh(viewportBvhData.sceneBvhId, viewportBvhData.instances);
        }




       // for (int i = 0; i < 4; i++) {
       //     Viewport* viewport = ViewportManager::GetViewportByIndex(i);
       //     if (!viewport->IsVisible()) continue;
       //
       //     ViewportBvhData& viewportBvhData = g_viewportBvhData[i];
       //     SceneBvh* sceneBVH = BVH::GetSceneBvhById(viewportBvhData.sceneBvhId);
       //
       //     Player* player = Game::GetLocalPlayerByIndex(i);
       //
       //     glm::vec3 rayOrigin = player->GetCameraPosition();
       //     glm::vec3 rayDir = player->GetCameraForward();
       //     float maxRayDistance = 4;
       //
       //     RayTraversalResult result = ClosestHit(rayOrigin, rayDir, maxRayDistance, i);
       //     if (result.hitFound) {
       //
       //         std::vector<PrimitiveInstance>& instances = g_viewportBvhData[i].instances;
       //
       //         PrimitiveInstance& instanceData = instances[result.primtiviveId];
       //         BVH::RenderRayResultTriangle(result, GREEN);
       //     }
       //
       //     //for (PrimitiveInstance& instance : viewportBvhData.instances) {
       //     //   //BVH::RenderMeshBvh(instance.meshBvhId, GREY, instance.worldTransform);            
       //     //}
       //     //BVH::RenderSceneBvh(viewportBvhData.sceneBvhId, GREEN);
       //                 
       // }
    }

    RayTraversalResult ClosestHit(glm::vec3 rayOrigin, glm::vec3 rayDir, float maxRayDistance, int viewportIndex) {
        std::vector<PrimitiveInstance>& instances = g_viewportBvhData[viewportIndex].instances;
        uint64_t& sceneBvhId = g_viewportBvhData[viewportIndex].sceneBvhId;
        if (Util::IsNan(rayDir)) {
            return RayTraversalResult();
        }
        return BVH::ClosestHit(sceneBvhId, rayOrigin, rayDir, maxRayDistance);;
    }

    void TestBvh() {

    }
}