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

    // m_animationStates[0].m_AnimationWeight = 0.1;
    std::vector<float> weightSum(nodeCount, 0.0f);
   
    // Clear output and weight accumulators
    for (int i = 0; i < nodeCount; ++i) {
        weightSum[i] = 0.0f;
        finalLocals[i].translation = glm::vec3(0.0f);
        finalLocals[i].scale = glm::vec3(0.0f);
        finalLocals[i].rotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f); // Identity quaternion
    }

    // Blend all animation states
    for (int stateIdx = 0; stateIdx < m_animationStates.size(); ++stateIdx) {
        const auto& state = m_animationStates[stateIdx];
        if (state.m_AnimationWeight <= 1e-5f) continue;

        for (int i = 0; i < nodeCount; ++i) {
            float w = state.m_AnimationWeight * state.m_boneWeights[i];

            if (w <= 1e-5f) continue;

            float newWeightSum = weightSum[i] + w;
            float blendFactor = w / newWeightSum;

            // Linearly blend translation and scale
            finalLocals[i].translation += state.m_localNodeTransforms[i].translation * w;
            finalLocals[i].scale += state.m_localNodeTransforms[i].scale * w;

            // Slerp rotation
            glm::quat current = glm::normalize(finalLocals[i].rotation);
            glm::quat target = state.m_localNodeTransforms[i].rotation;
            if (glm::dot(current, target) < 0.0f) target = -target;

            finalLocals[i].rotation = glm::slerp(current, target, blendFactor);
            finalLocals[i].rotation = glm::normalize(finalLocals[i].rotation);

            weightSum[i] = newWeightSum;
        }
    }

    // Normalize translation and scale by total weight
    for (int i = 0; i < nodeCount; ++i) {
        if (weightSum[i] > 1e-5f) {
            float inv = 1.0f / weightSum[i];
            finalLocals[i].translation *= inv;
            finalLocals[i].scale *= inv;
            finalLocals[i].rotation = glm::normalize(finalLocals[i].rotation); // already normalized, but safe
        }
        else {
         // Fallback to bind pose or default animation
            finalLocals[i] = m_animationStates[0].m_localNodeTransforms[i];
        }
    }

    //finalLocals = m_animationStates[0].m_localNodeTransforms;

    // Rebuild global chain
    m_globalBlendedNodeTransforms.resize(nodeCount);
    for (int i = 0; i < nodeCount; ++i) {
        glm::mat4 localM = finalLocals[i].to_mat4();
        int parent = m_skinnedModel->m_nodes[i].parentIndex;
        if (parent >= 0) {
            m_globalBlendedNodeTransforms[i] = m_globalBlendedNodeTransforms[parent] * localM;
        }
        else {
            m_globalBlendedNodeTransforms[i] = localM;
        }
    }
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

        //if (nodeName == "def_spine_02") {
        //    Transform transform;
        //    transform.rotation.y = HELL_PI;
        //    nodeTransformation = transform.to_mat4();
        //}

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
