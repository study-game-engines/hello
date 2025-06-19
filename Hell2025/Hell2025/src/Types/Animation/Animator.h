#pragma once
#include "Animation.h"
#include "Types/Renderer/SkinnedModel.h"
#include <string>
#include <vector>

struct AnimationState {
    Animation* m_animation;
    std::vector<AnimatedTransform> m_globalNodeTransforms;
    std::vector<AnimatedTransform> m_localNodeTransforms;
    float m_currentTime = 0.0f;
    float m_animationSpeed = 1.0f;
    bool m_loop = true;
    bool m_paused = false;
    bool m_isComplete = false;
    float m_AnimationWeight = 1.0f;
    std::vector<float> m_boneWeights;
};

struct Animator {
    void SetSkinnedModel(const std::string& skinnedModelName);
    void RegisterAnimation(const std::string& animationName);
    void UpdateAnimations(float deltaTime);
    
    std::vector<glm::mat4> m_LocalBlendedBoneTransforms;
    std::vector<glm::mat4> m_globalBlendedNodeTransforms;

    std::vector<AnimationState> m_animationStates;

private:
    void UpdateAnimation(AnimationState& animationState, float deltaTime);
    float GetAnimationTimeInTicks(AnimationState& animationState);
    const AnimatedNode* FindAnimatedNode(Animation* animation, const char* NodeName);

    SkinnedModel* m_skinnedModel = nullptr;
};