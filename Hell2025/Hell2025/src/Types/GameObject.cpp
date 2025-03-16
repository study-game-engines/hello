#include "GameObject.h"
#include "AssetManagement/AssetManager.h"
#include "Physics/Physics.h"
#include "Util.h"

GameObject::GameObject(GameObjectCreateInfo createInfo) {
    SetModel(createInfo.modelName);

    // For each MeshRenderingInfo in m_meshRenderingInfoSet, 
    // find the matching one in createInfo.meshRenderingInfoSet by meshIndex, and update it.
    for (MeshRenderingInfo& meshRenderingInfo : m_meshRenderingInfoSet) {
        for (const MeshRenderingInfo& queryMeshRenderingInfo : createInfo.meshRenderingInfoSet) {
            if (meshRenderingInfo.meshIndex == queryMeshRenderingInfo.meshIndex) {
                meshRenderingInfo = queryMeshRenderingInfo;
                break;
            }
        }
    }

    m_transform.position = createInfo.position;
    m_transform.rotation = createInfo.rotation;
    m_transform.scale = createInfo.scale;

    if (createInfo.modelName == "Bench2") {

        PhysicsFilterData filterData;
        filterData.raycastGroup = RaycastGroup::RAYCAST_ENABLED;
        filterData.collisionGroup = CollisionGroup::ENVIROMENT_OBSTACLE;
        filterData.collidesWith = (CollisionGroup)(PLAYER | BULLET_CASING | ITEM_PICK_UP);


        m_physicsId = Physics::CreateRigidStaticFromConvexMeshFromModel(m_transform, "Bench_ConvexHulls", filterData);


       //Model* convexHullsModel = AssetManager::GetModelByName("Bench_ConvexHulls");
       //for (uint32_t meshIndex : convexHullsModel->GetMeshIndices()) {
       //    Mesh* mesh = AssetManager::GetMeshByIndex(meshIndex);
       //
       //    std::span<Vertex> vertices = AssetManager::GetVerticesSpan(mesh->baseVertex, mesh->vertexCount);
       //
       //
       //    m_physicsId = Physics::CreateRigidStaticFromConvexMeshVertices(m_transform, vertices, filterData);
       //
       //    std::cout << "attempting to create convex hull for mesh index " << meshIndex << "\n";
       //
       //    //break;
       //
       //}
       //glm::vec3 modelExtents = glm::vec3(1.0f);
       ////m_physicsId = Physics::CreateRigidStaticFromBoxExtents(m_transform, modelExtents, filterData);
       //m_physicsId = Physics::CreateRigidStaticFromConvexMeshVertices(m_transform, modelExtents, filterData);


        //SetConvexHullsFromModel("Bench_ConvexHulls");

        std::cout << "FUUUUUUUUUUUUUCK\n";
    }
}

GameObjectCreateInfo GameObject::GetCreateInfo() {
    GameObjectCreateInfo createInfo;
    createInfo.position = m_transform.position;
    createInfo.rotation = m_transform.rotation;
    createInfo.scale = m_transform.scale;
    createInfo.modelName = m_model->GetName();
    createInfo.meshRenderingInfoSet = m_meshRenderingInfoSet;
    return createInfo;
}

void GameObject::SetPosition(glm::vec3 position) {
    m_transform.position = position;
}

void GameObject::SetRotation(glm::vec3 rotation) {
    m_transform.rotation = rotation;
}

void GameObject::SetRotationY(float rotation) {
    m_transform.rotation.y = rotation;
}

void GameObject::SetScale(glm::vec3 scale) {
    m_transform.scale = scale;
}

void GameObject::SetModel(const std::string& name) {
    m_model = AssetManager::GetModelByIndex(AssetManager::GetModelIndexByName(name.c_str()));
    if (m_model) {
        m_meshRenderingInfoSet.clear();
        for (uint32_t& meshIndex : m_model->GetMeshIndices()) {
            MeshRenderingInfo& meshRenderingInfo = m_meshRenderingInfoSet.emplace_back();
            meshRenderingInfo.meshIndex = meshIndex;
            meshRenderingInfo.materialIndex = AssetManager::GetMaterialIndexByName(DEFAULT_MATERIAL_NAME);
        }
    }
    else {
        std::cout << "Failed to set model '" << name << "', it does not exist.\n";
    }
}

void GameObject::SetMeshMaterialByMeshName(const std::string& meshName, const std::string& materialName) {    
    Material* material = AssetManager::GetMaterialByName(materialName);
    
    if (!m_model) return; // Invalid model
    if (!material) return; // Invalid material
 
    // Iterate each mesh, find the match based on name, and set the corresponding MeshRenderingInfo's material name
    for (uint32_t meshIndex : m_model->GetMeshIndices()) {
        Mesh* mesh = AssetManager::GetMeshByIndex(meshIndex); {
            if (mesh->GetName() == meshName) {
                for (MeshRenderingInfo& meshRenderingInfo : m_meshRenderingInfoSet) {
                    if (meshRenderingInfo.meshIndex == meshIndex) {
                        meshRenderingInfo.materialIndex = AssetManager::GetMaterialIndexByName(materialName);
                    }
                }
            }
        }
    }
}

void GameObject::SetMeshBlendingMode(const std::string& meshName, BlendingMode blendingMode) {
    if (!m_model) return; // Invalid model

    // Iterate each mesh, find the match based on name, and set the corresponding MeshRenderingInfo's blending mode
    for (uint32_t meshIndex : m_model->GetMeshIndices()) {
        Mesh* mesh = AssetManager::GetMeshByIndex(meshIndex); {
            if (mesh->GetName() == meshName) {
                for (MeshRenderingInfo& meshRenderingInfo : m_meshRenderingInfoSet) {
                    if (meshRenderingInfo.meshIndex == meshIndex) {
                        meshRenderingInfo.blendingMode = blendingMode;
                    }
                }
            }
        }
    }
}

void GameObject::SetMousePickIndex(int index) {
    m_mousePickIndex = index;
}

void GameObject::SetName(const std::string& name) {
    m_name = name;
}

void GameObject::PrintMeshNames() {
    if (m_model) {
        std::cout << m_model->GetName() << "\n";
        for (uint32_t meshIndex : m_model->GetMeshIndices()) {
            Mesh* mesh = AssetManager::GetMeshByIndex(meshIndex);
            if (mesh) {
                std::cout << "-" << meshIndex << ": " << mesh->GetName() << "\n";
            }
        }
    }
}


void GameObject::BeginFrame() {
    m_selected = false;
}

void GameObject::MarkAsSelected() {
    m_selected = true;
}

bool GameObject::IsSelected() {
    return m_selected;
}

void GameObject::UpdateRenderItems() {
    m_renderItems.clear();
    m_renderItemsBlended.clear();
    m_renderItemsAlphaDiscarded.clear();
    m_renderItemsHairTopLayer.clear();
    m_renderItemsHairBottomLayer.clear();

    for (MeshRenderingInfo& meshRenderingInfo : m_meshRenderingInfoSet) {
        Material* material = AssetManager::GetMaterialByIndex(meshRenderingInfo.materialIndex);
        if (!material) continue;

        RenderItem renderItem;
        renderItem.mousePickType = (int)ObjectType::GAME_OBJECT;
        renderItem.mousePickIndex = m_mousePickIndex;
        renderItem.modelMatrix = GetModelMatrix();
        renderItem.inverseModelMatrix = glm::inverse(renderItem.modelMatrix);
        renderItem.meshIndex = meshRenderingInfo.meshIndex;
        renderItem.baseColorTextureIndex = material->m_basecolor;
        renderItem.normalMapTextureIndex = material->m_normal;
        renderItem.rmaTextureIndex = material->m_rma;

        // get me out of here
        Util::UpdateRenderItemAABB(renderItem);
        // get me out of here

        BlendingMode blendingMode = meshRenderingInfo.blendingMode;
        switch (blendingMode) {
            case BlendingMode::NONE: m_renderItems.push_back(renderItem); break;
            case BlendingMode::BLENDED: m_renderItemsBlended.push_back(renderItem); break;
            case BlendingMode::ALPHA_DISCARDED: m_renderItemsAlphaDiscarded.push_back(renderItem); break;
            case BlendingMode::HAIR_TOP_LAYER: m_renderItemsHairTopLayer.push_back(renderItem); break;
            case BlendingMode::HAIR_UNDER_LAYER: m_renderItemsHairBottomLayer.push_back(renderItem); break;
            default: break;
        }
    }
}

void GameObject::SetConvexHullsFromModel(const std::string modelName) {
    Model* model = AssetManager::GetModelByName(modelName);
    if (!model) return;

    PhysicsFilterData filterData;
    filterData.raycastGroup = RaycastGroup::RAYCAST_ENABLED;
    filterData.collisionGroup = CollisionGroup::ENVIROMENT_OBSTACLE;
    filterData.collidesWith = CollisionGroup::PLAYER;

    PxFilterData pxFilterData;
    pxFilterData.word0 = (PxU32)filterData.raycastGroup;
    pxFilterData.word1 = (PxU32)filterData.collisionGroup;
    pxFilterData.word2 = (PxU32)filterData.collidesWith;

    //PxRigidDynamic* pxRigidDynamic = m_rigidDynamic.GetPxRigidDynamic();

    // Create rigid dynamic
    PxPhysics* pxPhysics = Physics::GetPxPhysics();
    PxScene* pxScene = Physics::GetPxScene();

    

    PxQuat quat = Physics::GlmQuatToPxQuat(glm::quat(m_transform.rotation));
    PxTransform pxTransform = PxTransform(PxVec3(m_transform.position.x, m_transform.position.y, m_transform.position.z), quat);
    PxRigidDynamic* pxRigidDynamic = pxPhysics->createRigidDynamic(pxTransform);
    //pxRigidDynamic->attachShape(*pxShape);
    //PxRigidBodyExt::updateMassAndInertia(*pxRigidDynamic, density);
    pxScene->addActor(*pxRigidDynamic);


    for (uint32_t meshIndex : model->GetMeshIndices()) {
        Mesh* mesh = AssetManager::GetMeshByIndex(meshIndex);

        std::span<Vertex> vertices = AssetManager::GetVerticesSpan(mesh->baseVertex, mesh->vertexCount);
        PxShape* pxShape = Physics::CreateConvexShapeFromVertexList(vertices);

        pxShape->setQueryFilterData(pxFilterData);       // ray casts
        pxShape->setSimulationFilterData(pxFilterData);  // collisions
        //collisionShapes.push_back(shape);

        pxRigidDynamic->attachShape(*pxShape);
    }
}

glm::vec3 GameObject::GetPosition() const {
    return m_transform.position;
}

glm::vec3 GameObject::GetEulerRotation() const {
    return m_transform.rotation;
}

glm::vec3 GameObject::GetScale() const {
    return m_transform.scale;
}

const glm::mat4 GameObject::GetModelMatrix() {
    return m_transform.to_mat4();
}

const glm::vec3 GameObject::GetObjectCenter() {
    glm::vec3 aabbCenter = (m_model->m_aabbMin + m_model->m_aabbMax) * 0.5f;
    return GetModelMatrix() * glm::vec4(aabbCenter, 1.0f);
}

const glm::vec3 GameObject::GetObjectCenterOffsetFromOrigin() {
    return GetObjectCenter() - glm::vec3(GetModelMatrix()[3]);
}

const std::vector<RenderItem>& GameObject::GetRenderItems() {
    return m_renderItems;
}

const std::vector<RenderItem>& GameObject::GetRenderItemsBlended() {
    return m_renderItemsBlended;
}

const std::vector<RenderItem>& GameObject::GetRenderItemsAlphaDiscarded() {
    return m_renderItemsAlphaDiscarded;
}

const std::vector<RenderItem>& GameObject::GetRenderItemsHairTopLayer() {
    return m_renderItemsHairTopLayer;
}

const std::vector<RenderItem>& GameObject::GetRenderItemsHairBottomLayer() {
    return m_renderItemsHairBottomLayer;
}
