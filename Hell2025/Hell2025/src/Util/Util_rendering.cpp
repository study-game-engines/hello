#include "Util.h"
#include "AssetManagement/AssetManager.h"
#include "Input/Input.h"
#include <array>

namespace Util {

    void UpdateRenderItemAABBFastA(RenderItem& renderItem) {
        Mesh* mesh = AssetManager::GetMeshByIndex(renderItem.meshIndex);
        if (!mesh) return;

        renderItem.aabbMin = glm::vec4(std::numeric_limits<float>::max());
        renderItem.aabbMax = glm::vec4(-std::numeric_limits<float>::max());

        const std::array<glm::vec4, 8> corners = {
            glm::vec4(renderItem.modelMatrix * glm::vec4(mesh->aabbMin.x, mesh->aabbMax.y, mesh->aabbMax.z, 1.0f)),
            glm::vec4(renderItem.modelMatrix * glm::vec4(mesh->aabbMax.x, mesh->aabbMax.y, mesh->aabbMax.z, 1.0f)),
            glm::vec4(renderItem.modelMatrix * glm::vec4(mesh->aabbMin.x, mesh->aabbMin.y, mesh->aabbMax.z, 1.0f)),
            glm::vec4(renderItem.modelMatrix * glm::vec4(mesh->aabbMax.x, mesh->aabbMin.y, mesh->aabbMax.z, 1.0f)),
            glm::vec4(renderItem.modelMatrix * glm::vec4(mesh->aabbMin.x, mesh->aabbMax.y, mesh->aabbMin.z, 1.0f)),
            glm::vec4(renderItem.modelMatrix * glm::vec4(mesh->aabbMax.x, mesh->aabbMax.y, mesh->aabbMin.z, 1.0f)),
            glm::vec4(renderItem.modelMatrix * glm::vec4(mesh->aabbMin.x, mesh->aabbMin.y, mesh->aabbMin.z, 1.0f)),
            glm::vec4(renderItem.modelMatrix * glm::vec4(mesh->aabbMax.x, mesh->aabbMin.y, mesh->aabbMin.z, 1.0f))
        };

        for (const auto& corner : corners) {
            renderItem.aabbMin = glm::min(renderItem.aabbMin, corner);
            renderItem.aabbMax = glm::max(renderItem.aabbMax, corner);
        }
    }

    void UpdateRenderItemAABBFastB(RenderItem& renderItem) {
        Mesh* mesh = AssetManager::GetMeshByIndex(renderItem.meshIndex);
        if (!mesh) return;

        glm::vec3& localMin = mesh->aabbMin;
        glm::vec3& localMax = mesh->aabbMax;
        glm::vec3 localCenter = (localMin + localMax) * 0.5f;
        glm::vec3 localHalfExtents = (localMax - localMin) * 0.5f;
        glm::vec3 worldCenter = glm::vec3(renderItem.modelMatrix * glm::vec4(localCenter, 1.0f));

        const glm::mat3 R = glm::mat3(renderItem.modelMatrix);

        glm::vec3 abs_row0 = glm::abs(glm::vec3(R[0][0], R[1][0], R[2][0]));
        glm::vec3 abs_row1 = glm::abs(glm::vec3(R[0][1], R[1][1], R[2][1]));
        glm::vec3 abs_row2 = glm::abs(glm::vec3(R[0][2], R[1][2], R[2][2]));

        glm::vec3 worldHalfExtents = glm::vec3(
            glm::dot(abs_row0, localHalfExtents),
            glm::dot(abs_row1, localHalfExtents),
            glm::dot(abs_row2, localHalfExtents)
        );

        glm::vec3 worldMin = worldCenter - worldHalfExtents;
        glm::vec3 worldMax = worldCenter + worldHalfExtents;

        renderItem.aabbMin = glm::vec4(worldMin, 1.0f);
        renderItem.aabbMax = glm::vec4(worldMax, 1.0f);
    }

    void UpdateRenderItemAABB(RenderItem& renderItem) {
        Mesh* mesh = AssetManager::GetMeshByIndex(renderItem.meshIndex);
        if (!mesh) return;

        glm::vec3 aabbMin = glm::vec3(std::numeric_limits<float>::max());
        glm::vec3 aabbMax = glm::vec3(-std::numeric_limits<float>::max());

        const std::array<glm::vec3, 8> corners = {
            glm::vec3(renderItem.modelMatrix * glm::vec4(mesh->aabbMin.x, mesh->aabbMax.y, mesh->aabbMax.z, 1.0f)),
            glm::vec3(renderItem.modelMatrix * glm::vec4(mesh->aabbMax.x, mesh->aabbMax.y, mesh->aabbMax.z, 1.0f)),
            glm::vec3(renderItem.modelMatrix * glm::vec4(mesh->aabbMin.x, mesh->aabbMin.y, mesh->aabbMax.z, 1.0f)),
            glm::vec3(renderItem.modelMatrix * glm::vec4(mesh->aabbMax.x, mesh->aabbMin.y, mesh->aabbMax.z, 1.0f)),
            glm::vec3(renderItem.modelMatrix * glm::vec4(mesh->aabbMin.x, mesh->aabbMax.y, mesh->aabbMin.z, 1.0f)),
            glm::vec3(renderItem.modelMatrix * glm::vec4(mesh->aabbMax.x, mesh->aabbMax.y, mesh->aabbMin.z, 1.0f)),
            glm::vec3(renderItem.modelMatrix * glm::vec4(mesh->aabbMin.x, mesh->aabbMin.y, mesh->aabbMin.z, 1.0f)),
            glm::vec3(renderItem.modelMatrix * glm::vec4(mesh->aabbMax.x, mesh->aabbMin.y, mesh->aabbMin.z, 1.0f))
        };

        for (const auto& corner : corners) {
            aabbMin = glm::min(aabbMin, corner);
            aabbMax = glm::max(aabbMax, corner);
        }

        renderItem.aabbMin = glm::vec4(aabbMin, 0.0f);
        renderItem.aabbMax = glm::vec4(aabbMax, 0.0f);
    }

    AABB ComputeWorldAABB(glm::vec3& localAabbMin, glm::vec3& localAabbMax, glm::mat4& modelMatrix) {
        glm::vec3 worldAabbMin = glm::vec3(std::numeric_limits<float>::max());
        glm::vec3 worldAabbMax = glm::vec3(-std::numeric_limits<float>::max());

        std::vector<glm::vec3> corners = {
            modelMatrix* glm::vec4(localAabbMin.x, localAabbMax.y, localAabbMax.z, 1.0f),
            modelMatrix* glm::vec4(localAabbMax.x, localAabbMax.y, localAabbMax.z, 1.0f),
            modelMatrix* glm::vec4(localAabbMin.x, localAabbMin.y, localAabbMax.z, 1.0f),
            modelMatrix* glm::vec4(localAabbMax.x, localAabbMin.y, localAabbMax.z, 1.0f),
            modelMatrix* glm::vec4(localAabbMin.x, localAabbMax.y, localAabbMin.z, 1.0f),
            modelMatrix* glm::vec4(localAabbMax.x, localAabbMax.y, localAabbMin.z, 1.0f),
            modelMatrix* glm::vec4(localAabbMin.x, localAabbMin.y, localAabbMin.z, 1.0f),
            modelMatrix* glm::vec4(localAabbMax.x, localAabbMin.y, localAabbMin.z, 1.0f)
        };

        for (glm::vec3& corner : corners) {
            worldAabbMin = glm::min(worldAabbMin, corner);
            worldAabbMax = glm::max(worldAabbMax, corner);
        }

        return AABB(worldAabbMin, worldAabbMax);
    }
}