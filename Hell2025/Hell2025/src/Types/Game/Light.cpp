#include "Light.h"
#include "AssetManagement/AssetManager.h"
#include "Physics/Physics.h"
#include "Util.h"
#include "UniqueID.h"

Light::Light(LightCreateInfo createInfo) {   
    m_position = createInfo.position;
    m_color = createInfo.color;
    m_radius = createInfo.radius;
    m_strength = createInfo.strength;
    m_type = Util::StringToLightType(createInfo.type);

    if (m_type == LightType::LAMP_POST) {
        m_material = AssetManager::GetMaterialByIndex(AssetManager::GetMaterialIndexByName("LampPost"));
        m_model0 = AssetManager::GetModelByIndex(AssetManager::GetModelIndexByName("LampPost"));
        m_transform0.position = m_position;
    }
    if (m_type == LightType::HANGING_LIGHT) {
        m_material = AssetManager::GetMaterialByIndex(AssetManager::GetMaterialIndexByName("Light"));
        m_model0 = AssetManager::GetModelByIndex(AssetManager::GetModelIndexByName("LightHanging"));
        m_transform0.position = m_position;
        PhysXRayResult rayResult = Physics::CastPhysXRay(m_position, glm::vec3(0.0f, 1.0f, 0.0f), 100.0f, RaycastGroup::RAYCAST_ENABLED);
        if (rayResult.hitFound) {
            m_model1 = AssetManager::GetModelByIndex(AssetManager::GetModelIndexByName("LightHangingCord"));
            m_model2 = AssetManager::GetModelByIndex(AssetManager::GetModelIndexByName("LightHangingMount"));
            m_transform1.position = m_position;
            m_transform1.scale.y = glm::distance(rayResult.hitPosition, m_position);
            m_transform2.position = rayResult.hitPosition;
            std::cout << "hit found: " << Util::Vec3ToString(rayResult.hitPosition) << "\n";
        }
        else {
            m_model1 = nullptr;
            m_model2 = nullptr;
            std::cout << "no hit\n";
        }
    }

    m_objectId = UniqueID::GetNext();
}


void Light::Update(float deltaTime) {
    UpdateRenderItems();
}

void Light::UpdateRenderItems() {
    m_renderItems.clear();

    if (m_model0) {
        for (uint32_t meshIndex : m_model0->GetMeshIndices()) {
            RenderItem& renderItem = m_renderItems.emplace_back();
            renderItem.objectType = (int)ObjectType::LIGHT;
            renderItem.mousePickIndex = m_mousePickIndex;
            renderItem.modelMatrix = m_transform0.to_mat4();
            renderItem.inverseModelMatrix = glm::inverse(renderItem.modelMatrix);
            renderItem.meshIndex = meshIndex;
            if (m_material) {
                renderItem.baseColorTextureIndex = m_material->m_basecolor;
                renderItem.normalMapTextureIndex = m_material->m_normal;
                renderItem.rmaTextureIndex = m_material->m_rma;
            }
            Mesh* mesh = AssetManager::GetMeshByIndex(meshIndex);
            if (mesh->GetName() == "Lamp") {
                renderItem.emissiveR = m_color.r;
                renderItem.emissiveG = m_color.g;
                renderItem.emissiveB = m_color.b;
            }
        }
    }
   
    if (m_model1) {
        for (uint32_t meshIndex : m_model1->GetMeshIndices()) {
            RenderItem& renderItem = m_renderItems.emplace_back();
            renderItem.objectType = (int)ObjectType::LIGHT;
            renderItem.mousePickIndex = m_mousePickIndex;
            renderItem.modelMatrix = m_transform1.to_mat4();
            renderItem.inverseModelMatrix = glm::inverse(renderItem.modelMatrix);
            renderItem.meshIndex = meshIndex;
            if (m_material) {
                renderItem.baseColorTextureIndex = m_material->m_basecolor;
                renderItem.normalMapTextureIndex = m_material->m_normal;
                renderItem.rmaTextureIndex = m_material->m_rma;
            }
            Mesh* mesh = AssetManager::GetMeshByIndex(meshIndex);
            if (mesh->GetName() == "Lamp") {
                renderItem.emissiveR = m_color.r;
                renderItem.emissiveG = m_color.g;
                renderItem.emissiveB = m_color.b;
            }
        }
    }
    if (m_model2) {
        for (uint32_t meshIndex : m_model2->GetMeshIndices()) {
            RenderItem& renderItem = m_renderItems.emplace_back();
            renderItem.objectType = (int)ObjectType::LIGHT;
            renderItem.mousePickIndex = m_mousePickIndex;
            renderItem.modelMatrix = m_transform2.to_mat4();
            renderItem.inverseModelMatrix = glm::inverse(renderItem.modelMatrix);
            renderItem.meshIndex = meshIndex;
            if (m_material) {
                renderItem.baseColorTextureIndex = m_material->m_basecolor;
                renderItem.normalMapTextureIndex = m_material->m_normal;
                renderItem.rmaTextureIndex = m_material->m_rma;
            }
            Mesh* mesh = AssetManager::GetMeshByIndex(meshIndex);
            if (mesh->GetName() == "Lamp") {
                renderItem.emissiveR = m_color.r;
                renderItem.emissiveG = m_color.g;
                renderItem.emissiveB = m_color.b;
            }
        }
    }

    for (RenderItem& renderItem : m_renderItems) {
        Util::UpdateRenderItemAABB(renderItem);
    }
}

void Light::SetMousePickIndex(int index) {
    m_mousePickIndex = index;
}

void Light::SetPosition(glm::vec3 position) {
    m_position = position;
}

glm::vec3 Light::GetPosition() {
    return m_position;
}

glm::vec3 Light::GetColor() {
    return m_color;
}

float Light::GetStrength() {
    return m_strength;
}

float Light::GetRadius() {
    return m_radius;
}

LightCreateInfo Light::GetCreateInfo() {
    LightCreateInfo createInfo;
    createInfo.position = m_position;
    createInfo.color = m_color;
    createInfo.strength = m_strength;
    createInfo.radius = m_radius;
    createInfo.type = Util::LightTypeToString(m_type);
    return createInfo;
}
