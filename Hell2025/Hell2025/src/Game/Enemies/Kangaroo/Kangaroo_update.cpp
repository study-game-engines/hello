#include "Kangaroo.h"
#include "Pathfinding/AStarMap.h"
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

    // Rotate to target test
    if (Input::KeyDown(HELL_KEY_COMMA)) {

        AStarMap::Init();

        if (HasValidPath()) {
            Cell* nextPathCell = m_aStar.m_finalPath[1];
            glm::ivec2 nextPathCellCoords = glm::ivec2(nextPathCell->x, nextPathCell->y);
            glm::vec3 nextPathWorldPosition = AStarMap::GetWorldSpacePositionFromCellCoords(nextPathCellCoords);

            // Compute forward vector from 2d direction
            glm::vec3 normalizedPosition = m_position * glm::vec3(1.0f, 0.0f, 1.0f);
            glm::vec3 targetForward = glm::normalize(nextPathWorldPosition - normalizedPosition);
            
            float turnSpeed = 1.5f;
            float alpha = glm::clamp(turnSpeed * deltaTime, 0.0f, 1.0f);
            m_forward = glm::normalize(m_forward * (1.0f - alpha) + targetForward * alpha);
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
    
    //DebugDraw();

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