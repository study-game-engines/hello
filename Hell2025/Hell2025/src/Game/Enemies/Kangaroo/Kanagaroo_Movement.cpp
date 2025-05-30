#include "Kangaroo.h"


void Kangaroo::GoToTarget(glm::vec3 targetPosition) {
    m_targetPosition = targetPosition;

    if (m_animationState == KanagarooAnimationState::IDLE) {
        m_animationState = KanagarooAnimationState::IDLE_TO_HOP;
        PlayAnimation("Kangaroo_IdleToHop", 1.0f);
    }
}

void Kangaroo::UpdateAnimationStateMachine() {

    if (m_animationState == KanagarooAnimationState::IDLE_TO_HOP) {
        if (AnimationIsComplete()) {
            m_animationState = KanagarooAnimationState::HOP;
            PlayAndLoopAnimation("Kangaroo_Hop", 1.0f);
        }
    }

}

void Kangaroo::UpdateMovement(float deltaTime) {

    // you are never calling this bro.


    // Chilling idle
    if (m_movementState == KanagarooMovementState::IDLE) {

    }

    // Hopping
    if (m_movementState == KanagarooMovementState::HOP) {



    }

}