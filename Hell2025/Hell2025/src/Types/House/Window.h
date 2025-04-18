#pragma once
#include "HellTypes.h"
#include "CreateInfo.h"
#include "Types/Renderer/Model.h"

struct Window {
    void Init(WindowCreateInfo createInfo);
    void Update(float deltaTime);
    void CleanUp();
    void SetMousePickIndex(int mousePickIndex);
    void UpdateRenderItems();
    void SubmitRenderItems();

    const glm::vec3& GetPosition() const { return m_position; }
    const glm::vec3& GetRotation() const { return m_rotation; }
    const uint64_t GetObjectId() const { return m_objectId; }
    const std::vector<RenderItem>& GetRenderItems() const { return m_renderItems; }
    const std::vector<RenderItem>& GetGlassRenderItems() const { return m_glassRenderItems; }
    const WindowCreateInfo& GetCreateInfo() const { return m_createInfo; }

private:
    WindowCreateInfo m_createInfo;
    uint64_t m_objectId = 0;
    uint64_t m_physicsId = 0;
    Material* m_interiorMaterial = nullptr;
    Material* m_exteriorMaterial = nullptr;
    Model* m_model = nullptr;
    Model* m_glassModel = nullptr;
    std::vector<RenderItem> m_renderItems;
    std::vector<RenderItem> m_glassRenderItems;
    glm::vec3 m_position = glm::vec3(0.0f);
    glm::vec3 m_rotation = glm::vec3(0.0f);
    int m_mousePickIndex = 0;
    glm::mat4 m_modelMatrix = glm::mat4(1.0f);
};