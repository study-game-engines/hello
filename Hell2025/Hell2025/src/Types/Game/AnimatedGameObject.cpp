#include "AnimatedGameObject.h"
#include "AssetManagement/AssetManager.h"
#include "Core/Game.h"
#include "Input/Input.h"
#include "Physics/Physics.h"
#include "Renderer/Renderer.h"
#include "Renderer/RenderDataManager.h"
#include "UniqueID.h"
#include "Util.h"

#include <unordered_set>

void AnimatedGameObject::Init() {
    m_objectId = UniqueID::GetNext();
}

void AnimatedGameObject::SetMeshWoundMaskTextureIndex(const std::string& meshName, int32_t woundMaskTextureIndex) {
    std::vector<uint32_t>& meshIndices = m_skinnedModel->GetMeshIndices();

    for (int i = 0; i < meshIndices.size(); i++) {
        uint32_t meshIndex = meshIndices[i];
        SkinnedMesh* skinnedMesh = AssetManager::GetSkinnedMeshByIndex(meshIndex);
        if (skinnedMesh && skinnedMesh->name == meshName) {
            m_woundMaskTextureIndices[i] = woundMaskTextureIndex;
            return;
        }
    }
}

void AnimatedGameObject::SetRagdoll(const std::string& ragdollName, float ragdollTotalWeight) {
    m_ragdollId = Physics::CreateRagdollByName(ragdollName, ragdollTotalWeight);
}

const size_t AnimatedGameObject::GetAnimatedTransformCount() {
    return m_animatedTransforms.local.size();
}

void AnimatedGameObject::UpdateRenderItems() {
    if (!m_skinnedModel) return;

    int meshCount = m_meshRenderingEntries.size();
    m_renderItems.clear();
    for (int i = 0; i < meshCount; i++) {
        if (m_meshRenderingEntries[i].drawingEnabled) {
            RenderItem& renderItem = m_renderItems.emplace_back();
            SkinnedMesh* mesh = AssetManager::GetSkinnedMeshByIndex(m_meshRenderingEntries[i].meshIndex);
            Material* material = AssetManager::GetMaterialByIndex(m_meshRenderingEntries[i].materialIndex);
            renderItem.baseColorTextureIndex = material->m_basecolor;
            renderItem.normalMapTextureIndex = material->m_normal;
            renderItem.rmaTextureIndex = material->m_rma;
            renderItem.modelMatrix = GetModelMatrix();
            renderItem.inverseModelMatrix = glm::inverse(GetModelMatrix());
            renderItem.meshIndex = m_skinnedModel->GetMeshIndices()[i];
            renderItem.ignoredViewportIndex = m_ignoredViewportIndex;
            renderItem.exclusiveViewportIndex = m_exclusiveViewportIndex;
            renderItem.baseSkinnedVertex = RenderDataManager::GetBaseSkinnedVertex() + mesh->baseVertexLocal;
            renderItem.furLength = m_meshRenderingEntries[i].furLength;
            renderItem.furUVScale = m_meshRenderingEntries[i].furUVScale;
            renderItem.furShellDistanceAttenuation = m_meshRenderingEntries[i].furShellDistanceAttenuation;
            renderItem.woundMaskTexutreIndex = m_woundMaskTextureIndices[i];
            renderItem.blockScreenSpaceBloodDecals = (int)true;

            if (m_skinnedModel->GetName() == "Kangaroo") {
                renderItem.customFlag = 1;
            }
        }
    }
    RenderDataManager::IncrementBaseSkinnedVertex(m_skinnedModel->GetVertexCount());
}

std::vector<RenderItem>& AnimatedGameObject::GetRenderItems() {
    return m_renderItems;
}

const uint32_t AnimatedGameObject::GetVerteXCount() {
    if (m_skinnedModel) {
        return m_skinnedModel->GetVertexCount();
    }
    else {
        return 0;
    }
}

float AnimatedGameObject::GetBlendFactor() {
    return m_blendFactor;
}

void AnimatedGameObject::UpdateBoneTransformsFromRagdoll() {
    Ragdoll* ragdoll = Physics::GetRagdollById(m_ragdollId);
    if (!ragdoll) return;
    if (!m_skinnedModel) return;

    int nodeCount = m_skinnedModel->m_nodes.size();
    m_globalBlendedNodeTransforms.resize(nodeCount);

    SkinnedModel* skinnedModel = AssetManager::GetSkinnedModelByIndex(m_skinnedModelIndex);

    for (int i = 0; i < skinnedModel->m_nodes.size(); i++) {
        std::string NodeName = skinnedModel->m_nodes[i].name;
        glm::mat4 nodeTransformation = glm::mat4(1);
        nodeTransformation = skinnedModel->m_nodes[i].inverseBindTransform;
        unsigned int parentIndex = skinnedModel->m_nodes[i].parentIndex;
        glm::mat4 ParentTransformation = (parentIndex == -1) ? glm::mat4(1) : m_globalBlendedNodeTransforms[parentIndex];
        glm::mat4 GlobalTransformation = ParentTransformation * nodeTransformation;

        for (int j = 0; j < ragdoll->m_correspondingBoneNames.size(); j++) {
            if (ragdoll->m_correspondingBoneNames[j] == NodeName) {
                RigidDynamic* rigidDynamic = Physics::GetRigidDynamicById(ragdoll->m_rigidDynamicIds[j]);
                PxRigidDynamic* pxRigidDynamic = rigidDynamic->GetPxRigidDynamic();
                GlobalTransformation = Physics::PxMat44ToGlmMat4(pxRigidDynamic->getGlobalPose());
            }
        }

        m_animationLayer.m_globalBlendedNodeTransforms[i] = GlobalTransformation;             // one of these is overwritten by the other later, forget which atm
        m_globalBlendedNodeTransforms[i] = AnimatedTransform(GlobalTransformation).to_mat4(); // one of these is overwritten by the other later, forget which atm
    }
}

void AnimatedGameObject::Update(float deltaTime, std::unordered_map<std::string, glm::mat4> additiveBoneTransforms) {
    if (!m_skinnedModel) return;

    if (m_animationMode == AnimationMode::RAGDOLL) {
        UpdateBoneTransformsFromRagdoll();
    }
    else {
        if (m_animationMode == AnimationMode::BINDPOSE) {
            m_animationLayer.ClearAllAnimationStates();
        }

        m_animationLayer.Update(deltaTime, additiveBoneTransforms);
    }

    m_LocalBlendedBoneTransforms.clear();
    m_globalBlendedNodeTransforms.clear();

    for (int i = 0; i < m_skinnedModel->GetBoneCount(); i++) {
        m_LocalBlendedBoneTransforms.push_back(glm::mat4(1));
    }

    for (int i = 0; i < m_animationLayer.m_globalBlendedNodeTransforms.size(); i++) {
        m_globalBlendedNodeTransforms.push_back(m_animationLayer.m_globalBlendedNodeTransforms[i]);
    }
        
    // OVERWRITE WITH ANIMATOR
    if (m_skinnedModel->GetName() == "Kangaroo") {
        if (m_animator.m_animationStates.empty()) {
            m_animator.SetSkinnedModel("Kangaroo");
            m_animator.RegisterAnimation("Kangaroo_Hop");
            m_animator.RegisterAnimation("Kangaroo_Bite_UpperBody");

            int nodeCount = m_skinnedModel->GetNodeCount();
            std::vector<float> boneWeightsLowerBody(nodeCount);
            std::vector<float> boneWeightsUpperBody(nodeCount);

            std::unordered_set<std::string> upperBones = {
                "RootNode",
                "Armature" ,
                //"def_root" ,
                //"def_hip" ,
                "def_spine_01" ,
                "def_spine_02" ,
                "def_chest" ,
                "def_neck_01" ,
                "def_neck_02" ,
                "def_head" ,
                "def_ear_01_L" ,
                "def_ear_02_L" ,
                "def_ear_03_L" ,
                "def_ear_04_L" ,
                "def_jaw" ,
                "def_lower_lip_L" ,
                "def_lower_lip_C" ,
                "def_lower_lip_R" ,
                "def_tongue_01" ,
                "def_tongue_05" ,
                "def_tongue_03" ,
                "def_tongue_07" ,
                "def_tongue_02" ,
                "def_tongue_06" ,
                "def_tongue_04" ,
                "def_tongue_08" ,
                "def_tongue_09" ,
                "def_brow_L" ,
                "def_eye_L" ,
                "def_snout" ,
                "def_upper_lip_L" ,
                "def_upper_lip_C" ,
                "def_brow_R" ,
                "def_eye_R" ,
                "def_upper_lip_R" ,
                "def_ear_01_R" ,
                "def_ear_02_R" ,
                "def_ear_03_R" ,
                "def_ear_04_R" ,
                "def_upper_lid_L" ,
                "def_upper_lid_tweak_L" ,
                "def_lower_lid_L" ,
                "def_lower_lid_tweak_L" ,
                "def_upper_lid_R" ,
                "def_upper_lid_tweak_R" ,
                "def_lower_lid_R" ,
                "def_lower_lid_tweak_R" ,
                "def_corner_mouth_L" ,
                "def_corner_mouth_R" ,
                "def_collar_L" ,
                "def_upper_arm_01_L" ,
                "def_upper_arm_02_L" ,
                "def_upper_arm_03_L" ,
                "def_upper_arm_04_L" ,
                "def_upper_arm_05_L" ,
                "def_lower_arm_01_L" ,
                "def_lower_arm_02_L" ,
                "def_lower_arm_03_L" ,
                "def_lower_arm_04_L" ,
                "def_lower_arm_05_L" ,
                "def_hand_L" ,
                "def_pinky_finger_01_L" ,
                "def_pinky_finger_02_L" ,
                "def_pinky_finger_03_L" ,
                "def_carpol_01_L" ,
                "def_pointer_finger_01_L" ,
                "def_pointer_finger_02_L" ,
                "def_pointer_finger_03_L" ,
                "def_carpol_02_L" ,
                "def_middle_finger_01_L" ,
                "def_middle_finger_02_L" ,
                "def_middle_finger_03_L" ,
                "def_carpol_03_L" ,
                "def_ring_finger_01_L" ,
                "def_ring_finger_02_L" ,
                "def_ring_finger_03_L" ,
                "def_carpol_04_L" ,
                "def_carpol_05_L" ,
                "def_thumb_01_L" ,
                "def_thumb_02_L" ,
                "def_thumb_03_L" ,
                "def_pit_correction_L" ,
                "def_collar_R" ,
                "def_pit_correction_R" ,
                "def_upper_arm_01_R" ,
                "def_upper_arm_02_R" ,
                "def_upper_arm_03_R" ,
                "def_upper_arm_04_R" ,
                "def_upper_arm_05_R" ,
                "def_lower_arm_01_R" ,
                "def_lower_arm_02_R" ,
                "def_lower_arm_03_R" ,
                "def_lower_arm_04_R" ,
                "def_lower_arm_05_R" ,
                "def_hand_R" ,
                "def_pinky_finger_01_R" ,
                "def_pinky_finger_02_R" ,
                "def_pinky_finger_03_R" ,
                "def_carpol_01_R" ,
                "def_pointer_finger_01_R" ,
                "def_pointer_finger_02_R" ,
                "def_pointer_finger_03_R" ,
                "def_carpol_02_R" ,
                "def_middle_finger_01_R" ,
                "def_middle_finger_02_R" ,
                "def_middle_finger_03_R" ,
                "def_carpol_03_R" ,
                "def_ring_finger_01_R" ,
                "def_ring_finger_02_R" ,
                "def_ring_finger_03_R" ,
                "def_carpol_04_R" ,
                "def_carpol_05_R" ,
                "def_thumb_01_R" ,
                "def_thumb_02_R" ,
                "def_thumb_03_R" ,
                //"def_breathing" ,
                //"def_tail_01" ,
                //"def_tail_02" ,
                //"def_tail_03" ,
                //"def_tail_04" ,
                //"def_tail_05" ,
                //"def_tail_06" ,
                //"def_tail_07" ,
                //"def_tail_08" ,
                //"def_tail_09" ,
                //"def_trail_10" ,
                //"def_trail_11" ,
                //"def_upper_leg_L" ,
                //"def_lower_leg_L" ,
                //"def_foot_L" ,
                //"def_toe_right_01_L" ,
                //"def_toe_right_02_L" ,
                //"def_toe_left_01_L" ,
                //"def_toe_left_02_L" ,
                //"def_toe_middle_01_L" ,
                //"def_toe_middle_02_L" ,
                //"def_upper_leg_R" ,
                //"def_lower_leg_R" ,
                //"def_foot_R" ,
                //"def_toe_right_01_R" ,
                //"def_toe_right_02_R" ,
                //"def_toe_left_01_R" ,
                //"def_toe_left_02_R" ,
                //"def_toe_middle_01_R" ,
                //"def_toe_middle_02_R" ,
                "Body" ,
                "LeftEye_Iris" ,
                "LeftEye_Sclera" ,
                "Mouth" ,
                "Nails" ,
                "RightEye_Iris" ,
                "RightEye_Sclera",
                "Teeth",
                "Tongue"
            };

            for (int i = 0; i < nodeCount; ++i) {
                const std::string& name = m_skinnedModel->m_nodes[i].name;
                bool isUpper = upperBones.count(name) > 0;

                boneWeightsUpperBody[i] = isUpper ? 10.0f : 0.01f;
                boneWeightsLowerBody[i] = isUpper ? 0.01f : 10.0f;
            }

            m_animator.m_animationStates[0].m_boneWeights = boneWeightsLowerBody;
            m_animator.m_animationStates[1].m_boneWeights = boneWeightsUpperBody;
        }

        m_animator.m_animationStates[0].m_AnimationWeight = 1.0;
        m_animator.m_animationStates[1].m_AnimationWeight = 1.0;

        m_animator.UpdateAnimations(deltaTime);

        m_globalBlendedNodeTransforms = m_animator.m_globalBlendedNodeTransforms;

    }

    // Compute local bone matrices
    int boneCount = m_skinnedModel->GetBoneCount();
    m_LocalBlendedBoneTransforms.resize(boneCount);
    for (int b = 0; b < boneCount; ++b) {
        int nodeIdx = m_skinnedModel->m_boneNodeIndex[b];
        m_LocalBlendedBoneTransforms[b] = m_globalBlendedNodeTransforms[nodeIdx] * m_skinnedModel->m_boneOffsets[b];
    }



    // If it has a ragdoll
    if (m_animationMode != AnimationMode::RAGDOLL) {
        Ragdoll* ragdoll = Physics::GetRagdollById(m_ragdollId);
        if (ragdoll) {
            ragdoll->SetRigidGlobalPosesFromAnimatedGameObject(this);
        }
    }

    Ragdoll* ragdoll = Physics::GetRagdollById(m_ragdollId);
    if (ragdoll && false) {
        for (int i = 0; i < ragdoll->m_components.joints.size(); i++) {

            JointComponent& joint = ragdoll->m_components.joints[i];
            D6Joint* d6Joint = Physics::GetD6JointById(ragdoll->m_d6JointIds[i]);
            PxD6Joint* pxD6Joint = d6Joint->GetPxD6Joint();

            // Linear spring
            joint.limit_linearStiffness = 10000;
            joint.limit_linearDampening = 1000000;
            joint.drive_angularStiffness = 10000;
            joint.drive_angularDamping = 1000000;

            const PxSpring linearSpring = PxSpring(joint.limit_linearStiffness, joint.limit_linearDampening);

            if (joint.limit.x > -1) {
                const PxJointLinearLimitPair limitX = PxJointLinearLimitPair(-joint.limit.x, joint.limit.x, linearSpring);
                pxD6Joint->setLinearLimit(PxD6Axis::eX, limitX);
            }

            if (joint.limit.y > -1) {
                const PxJointLinearLimitPair limitY = PxJointLinearLimitPair(-joint.limit.y, joint.limit.y, linearSpring);
                pxD6Joint->setLinearLimit(PxD6Axis::eY, limitY);
            }

            if (joint.limit.z > -1) {
                const PxJointLinearLimitPair limitZ = PxJointLinearLimitPair(-joint.limit.z, joint.limit.z, linearSpring);
                pxD6Joint->setLinearLimit(PxD6Axis::eZ, limitZ);
            }

            const PxSpring angularSpring = PxSpring(joint.drive_angularStiffness, joint.drive_angularDamping);
            const PxJointAngularLimitPair twistLimit = PxJointAngularLimitPair(-joint.twist, joint.twist, angularSpring);
            const PxJointLimitCone swingLimit = PxJointLimitCone(joint.swing1, joint.swing2, angularSpring);

            pxD6Joint->setTwistLimit(twistLimit);
            pxD6Joint->setSwingLimit(swingLimit);
        }

    }
}

void AnimatedGameObject::CleanUp() {
    if (m_ragdollId != 0) {
        //std::cout << "AnimatedGameObject::CleanUp() ragdollId: " << m_ragdollId << "\n";
        Physics::MarkRagdollForRemoval(m_ragdollId);
    }
}

void AnimatedGameObject::SetMeshMaterialByMeshName(const std::string& meshName, const std::string& materialName) {
    if (!m_skinnedModel) {
        return;
    }
    for (MeshRenderingEntry& meshRenderingEntry : m_meshRenderingEntries) {
        if (meshRenderingEntry.meshName == meshName) {
            meshRenderingEntry.materialIndex = AssetManager::GetMaterialIndexByName(materialName);
        }
    }
}

void AnimatedGameObject::SetMeshFurLength(const std::string& meshName, float furLength) {
    if (!m_skinnedModel) {
        return;
    }
    for (MeshRenderingEntry& meshRenderingEntry : m_meshRenderingEntries) {
        if (meshRenderingEntry.meshName == meshName) {
            meshRenderingEntry.furLength = furLength;
        }
    }
}

void AnimatedGameObject::SetMeshFurUVScale(const std::string& meshName, float uvScale) {
    if (!m_skinnedModel) {
        return;
    }
    for (MeshRenderingEntry& meshRenderingEntry : m_meshRenderingEntries) {
        if (meshRenderingEntry.meshName == meshName) {
            meshRenderingEntry.furUVScale = uvScale;
        }
    }
}

void AnimatedGameObject::SetMeshFurShellDistanceAttenuation(const std::string& meshName, float furShellDistanceAttenuation) {
    if (!m_skinnedModel) {
        return;
    }
    for (MeshRenderingEntry& meshRenderingEntry : m_meshRenderingEntries) {
        if (meshRenderingEntry.meshName == meshName) {
            meshRenderingEntry.furShellDistanceAttenuation = furShellDistanceAttenuation;
        }
    }
}

void AnimatedGameObject::SetMeshMaterialByMeshIndex(int meshIndex, const std::string& materialName) {
    if (!m_skinnedModel) {
        return;
    }
    if (meshIndex >= 0 && meshIndex < m_meshRenderingEntries.size()) {
        m_meshRenderingEntries[meshIndex].materialIndex = AssetManager::GetMaterialIndexByName(materialName);
    }
}

void AnimatedGameObject::SetMeshToRenderAsGlassByMeshIndex(const std::string& meshName) {
    if (!m_skinnedModel) {
        return;
    }
    for (MeshRenderingEntry& meshRenderingEntry : m_meshRenderingEntries) {
        if (meshRenderingEntry.meshName == meshName) {
            meshRenderingEntry.renderAsGlass = true;
        }
    }
}

void AnimatedGameObject::SetMeshEmissiveColorTextureByMeshName(const std::string& meshName, const std::string& textureName) {
    if (!m_skinnedModel) {
        return;
    }
    for (MeshRenderingEntry& meshRenderingEntry : m_meshRenderingEntries) {
        if (meshRenderingEntry.meshName == meshName) {
            meshRenderingEntry.emissiveColorTexutreIndex = AssetManager::GetTextureIndexByName(textureName);
        }
    }
}

void AnimatedGameObject::EnableBlendingByMeshIndex(int meshIndex) {
    if (!m_skinnedModel) {
        return;
    }
    if (meshIndex >= 0 && meshIndex < m_meshRenderingEntries.size()) {
        m_meshRenderingEntries[meshIndex].blendingEnabled = true;
    }
}

void AnimatedGameObject::SetAllMeshMaterials(const std::string& materialName) {
    if (!m_skinnedModel) {
        return;
    }
    for (MeshRenderingEntry& meshRenderingEntry : m_meshRenderingEntries) {
        meshRenderingEntry.materialIndex =AssetManager::GetMaterialIndexByName(materialName);
    }
}

glm::mat4 AnimatedGameObject::GetBindPoseByBoneName(const std::string& name) {
    for (int i = 0; i < m_skinnedModel->m_nodes.size(); i++) {
        if (m_skinnedModel->m_nodes[i].name == name) {
            return m_skinnedModel->m_nodes[i].inverseBindTransform;
        }
    }
    std::cout << "GetBindPoseByBoneName() failed to find name " << name << "\n";
    return glm::mat4();
}

void AnimatedGameObject::SetAnimationModeToBindPose() {
    m_animationMode = AnimationMode::BINDPOSE;
    m_animationLayer.ClearAllAnimationStates();
}

void AnimatedGameObject::SetAnimationModeToRagdoll() {
    Ragdoll* ragdoll = Physics::GetRagdollById(m_ragdollId);
    if (!ragdoll) return;

    if (m_animationMode != AnimationMode::RAGDOLL) {
        m_animationMode = AnimationMode::RAGDOLL;
        m_animationLayer.ClearAllAnimationStates();
        ragdoll->ActivatePhysics();
    }
}

void AnimatedGameObject::PlayAnimation(const std::string& animationName, float speed) {
    AnimationPlaybackParams params = AnimationPlaybackParams::GetDefaultPararms();
    params.animationSpeed = speed;
    m_animationMode = AnimationMode::ANIMATION;
    m_animationLayer.PlayAnimation(animationName, params);
}

void AnimatedGameObject::PlayAnimation(const std::string& animationName, const AnimationPlaybackParams& playbackParams) {
    m_animationMode = AnimationMode::ANIMATION;
    m_animationLayer.PlayAnimation(animationName, playbackParams);
}

void AnimatedGameObject::PlayAnimation(const std::vector<std::string>& animationNames, float speed) {
    AnimationPlaybackParams params = AnimationPlaybackParams::GetDefaultPararms();
    params.animationSpeed = speed;
    int rand = std::rand() % animationNames.size();
    m_animationLayer.PlayAnimation(animationNames[rand], params);
}

void AnimatedGameObject::PlayAnimation(const std::vector<std::string>& animationNames, const AnimationPlaybackParams& playbackParams) {
    int rand = std::rand() % animationNames.size();
    m_animationLayer.PlayAnimation(animationNames[rand], playbackParams);
}

void AnimatedGameObject::PlayAndLoopAnimation(const std::string& animationName, const AnimationPlaybackParams& playbackParams) {
    m_animationMode = AnimationMode::ANIMATION;
    m_animationLayer.PlayAndLoopAnimation(animationName, playbackParams);
}

void AnimatedGameObject::PlayAndLoopAnimation(const std::string& animationName, float speed) {
    AnimationPlaybackParams params = AnimationPlaybackParams::GetDefaultPararms();
    params.animationSpeed = speed;
    m_animationMode = AnimationMode::ANIMATION;
    m_animationLayer.PlayAndLoopAnimation(animationName, params);
}

void AnimatedGameObject::PlayAnimation(const std::string& animationName, float speed, unsigned int layer) {
  
}

void AnimatedGameObject::PlayAnimation(std::vector<const std::string&> animationNames, float speed, unsigned int layer) {

}

void AnimatedGameObject::PlayAndLoopAnimation(const std::string& animationName, float speed, unsigned int layer) {

}

void AnimatedGameObject::PlayAndLoopAnimation(std::vector<const std::string&> animationNames, float speed, unsigned int layer) {

}

std::vector<glm::mat4>& AnimatedGameObject::GetLocalBlendedBoneTransforms() {
    return m_LocalBlendedBoneTransforms;
}

std::vector<glm::mat4>& AnimatedGameObject::GetGlobalBlendedNodeTransforms() {
    return m_globalBlendedNodeTransforms;
}

const glm::mat4 AnimatedGameObject::GetModelMatrix() {

    if (useCameraMatrix) {
        return m_cameraMatrix;
    }

    if (m_animationMode == AnimationMode::RAGDOLL) {
        return glm::mat4(1);
    }
    else {
        return _transform.to_mat4();
    }
}

bool AnimatedGameObject::IsAnimationComplete() {
    if (m_animationLayer.m_animationStates.size()) {
        return m_animationLayer.m_animationStates[0].IsComplete();
    }
    return true;
}

std::string AnimatedGameObject::GetName() {
    return m_name;
}

void AnimatedGameObject::SetName(std::string name) {
    m_name = name;
}

void AnimatedGameObject::SetSkinnedModel(std::string name) {
    SkinnedModel* ptr = AssetManager::GetSkinnedModelByName(name);
    if (ptr) {
        m_skinnedModelIndex = AssetManager::GetSkinnedModelIndexByName(name);
        m_animationLayer.SetSkinnedModel(name);
        m_skinnedModel = ptr;
        m_meshRenderingEntries.clear();
        m_woundMaskTextureIndices.resize(m_skinnedModel->GetMeshCount());

        int meshCount = m_skinnedModel->GetMeshCount();

        for (int i = 0; i < meshCount; i++) {
            SkinnedMesh* skinnedMesh = AssetManager::GetSkinnedMeshByIndex(m_skinnedModel->GetMeshIndices()[i]);
            MeshRenderingEntry& meshRenderingEntry = m_meshRenderingEntries.emplace_back();
            meshRenderingEntry.meshName = skinnedMesh->name;
            meshRenderingEntry.meshIndex = m_skinnedModel->GetMeshIndices()[i];
        }

        // Store bone indices
        m_boneMapping.clear();
        for (int i = 0; i < m_skinnedModel->m_nodes.size(); i++) {
            m_boneMapping[m_skinnedModel->m_nodes[i].name] = i;
        }
    }
    else {
        std::cout << "Could not SetSkinnedModel(name) with name: \"" << name << "\", it does not exist\n";
    }
}

glm::mat4 AnimatedGameObject::GetAnimatedTransformByBoneName(const std::string& name) {
    int index = m_boneMapping[name];
    if (index >= 0 && index < m_animationLayer.m_globalBlendedNodeTransforms.size()) {
        return m_animationLayer.m_globalBlendedNodeTransforms[index];
    }
    //std::cout << "AnimatedGameObject::GetAnimatedTransformByBoneName() FAILED, no matching bone name: " << name << "\n";
    return glm::mat4(1);
}


glm::vec3 AnimatedGameObject::GetScale() {
    return  _transform.scale;
}

void AnimatedGameObject::SetScale(float scale) {
    _transform.scale = glm::vec3(scale);
}
void AnimatedGameObject::SetPosition(glm::vec3 position) {
    _transform.position = position;
}

void AnimatedGameObject::SetRotationX(float rotation) {
    _transform.rotation.x = rotation;
}

void AnimatedGameObject::SetRotationY(float rotation) {
    _transform.rotation.y = rotation;
}

void AnimatedGameObject::SetRotationZ(float rotation) {
    _transform.rotation.z = rotation;
}

void AnimatedGameObject::EnableDrawingForAllMesh() {
    for (MeshRenderingEntry& meshRenderingEntry : m_meshRenderingEntries) {
        meshRenderingEntry.drawingEnabled = true;
    }
}

void AnimatedGameObject::EnableDrawingForMeshByMeshName(const std::string& meshName) {
    for (MeshRenderingEntry& meshRenderingEntry : m_meshRenderingEntries) {
        if (meshRenderingEntry.meshName == meshName) {
            meshRenderingEntry.drawingEnabled = true;
            return;
        }
    }
    //std::cout << "DisableDrawingForMeshByMeshName() called but name " << meshName << " was not found!\n";
}

void AnimatedGameObject::DisableDrawingForMeshByMeshName(const std::string& meshName) {
    for (MeshRenderingEntry& meshRenderingEntry : m_meshRenderingEntries) {
        if (meshRenderingEntry.meshName == meshName) {
            meshRenderingEntry.drawingEnabled = false;
            return;
        }
    }
    //std::cout << "DisableDrawingForMeshByMeshName() called but name " << meshName << " was not found!\n";
}

void AnimatedGameObject::PrintBoneNames() {
    std::cout << m_skinnedModel->GetName() << "\n";
    for (int i = 0; i < m_skinnedModel->m_nodes.size(); i++) {
        std::cout << "-" << i << " " << m_skinnedModel->m_nodes[i].name << "\n";
    }
}

void AnimatedGameObject::PrintMeshNames() {
    std::cout << m_skinnedModel->GetName() << "\n";
    for (int i = 0; i < m_meshRenderingEntries.size(); i++) {
        std::cout << "-" << i << " " << m_meshRenderingEntries[i].meshName << "\n";
    }
}

uint32_t AnimatedGameObject::GetAnimationFrameNumber() {
    if (m_animationLayer.m_animationStates.size()) {
        return m_animationLayer.m_animationStates[0].GetAnimationFrameNumber();
    }
    return 0;
}

bool AnimatedGameObject::AnimationIsPastFrameNumber(int frameNumber) {
    return frameNumber < GetAnimationFrameNumber();
}

void AnimatedGameObject::DrawBones(int exclusiveViewportIndex) {
    SkinnedModel* skinnedModel = AssetManager::GetSkinnedModelByIndex(m_skinnedModelIndex);
    if (!skinnedModel) return;
    
    // Traverse the tree
    for (int i = 0; i < skinnedModel->m_nodes.size(); i++) {
        glm::mat4 nodeTransformation = glm::mat4(1);
        unsigned int parentIndex = skinnedModel->m_nodes[i].parentIndex;
        std::string& nodeName = skinnedModel->m_nodes[i].name;
        std::string& parentNodeName = skinnedModel->m_nodes[parentIndex].name;

        if (parentIndex != -1 && skinnedModel->BoneExists(nodeName) && skinnedModel->BoneExists(parentNodeName)) {
            const glm::mat4& boneWorldMatrix = m_animationLayer.m_globalBlendedNodeTransforms[i];
            const glm::mat4& parentBoneWorldMatrix = m_animationLayer.m_globalBlendedNodeTransforms[parentIndex];
            glm::vec3 position = GetModelMatrix() * boneWorldMatrix * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
            glm::vec3 parentPosition = GetModelMatrix() * parentBoneWorldMatrix * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
            Renderer::DrawPoint(position, OUTLINE_COLOR, false, exclusiveViewportIndex);
            Renderer::DrawLine(position, parentPosition, WHITE, false, exclusiveViewportIndex);
        }
    }

    // // To draw all nodes
    // for (const glm::mat4& boneWorldMatrix : m_animationLayer.m_globalBlendedNodeTransforms) {
    //     glm::vec3 position = GetModelMatrix() * boneWorldMatrix * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
    //     Renderer::DrawPoint(position, color, false, exclusiveViewportIndex);
    // }
}

void AnimatedGameObject::DrawBoneTangentVectors(float size, int exclusiveViewportIndex) {
    for (const glm::mat4& boneWorldMatrix : m_animationLayer.m_globalBlendedNodeTransforms) {
        glm::vec3 origin = GetModelMatrix() * boneWorldMatrix * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
        glm::vec3 right = glm::normalize(glm::vec3(boneWorldMatrix[0]));
        glm::vec3 up = glm::normalize(glm::vec3(boneWorldMatrix[1]));
        glm::vec3 forward = glm::normalize(glm::vec3(boneWorldMatrix[2]));
        Renderer::DrawLine(origin, origin + (forward * size), BLUE, false, exclusiveViewportIndex);
        Renderer::DrawLine(origin, origin + (up * size), GREEN, false, exclusiveViewportIndex);
        Renderer::DrawLine(origin, origin + (right * size), RED, false, exclusiveViewportIndex);
    }
}

void AnimatedGameObject::SetExclusiveViewportIndex(int index) {
    m_exclusiveViewportIndex = index;
}

void AnimatedGameObject::SetIgnoredViewportIndex(int index) {
    m_ignoredViewportIndex = index;
}

bool AnimatedGameObject::AnimationByNameIsComplete(const std::string& name) {
    for (AnimationStateOld& AnimationState : m_animationLayer.m_animationStates) {
        int animationIndex = AssetManager::GetAnimationIndexByName(name);
        if (AnimationState.m_index == animationIndex) {
            return AnimationState.IsComplete();
        }
    }
    return true;
}

int AnimatedGameObject::GetBoneIndex(const std::string& boneName) {
    auto it = m_boneMapping.find(boneName);
    return (it != m_boneMapping.end()) ? it->second : -1;
}

glm::mat4 AnimatedGameObject::GetBoneWorldMatrix(const std::string& boneName) {
    int boneIndex = GetBoneIndex(boneName);
    if (boneIndex == -1 || m_animationLayer.m_globalBlendedNodeTransforms.empty()) {
        return glm::mat4(1.0f);
    }
    else {
        return GetModelMatrix() * m_animationLayer.m_globalBlendedNodeTransforms[boneIndex];
    }
}

glm::vec3 AnimatedGameObject::GetBoneWorldPosition(const std::string& boneName) {
    return GetBoneWorldMatrix(boneName)[3];
}


void AnimatedGameObject::SubmitForSkinning() {
    RenderDataManager::SubmitAnimatedGameObjectForSkinning(this);
}