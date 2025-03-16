#pragma once
#include "glm/vec3.hpp"
#include "Util.h"
#include "CreateInfo.h"

struct Decal {
    void Init(const DecalCreateInfo& createInfo);

    const glm::mat4& GetModelMatrix()        const { return m_modelMatrix; }
    const glm::vec3& GetPosition()           const { return m_position; }
    const glm::vec3& GetRotation()           const { return m_rotation; }
    const glm::vec3& GetScale()              const { return m_scale; }

private:
    uint64_t m_parentPhysicsId = 0;
    uint64_t m_parentObjectId = 0;
    PhysicsType m_parentPhysicsType = PhysicsType::UNDEFINED;
    ObjectType m_parentObjectType = ObjectType::UNDEFINED;
    glm::vec3 m_localPosition = glm::vec3(0.0f);
    glm::vec3 m_localNormal = glm::vec3(0.0f);
    glm::vec3 m_position = glm::vec3(0.0f);
    glm::vec3 m_rotation = glm::vec3(0.0f);
    glm::vec3 m_scale = glm::vec3(1.0f);
    glm::mat4 m_modelMatrix = glm::mat4(1.0f);
};


