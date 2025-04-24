#pragma once
#include "HellTypes.h"
#include "CreateInfo.h"
#include "Physics/Types/RigidDynamic.h"
#include "Types/Renderer/Model.h"

struct GameObject {
    GameObject() = default;
    GameObject(GameObjectCreateInfo createInfo);
    GameObjectCreateInfo GetCreateInfo();

    std::string m_name;
    Model* m_model = nullptr;
    Transform m_transform;
    std::vector<MeshRenderingInfo> m_meshRenderingInfoSet;

    void CleanUp();
    void Update(float deltaTime);
    void SetName(const std::string& name);
    void SetPosition(glm::vec3 position);
    void SetRotation(glm::vec3 rotation);
    void SetRotationY(float rotation);
    void SetScale(glm::vec3 scale);
    void SetModel(const std::string& name);
    void SetMeshMaterialByMeshName(const std::string& meshName, const std::string& materialName);
    void SetMeshBlendingMode(const std::string& meshName, BlendingMode blendingMode);
    void SetMeshBlendingModes(BlendingMode blendingMode);
    void PrintMeshNames();
    void UpdateRenderItems();
    void SetConvexHullsFromModel(const std::string modelName);
    
    void BeginFrame();
    void MarkAsSelected();
    bool IsSelected();

    glm::vec3 GetPosition() const;
    glm::vec3 GetEulerRotation() const;
    glm::vec3 GetScale() const;
    const glm::mat4 GetModelMatrix();
    const glm::vec3 GetObjectCenter();
    const glm::vec3 GetObjectCenterOffsetFromOrigin();

    const std::vector<RenderItem>& GetRenderItems() const                   { return m_renderItems; }
    const std::vector<RenderItem>& GetRenderItemsBlended()const             { return m_renderItemsBlended; }
    const std::vector<RenderItem>& GetRenderItemsAlphaDiscarded() const     { return m_renderItemsAlphaDiscarded; }
    const std::vector<RenderItem>& GetRenderItemsHairTopLayer() const       { return m_renderItemsHairTopLayer; }
    const std::vector<RenderItem>& GetRenderItemsHairBottomLayer() const    { return m_renderItemsHairBottomLayer; }
    const uint64_t GetObjectId() const                                      { return m_objectId; }

private:
    uint64_t m_physicsId = 0;
    uint64_t m_objectId = 0;
    bool m_selected = false;
    bool m_hasPhysics = false;
    std::vector<RenderItem> m_renderItems;
    std::vector<RenderItem> m_renderItemsBlended;
    std::vector<RenderItem> m_renderItemsAlphaDiscarded;
    std::vector<RenderItem> m_renderItemsHairTopLayer;
    std::vector<RenderItem> m_renderItemsHairBottomLayer;    
};