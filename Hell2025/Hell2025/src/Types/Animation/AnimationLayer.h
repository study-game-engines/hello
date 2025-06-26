#pragma once
#include <string>
#include "AnimationState.h"
#include <unordered_map>

struct AnimationLayerOLD {

    std::string m_name;
    int m_skinnedModelIndex = -1;
    int m_jointCount = -1;
    std::vector<AnimationStateOld> m_animationStates;   
    std::vector<glm::mat4> m_globalBlendedNodeTransforms;

    void SetSkinnedModel(const std::string& skinnedModelName);
    void Update(float deltaTime, std::unordered_map<std::string, glm::mat4>& additiveBoneTransforms);
    void PlayAnimation(const std::string& animationName, const AnimationPlaybackParams& playbackParams = AnimationPlaybackParams());
    void PlayAndLoopAnimation(const std::string& animationName, const AnimationPlaybackParams& playbackParams = AnimationPlaybackParams());
    void ClearAllAnimationStates();
    void SkinToBindPose();
    bool AllAnimationIsComplete();
    void ForceStopAnimationStateByName(const std::string& animationName);
    void ForceEaseOutAnimationStateByName(const std::string& animationName);
    void SetLoopStateByName(const std::string& animationName, bool loopState);
    void PauseAnimationStateByName(const std::string& animationName);
};