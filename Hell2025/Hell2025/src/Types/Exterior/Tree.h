#pragma once
#include "HellTypes.h"
#include "CreateInfo.h"
#include "Types/Renderer/Model.h"
#include "Types/Renderer/MeshNodes.h"

struct Tree {
    Tree() = default;
    Tree(TreeCreateInfo createInfo);
    TreeCreateInfo GetCreateInfo();
    void SetPosition(glm::vec3 position);
    void BeginFrame();
    void MarkAsSelected();
    void Update(float deltaTime);
    void UpdateRenderItems();

    bool IsSelected();

    const std::vector<RenderItem>& GetRenderItems() const                   { return m_meshNodes.GetRenderItems(); }
    const std::vector<RenderItem>& GetRenderItemsBlended()const             { return m_meshNodes.GetRenderItemsBlended(); }
    const std::vector<RenderItem>& GetRenderItemsAlphaDiscarded() const     { return m_meshNodes.GetRenderItemsAlphaDiscarded(); }
    const std::vector<RenderItem>& GetRenderItemsHairTopLayer() const       { return m_meshNodes.GetRenderItemsHairTopLayer(); }
    const std::vector<RenderItem>& GetRenderItemsHairBottomLayer() const    { return m_meshNodes.GetRenderItemsHairBottomLayer(); }
    const glm::vec3& GetPosition() const                    { return m_transform.position; }
    const glm::vec3& GetRotation() const                    { return m_transform.rotation; }
    const glm::vec3& GetScale() const                       { return m_transform.scale; }
    const glm::mat4& GetModelMatrix() const                 { return m_modelMatrix; }
    const uint64_t GetObjectId() const                      { return m_objectId; }

private:
    MeshNodes m_meshNodes;
    uint64_t m_objectId = 0;
    Model* m_model = nullptr;
    TreeType m_treeType = TreeType::UNDEFINED;
    Transform m_transform;
    glm::mat4 m_modelMatrix = glm::mat4(1.0f);
    std::vector<RenderItem> m_renderItems;
    bool m_isSelected = false;
};