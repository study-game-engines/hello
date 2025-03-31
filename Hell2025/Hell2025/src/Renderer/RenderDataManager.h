#pragma once
#include "HellTypes.h"
#include "Types/Game/AnimatedGameObject.h"
#include <vector>

namespace RenderDataManager {
    void BeginFrame();
    void Update();


    void SubmitAnimatedGameObjectForSkinning(AnimatedGameObject* animatedGameObject);
    void ResetBaseSkinnedVertex();
    void IncrementBaseSkinnedVertex(uint32_t vertexCount);

    uint32_t GetBaseSkinnedVertex();
    std::vector<AnimatedGameObject*>& GetAnimatedGameObjectToSkin();

    inline std::vector<glm::mat4> skinningTransforms;

    int EncodeBaseInstance(int playerIndex, int instanceOffset);
    void DecodeBaseInstance(int baseInstance, int& playerIndex, int& instanceOffset);

    // Submissions
    void SubmitDecalRenderItem(const RenderItem& renderItem);
    void SubmitRenderItem(const RenderItem& renderItem);
    void SubmitHouseRenderItem(const HouseRenderItem& renderItem);
    void SubmitOutlineRenderItem(const RenderItem& renderItem);
    void SubmitOutlineRenderItems(const std::vector<RenderItem>& renderItems);

    const RendererData& GetRendererData();
    const std::vector<GPULight>& GetGPULightData();
    const std::vector<HouseRenderItem>& GetHouseRenderItems();
    const std::vector<RenderItem>& GetDecalRenderItems();
    const std::vector<RenderItem>& GetInstanceData();
    const std::vector<RenderItem>& GetOutlineRenderItems();
    const std::vector<ViewportData>& GetViewportData();
    const DrawCommandsSet& GetDrawInfoSet();

}