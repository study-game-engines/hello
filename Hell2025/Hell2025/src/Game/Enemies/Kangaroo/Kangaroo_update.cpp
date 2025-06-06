#include "Kangaroo.h"
#include "Util.h"

#include "Core/Game.h"   // remove me
#include "Input/Input.h" // remove me

void Kangaroo::Update(float deltaTime) {
    if (m_animationState == KanagarooAnimationState::BITE) {
        m_timeSinceBiteBegan += deltaTime;
    }
    else {
        m_timeSinceBiteBegan = 0.0f;
    }
    if (m_animationState == KanagarooAnimationState::IDLE) {
        m_timeSinceIdleBegan += deltaTime;
    }
    else {
        m_timeSinceIdleBegan = 0.0f;
    }

    if (Input::KeyPressed(HELL_KEY_COMMA)) {
        //SetMovementState(KanagarooMovementState::HOP);

        //glm::vec3 targetPosition = glm::vec3(42, 32, 40);
        //GoToTarget(targetPosition);

        if (HasValidPath()) {
            m_forward = glm::vec3(0, 0, 1);
        }
    }

    if (Input::KeyPressed(HELL_KEY_PERIOD)) {
        Respawn();
    }

    FindPathToTarget();

    UpdateAnimationStateMachine();
    UpdateMovementLogic(deltaTime);
    UpdateAnimatedGameObjectPositionRotation();

    UpdateAudio();
    
    DebugDraw();

    // Death check
    if (m_health <= 0) {
        Kill();
    }
    m_health = glm::clamp(m_health, 0, 9999999);
}

void Kangaroo::UpdateAnimatedGameObjectPositionRotation() {
    AnimatedGameObject* animatedGameObject = GetAnimatedGameObject();

    // TODO:
    // Get position from PhysX character controller

    // Compute euler from forward vector
    glm::vec3 start = m_position;
    glm::vec3 end = m_position + m_forward;
    m_rotation.x = 0.0f;
    m_rotation.y = Util::EulerYRotationBetweenTwoPoints(start, end) + (HELL_PI * 0.5f);
    m_rotation.z = 0.0f;

    animatedGameObject->SetPosition(m_position);
    animatedGameObject->SetRotationX(m_rotation.x);
    animatedGameObject->SetRotationY(m_rotation.y);
    animatedGameObject->SetRotationZ(m_rotation.z);
}