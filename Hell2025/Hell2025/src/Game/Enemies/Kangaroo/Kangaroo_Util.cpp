#include "Kangaroo.h"

const std::string Kangaroo::GetAnimationStateAsString() {
    switch (m_animationState) {
        case KanagarooAnimationState::IDLE:             return "IDLE";
        case KanagarooAnimationState::HOP_TO_IDLE:      return "HOP_TO_IDLE";
        case KanagarooAnimationState::IDLE_TO_HOP:      return "IDLE_TO_HOP";
        case KanagarooAnimationState::HOP:              return "HOP";
        default: return "UNKNOWN";
    }
}