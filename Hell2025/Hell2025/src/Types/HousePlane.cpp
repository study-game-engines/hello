#include "HousePlane.h"
#include "AssetManagement/AssetManager.h"
#include "Physics/Physics.h"
#include "Util/Util.h"
#include "UniqueID.h"

void HousePlane::InitFromPoints(glm::vec3 p0, glm::vec3 p1, glm::vec3 p2, glm::vec3 p3) {
    m_p0 = p0;
    m_p1 = p1;
    m_p2 = p2;
    m_p3 = p3;

    m_vertices.clear();
    m_indices.clear();

    // Vertices
    m_vertices.push_back(m_p0);
    m_vertices.push_back(m_p1);
    m_vertices.push_back(m_p2);
    m_vertices.push_back(m_p3);

    // Indices
    m_indices = { 0, 1, 2, 2, 3, 0 };

    // Update UVs
    float textureScale = 0.5f;
    float textureOffsetX = 0.0f;
    float textureOffsetY = 0.0f;
    for (Vertex& vertex : m_vertices) {
        glm::vec3 origin = glm::vec3(0, 0, 0);
        origin = glm::vec3(0);
        vertex.uv = Util::CalculateUV(vertex.position, glm::vec3(0.0f, 1.0f, 0.0f));
        vertex.uv *= textureScale;
        vertex.uv.x += textureOffsetX;
        vertex.uv.y += textureOffsetY;
    }

    // Update normals and tangents
    for (int i = 0; i < m_indices.size(); i += 3) {
        Vertex& v0 = m_vertices[m_indices[i + 0]];
        Vertex& v1 = m_vertices[m_indices[i + 1]];
        Vertex& v2 = m_vertices[m_indices[i + 2]];
        Util::SetNormalsAndTangentsFromVertices(v0, v1, v2);
    }

    CreatePhysicsObject();
}

void HousePlane::CleanUp() {
    Physics::MarkRigidStaticForRemoval(m_physicsId);
    m_vertices.clear();
    m_indices.clear();
    m_objectId = 0;
    m_physicsId = 0;
    m_p0 = glm::vec3(0.0f);
    m_p1 = glm::vec3(0.0f);
    m_p2 = glm::vec3(0.0f);
    m_p3 = glm::vec3(0.0f);
    Material* m_material = nullptr;
}

void HousePlane::SetMaterial(const std::string& materialName) {
    m_material = AssetManager::GetMaterialByName(materialName);
}

void HousePlane::CreatePhysicsObject() {
    Physics::MarkRigidStaticForRemoval(m_physicsId);

    PhysicsFilterData filterData;
    filterData.raycastGroup = RAYCAST_ENABLED;
    filterData.collisionGroup = CollisionGroup::ENVIROMENT_OBSTACLE;
    filterData.collidesWith = (CollisionGroup)(GENERIC_BOUNCEABLE | BULLET_CASING | RAGDOLL | PLAYER | ITEM_PICK_UP);

    m_physicsId = Physics::CreateRigidStaticTriangleMeshFromVertexData(Transform(), m_vertices, m_indices, filterData);
    m_objectId = UniqueID::GetNext();

    // Set PhysX user data
    PhysicsUserData userData;
    userData.physicsId = m_physicsId;
    userData.objectId = m_objectId;
    userData.physicsType = PhysicsType::RIGID_STATIC;
    userData.objectType = ObjectType::HOUSE_PLANE;
    Physics::SetRigidStaticUserData(m_physicsId, userData);
}