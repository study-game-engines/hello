#include "Animator.h"
#include "AssetManagement/AssetManager.h"
#include "Util.h"

void Animator::SetSkinnedModel(const std::string& skinnedModelName) {
    m_skinnedModel= AssetManager::GetSkinnedModelByName(skinnedModelName);
}

void Animator::RegisterAnimation(const std::string& animationName) {
    Animation* animation = AssetManager::GetAnimationByName(animationName);
    if (!animation) return;

    // Bail if animation already exists
    for (AnimationState& animationState : m_animationStates) {
        if (animationState.m_animation == animation) {
            return;
        }
    }
    int nodeCount = animation->m_animatedNodes.size();

    AnimationState& animationState = m_animationStates.emplace_back();
    animationState.m_animation = animation;
    animationState.m_globalNodeTransforms.resize(nodeCount);
    animationState.m_boneWeights.resize(nodeCount);
    animationState.m_AnimationWeight = 1.0f;
    animationState.m_animationSpeed = 1.0f;
    animationState.m_currentTime = 0.0f;
    animationState.m_isComplete = false;
    animationState.m_paused = false;
    animationState.m_loop = true;

    // Set default weight for each bone to 1.0 and transform to identity
    for (int i = 0; i < nodeCount; i++) {
        animationState.m_globalNodeTransforms[i] = glm::mat4(1.0f);
        animationState.m_boneWeights[i] = 1.0f;
    }
}


void Animator::UpdateAnimations(float deltaTime) {
    // Bail if invalid skinned model
    if (!m_skinnedModel) return;

    // Compute all animations individually
    for (AnimationState& animationState : m_animationStates) {
        UpdateAnimation(animationState, deltaTime);
    }


    int nodeCount = m_skinnedModel->GetNodeCount();
    std::vector<AnimatedTransform> finalLocals(nodeCount);

    for (int i = 0; i < nodeCount; ++i) {
        glm::mat4 bindLocal = m_skinnedModel->m_nodes[i].inverseBindTransform;
        m_animationStates[0].m_localNodeTransforms[i] = AnimatedTransform(bindLocal);
    }


    /*std::vector<float> weightSum(nodeCount, 0.0f);



    // DEBUG SET ANIMATION STATE 0 TO BINDPOSE
    for (int i = 0; i < nodeCount; ++i) {
        glm::mat4 bindLocal = m_skinnedModel->m_nodes[i].inverseBindTransform;
        //m_animationStates[0].m_localNodeTransforms[i] = AnimatedTransform(bindLocal);
    }

    // Find the first active animation state to initialize our blend.
    int firstActiveStateIndex = -1;
    for (int i = 0; i < m_animationStates.size(); ++i) {
        if (m_animationStates[i].m_AnimationWeight > 1e-5f) {
            firstActiveStateIndex = i;
            break;
        }
    }
    
    // If no animation has weight, use the first animation's pose.
    if (firstActiveStateIndex == -1) {
        for (int i = 0; i < nodeCount; ++i) {
            finalLocals[i] = m_animationStates[0].m_localNodeTransforms[i];
        }
    }
    else {
     // 1) Initialize with the first active state
        for (int i = 0; i < nodeCount; ++i) {
            float w = m_animationStates[firstActiveStateIndex].m_AnimationWeight * m_animationStates[firstActiveStateIndex].m_boneWeights[i];
            weightSum[i] = w;
            finalLocals[i].translation = m_animationStates[firstActiveStateIndex].m_localNodeTransforms[i].translation * w;
            finalLocals[i].scale = m_animationStates[firstActiveStateIndex].m_localNodeTransforms[i].scale * w;
            finalLocals[i].rotation = m_animationStates[firstActiveStateIndex].m_localNodeTransforms[i].rotation * w; // Note: This is still a weighted sum for now
        }

        // 2) Accumulate weighted TRS from the rest of the states
        for (int stateIdx = firstActiveStateIndex + 1; stateIdx < m_animationStates.size(); ++stateIdx) {
            auto& state = m_animationStates[stateIdx];
            if (state.m_AnimationWeight <= 1e-5f) continue;

            for (int i = 0; i < nodeCount; ++i) {
                //float w = state.m_AnimationWeight * state.m_boneWeights[i];
                //if (w <= 1e-5f) continue;

                float w = 0.5f;

                float newWeightSum = weightSum[i] + w;
                float blendFactor = w / newWeightSum;

                // Blend Translation and Scale linearly
                finalLocals[i].translation += state.m_localNodeTransforms[i].translation * w;
                finalLocals[i].scale += state.m_localNodeTransforms[i].scale * w;

                // Blend Rotation using slerp
                glm::quat currentRotation = glm::normalize(finalLocals[i].rotation);
                glm::quat targetRotation = state.m_localNodeTransforms[i].rotation;

                // Ensure we take the shortest path
                if (glm::dot(currentRotation, targetRotation) < 0.0f) {
                    targetRotation = -targetRotation;
                }

                finalLocals[i].rotation = glm::slerp(currentRotation, targetRotation, blendFactor);
                // We need to re-normalize to account for the previous weighted sum in the first active state
                finalLocals[i].rotation = glm::normalize(finalLocals[i].rotation);


                weightSum[i] = newWeightSum;
            }
        }

        // 3) Normalize by total weight
        for (int i = 0; i < nodeCount; ++i) {
            if (weightSum[i] > 1e-5f) {
                float inv = 1.0f / weightSum[i];
                finalLocals[i].translation *= inv;
                finalLocals[i].scale *= inv;
                // The rotation is already blended, just ensure it's normalized.
                finalLocals[i].rotation = glm::normalize(finalLocals[i].rotation);
            }
            else {
             // Fallback to the first animation's transform if weight sum is near zero
                finalLocals[i] = m_animationStates[0].m_localNodeTransforms[i];
            }
        }
    }*/

    finalLocals = m_animationStates[0].m_localNodeTransforms;
    finalLocals = m_animationStates[1].m_localNodeTransforms;

    std::cout << "nodes:" << m_skinnedModel->m_nodes.size() << "\n";
    std::cout << "m_boneOffsets:" << m_skinnedModel->m_boneOffsets.size() << "\n";

    for (int i = 0; i < nodeCount; ++i) {
        glm::mat4 bindLocal = m_skinnedModel->m_nodes[i].inverseBindTransform;

        const std::string name = m_skinnedModel->m_nodes[i].name;
        if (name == "def_neck_01" ||
            name == "def_neck_02" ||
            name == "def_head" ||
            name == "def_ear_01_L" || 
            name == "def_ear_02_L" ||
            name == "def_ear_03_L" ||
            name == "def_ear_04_L" ||
            name == "def_jaw" ||
            name == "def_lower_lip_L" ||
            name == "def_lower_lip_C" ||
            name == "def_lower_lip_R" ||
            name == "def_tongue_01" ||
            name == "def_tongue_05" ||
            name == "def_tongue_03" ||
            name == "def_tongue_07" ||
            name == "def_tongue_02" ||
            name == "def_tongue_06" ||
            name == "def_tongue_04" ||
            name == "def_tongue_08" ||
            name == "def_tongue_09" ||
            name == "def_brow_L" ||
            name == "def_eye_L" ||
            name == "def_snout" ||
            name == "def_upper_lip_L" ||
            name == "def_upper_lip_C" ||
            name == "def_brow_R" ||
            name == "def_eye_R" ||
            name == "def_upper_lip_R" ||
            name == "def_ear_01_R" ||
            name == "def_ear_02_R" ||
            name == "def_ear_03_R" ||
            name == "def_ear_04_R" ||
            name == "def_upper_lid_L" ||
            name == "def_upper_lid_tweak_L" ||
            name == "def_lower_lid_L" ||
            name == "def_lower_lid_tweak_L" ||
            name == "def_upper_lid_R" ||
            name == "def_upper_lid_tweak_R" ||
            name == "def_lower_lid_R" ||
            name == "def_lower_lid_tweak_R" ||
            name == "def_corner_mouth_L" ||
            name == "def_corner_mouth_R" ||
            name == "def_collar_L" ||
            name == "def_upper_arm_01_L" ||
            name == "def_upper_arm_02_L" ||
            name == "def_upper_arm_03_L" ||
            name == "def_upper_arm_04_L" ||
            name == "def_upper_arm_05_L" ||
            name == "def_lower_arm_01_L" ||
            name == "def_lower_arm_02_L" ||
            name == "def_lower_arm_03_L" ||
            name == "def_lower_arm_04_L" ||
            name == "def_lower_arm_05_L" ||
            name == "def_pit_correction_L" ||
            name == "def_collar_R" ||
            name == "def_neck_01" ||
            name == "def_neck_02" ||
            name == "def_head" ||
            name == "def_ear_01_L" ||
            name == "def_ear_02_L" ||
            name == "def_ear_03_L" ||
            name == "def_ear_04_L" ||
            name == "def_jaw" ||
            name == "def_lower_lip_L" ||
            name == "def_lower_lip_C" ||
            name == "def_lower_lip_R" ||
            name == "def_tongue_01" ||
            name == "def_tongue_05" ||
            name == "def_tongue_03" ||
            name == "def_tongue_07" ||
            name == "def_tongue_02" ||
            name == "def_tongue_06" ||
            name == "def_tongue_04" ||
            name == "def_tongue_08" ||
            name == "def_tongue_09" ||
            name == "def_brow_L" ||
            name == "def_eye_L" ||
            name == "def_snout" ||
            name == "def_upper_lip_L" ||
            name == "def_upper_lip_C" ||
            name == "def_brow_R" ||
            name == "def_eye_R" ||
            name == "def_upper_lip_R" ||
            name == "def_ear_01_R" ||
            name == "def_ear_02_R" ||
            name == "def_ear_03_R" ||
            name == "def_ear_04_R" ||
            name == "def_upper_lid_L" ||
            name == "def_upper_lid_tweak_L" ||
            name == "def_lower_lid_L" ||
            name == "def_lower_lid_tweak_L" ||
            name == "def_upper_lid_R" ||
            name == "def_upper_lid_tweak_R" ||
            name == "def_lower_lid_R" ||
            name == "def_lower_lid_tweak_R" ||
            name == "def_corner_mouth_L" ||
            name == "def_corner_mouth_R" ||
            name == "def_collar_L" ||
            name == "def_upper_arm_01_L" ||
            name == "def_upper_arm_02_L" ||
            name == "def_upper_arm_03_L" ||
            name == "def_upper_arm_04_L" ||
            name == "def_upper_arm_05_L" ||
            name == "def_lower_arm_01_L" ||
            name == "def_lower_arm_02_L" ||
            name == "def_lower_arm_03_L" ||
            name == "def_lower_arm_04_L" ||
            name == "def_lower_arm_05_L" ||
            name == "def_pit_correction_L" ||
            name == "def_collar_R" ||
            name == "def_pit_correction_R" ||
            name == "def_upper_arm_01_R" ||
            name == "def_upper_arm_02_R" ||
            name == "def_upper_arm_03_R" ||
            name == "def_upper_arm_04_R" ||
            name == "def_upper_arm_05_R" ||
            name == "def_lower_arm_01_R" ||
            name == "def_lower_arm_02_R" ||
            name == "def_lower_arm_03_R" ||
            name == "def_lower_arm_04_R" ||
            name == "def_lower_arm_05_R" ||
            name == "def_hand_R" ||
            name == "def_pinky_finger_01_R" ||
            name == "def_pinky_finger_02_R" ||
            name == "def_pinky_finger_03_R" ||
            name == "def_carpol_01_R" ||
            name == "def_pointer_finger_01_R" ||
            name == "def_pointer_finger_02_R" ||
            name == "def_pointer_finger_03_R" ||
            name == "def_carpol_02_R" ||
            name == "def_middle_finger_01_R" ||
            name == "def_middle_finger_02_R" ||
            name == "def_middle_finger_03_R" ||
            name == "def_carpol_03_R" ||
            name == "def_ring_finger_01_R" ||
            name == "def_ring_finger_02_R" ||
            name == "def_ring_finger_03_R" ||
            name == "def_carpol_04_R" ||
            name == "def_carpol_05_R" ||
            name == "def_thumb_01_R" ||
            name == "def_thumb_02_R" ||
            name == "def_thumb_03_R" ||
            name == "def_hand_L" ||
            name == "def_pinky_finger_01_L" ||
            name == "def_pinky_finger_02_L" ||
            name == "def_pinky_finger_03_L" ||
            name == "def_carpol_01_L" ||
            name == "def_pointer_finger_01_L" ||
            name == "def_pointer_finger_02_L" ||
            name == "def_pointer_finger_03_L" ||
            name == "def_carpol_02_L" ||
            name == "def_middle_finger_01_L" ||
            name == "def_middle_finger_02_L" ||
            name == "def_middle_finger_03_L" ||
            name == "def_carpol_03_L" ||
            name == "def_ring_finger_01_L" ||
            name == "def_ring_finger_02_L" ||
            name == "def_ring_finger_03_L" ||
            name == "def_carpol_04_L" ||
            name == "def_carpol_05_L" ||
            name == "def_thumb_01_L" ||
            name == "def_thumb_02_L" ||
            name == "def_thumb_03_L" ||
            name == "Body" ||
            name == "Mouth" ||
            name == "Nails" ||
            name == "Teeth" ||
            name == "Tongue" ||
            name == "RightEye_Iris" ||
            name == "RightEye_Sclera" ||
            name == "LeftEye_Iris" ||
            name == "LeftEye_Sclera" ||
            name == "def_neck_01") {
            finalLocals[i] = m_animationStates[1].m_localNodeTransforms[i];
        }
    }


    // Rebuild global chain
    m_globalBlendedNodeTransforms.resize(nodeCount);
    for (int i = 0; i < nodeCount; ++i) {
        glm::mat4 localM = finalLocals[i].to_mat4();
        int parent = m_skinnedModel->m_nodes[i].parentIndex;
        if (parent >= 0)
            m_globalBlendedNodeTransforms[i] =
            m_globalBlendedNodeTransforms[parent] * localM;
        else
            m_globalBlendedNodeTransforms[i] = localM;
    }

    //for (auto& index : m_skinnedModel->m_boneNodeIndex) {
    //    int idx = m_skinnedModel->m_boneNodeIndex[index];
    //    std::cout << index << ": " << idx << ": " << m_skinnedModel->m_nodes[idx].name << "\n";
    //}
    //std::cout << "\n";


    //std::cout << m_globalBlendedNodeTransforms.size() << "\n";
}


void Animator::UpdateAnimation(AnimationState& animationState, float deltaTime) {
    int nodeCount = m_skinnedModel->GetNodeCount();

    // Ensure transforms match the number of nodes
    animationState.m_globalNodeTransforms.resize(nodeCount);
    animationState.m_localNodeTransforms.resize(nodeCount);

    // Update current animation time based on deltaTime
    float timeInTicks = 0;
    if (animationState.m_animation) {
        if (!animationState.m_paused) {
            animationState.m_currentTime += deltaTime * animationState.m_animationSpeed;
        }
        float duration = animationState.m_animation->m_duration / animationState.m_animation->m_ticksPerSecond;
        if (animationState.m_currentTime >= duration) {
            if (!animationState.m_loop) {
                animationState.m_currentTime = duration;
                animationState.m_paused = true;
                animationState.m_isComplete = true;
            }
            else {
                animationState.m_currentTime = 0;
            }
        }
        timeInTicks = GetAnimationTimeInTicks(animationState);
    }

    // Traverse the tree
    for (int i = 0; i < m_skinnedModel->m_nodes.size(); i++) {
        glm::mat4 nodeTransformation = glm::mat4(1.0f);// m_skinnedModel->m_nodes[i].inverseBindTransform;
        std::string& nodeName = m_skinnedModel->m_nodes[i].name;

        // Interpolate the node transformation if it's animated
        if (animationState.m_animation) {
            const AnimatedNode* animatedNode = FindAnimatedNode(animationState.m_animation, nodeName.c_str());
            if (animatedNode) {
                glm::vec3 translation;
                glm::quat rotation;
                glm::vec3 scale;
                Util::CalcInterpolatedPosition(translation, timeInTicks, animatedNode);
                Util::CalcInterpolatedRotation(rotation, timeInTicks, animatedNode);
                Util::CalcInterpolatedScale(scale, timeInTicks, animatedNode);
                nodeTransformation = glm::translate(glm::mat4(1.0f), translation) * glm::mat4(rotation);
                nodeTransformation = glm::scale(nodeTransformation, scale);
            }
        }
        else {
            nodeTransformation = m_skinnedModel->m_nodes[i].inverseBindTransform;
        }

        animationState.m_localNodeTransforms[i] = AnimatedTransform(nodeTransformation);
    }

    // Rebuild global matrices from the locals
    for (int i = 0; i < nodeCount; ++i) {
        glm::mat4 localM = animationState.m_localNodeTransforms[i].to_mat4();
        int parent = m_skinnedModel->m_nodes[i].parentIndex;
        glm::mat4 worldM = (parent >= 0) ? animationState.m_globalNodeTransforms[parent].to_mat4() * localM : localM;
        animationState.m_globalNodeTransforms[i] = AnimatedTransform(worldM);
    }
}

float Animator::GetAnimationTimeInTicks(AnimationState& animationState) {
    Animation* animation = animationState.m_animation;
    if (!animation) return 0;
    
    float ticksPerSecond = animation->m_ticksPerSecond != 0 ? animation->m_ticksPerSecond : 25.0f;
    float timeInTicks = animationState.m_currentTime * ticksPerSecond;
    if (timeInTicks == animation->m_duration) {
        return timeInTicks;
    }
    else {
        timeInTicks = fmod(timeInTicks, animation->m_duration);
        timeInTicks = std::min(timeInTicks, animation->m_duration);
        return timeInTicks;
    }
}

const AnimatedNode* Animator::FindAnimatedNode(Animation* animation, const char* NodeName) {
    for (unsigned int i = 0; i < animation->m_animatedNodes.size(); i++) {
        const AnimatedNode* animatedNode = &animation->m_animatedNodes[i];

        if (Util::StrCmp(animatedNode->m_nodeName, NodeName)) {
            return animatedNode;
        }
    }
    return nullptr;
}
