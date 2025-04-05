#pragma once
#include "HellTypes.h"
#include "CreateInfo.h"

struct BasicDoor {
    uint32_t m_meshIndex = 0;
    Material* m_material = nullptr;
    glm::vec3 m_rotation;
    glm::mat4 m_parentMatrix = glm::mat4(1.0f);

    std::vector<MeshRenderingInfo> m_meshRenderingInfoSet;
    std::vector<glm::mat4> m_localOffsetMatrices;
    std::vector<glm::mat4> m_inverselocalOffsetMatrices;

    enum struct Axis {
        AXIS_X,
        AXIS_Y,
        AXIS_Z,
    } m_rotationAxis = Axis::AXIS_Y;

    BasicDoor() = default;
    BasicDoor(BasicDoorCreateInfo createInfo);
};