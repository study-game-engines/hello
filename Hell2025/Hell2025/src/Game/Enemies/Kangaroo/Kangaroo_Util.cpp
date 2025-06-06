#include "Kangaroo.h"

const std::string Kangaroo::GetAnimationStateAsString() {
    switch (m_animationState) {
        case KanagarooAnimationState::IDLE:             return "IDLE";
        case KanagarooAnimationState::HOP_TO_IDLE:      return "HOP_TO_IDLE";
        case KanagarooAnimationState::IDLE_TO_HOP:      return "IDLE_TO_HOP";
        case KanagarooAnimationState::HOP:              return "HOP";
        case KanagarooAnimationState::BITE:             return "BITE";
        default: return "UNKNOWN";
    }
}

bool Kangaroo::HasValidPath() {
    return m_aStar.m_finalPath.size() >= 2; 
    
    // TODO!!! Replace with smooth path
}