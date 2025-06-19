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
    std::vector<AnimatedTransform> finalTransforms(nodeCount);
    std::vector<float> weightSum(nodeCount, 0.0f);

    for (auto& t : finalTransforms) {
        t.translation = glm::vec3(0.0f);
        t.scale = glm::vec3(0.0f);
        t.rotation = glm::quat(0.0f, 0.0f, 0.0f, 0.0f);
    }

    // Accumulate weights
    for (AnimationState& animationState : m_animationStates) {
        for (int i = 0; i < nodeCount; i++) {
            float weight = animationState.m_AnimationWeight * animationState.m_boneWeights[i];
            weightSum[i] += weight;
            finalTransforms[i].translation += animationState.m_globalNodeTransforms[i].translation * weight;
            finalTransforms[i].rotation += animationState.m_globalNodeTransforms[i].rotation * weight;  // Quaternion add
            finalTransforms[i].scale += animationState.m_globalNodeTransforms[i].scale * weight;
        }
    }
    // Normalize weights
    for (int i = 0; i < nodeCount; i++) {
        float inv = 1.0f / (weightSum[i] + 1e-5f);
        finalTransforms[i].translation *= inv;
        finalTransforms[i].scale *= inv;
        finalTransforms[i].rotation = glm::normalize(finalTransforms[i].rotation * inv);
    }

    // Store global matrices
    m_globalBlendedNodeTransforms.resize(nodeCount);
    for (int i = 0; i < nodeCount; i++) {
        m_globalBlendedNodeTransforms[i] = finalTransforms[i].to_mat4();
    }

}


void Animator::UpdateAnimation(AnimationState& animationState, float deltaTime) {
    // Ensure transforms match the number of joints
    animationState.m_globalNodeTransforms.resize(m_skinnedModel->m_nodes.size());

    std::cout << "m_skinnedModel->m_nodes.size(): " << m_skinnedModel->m_nodes.size() << "\n";

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
        glm::mat4 nodeTransformation = glm::mat4(1);
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

        // Apply additive matrix
        //auto it = additiveBoneTransforms.find(nodeName);
        //if (it != additiveBoneTransforms.end()) {
        //    nodeTransformation = it->second * nodeTransformation;
        //}

        // Calculate the world transform for this joint
        unsigned int parentIndex = m_skinnedModel->m_nodes[i].parentIndex;
        glm::mat4 ParentTransformation = (parentIndex == -1) ? glm::mat4(1) : animationState.m_globalNodeTransforms[parentIndex].to_mat4();
        glm::mat4 GlobalTransformation = ParentTransformation * nodeTransformation;
        animationState.m_globalNodeTransforms[i] = AnimatedTransform(GlobalTransformation);
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
