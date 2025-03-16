#pragma once
#include "HellTypes.h"
#include "CreateInfo.h"
#include "Renderer/Types/Model.hpp"

struct Door {
    void Init(DoorCreateInfo createInfo);
    void Update(float deltaTime);
    void CleanUp();
    void SetMousePickIndex(int mousePickIndex);
    void UpdateRenderItems();
    void Interact();

    const glm::vec3& GetPosition() const                    { return m_position; }
    const glm::vec3& GetRotation() const                    { return m_rotation; }
    const uint64_t GetObjectId() const                      { return m_objectId; }
    const OpeningState& GetOpeningState() const             { return m_openingState; }
    const std::vector<RenderItem>& GetRenderItems() const   { return m_renderItems; }

private:
    uint64_t m_objectId = 0;
    uint64_t m_physicsId = 0;
    Model* m_doorModel = nullptr;
    Model * m_frameModel = nullptr;
    Material* m_material = nullptr;
    OpeningState m_openingState = OpeningState::CLOSED;
    std::vector<RenderItem> m_renderItems;
    glm::vec3 m_position;
    glm::vec3 m_rotation;
    float m_currentOpenRotation = 0;
    int m_mousePickIndex = 0;
    float m_maxOpenRotation = 1.8f;
    glm::mat4 m_doorModelMatrix = glm::mat4(1.0f);
    glm::mat4 m_frameModelMatrix = glm::mat4(1.0f);
};