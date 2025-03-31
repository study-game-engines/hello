#pragma once
#include "glm/vec3.hpp"
#include "Util.h"
#include "CreateInfo.h"

struct Decal {
    void Init(const DecalCreateInfo& createInfo);
    void SubmitRenderItem();

    const glm::mat4& GetModelMatrix()        const { return m_modelMatrix; }
    const glm::vec3& GetPosition()           const { return glm::vec3(m_modelMatrix[3]); }
    const glm::vec3& GetWorldNormal()        const { return glm::normalize(glm::vec3(m_modelMatrix[2])); }

private:
    float m_randomRotation = 0;
    uint64_t m_parentPhysicsId = 0;
    uint64_t m_parentObjectId = 0;
    Material* m_material = nullptr;
    PhysicsType m_parentPhysicsType = PhysicsType::UNDEFINED;
    ObjectType m_parentObjectType = ObjectType::UNDEFINED;
    AABB m_localAABB;
    glm::vec3 m_localNormal = glm::vec3(0.0f); 
    glm::mat4 m_modelMatrix = glm::mat4(1.0f);
    glm::mat4 m_localMatrix = glm::mat4(1.0f);
};


