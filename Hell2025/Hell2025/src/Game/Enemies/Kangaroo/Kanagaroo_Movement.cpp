#include "Kangaroo.h"
#include "Util.h"
#include "Core/Game.h"
#include "Physics/Physics.h"
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/euler_angles.hpp>

void Kangaroo::GoToTarget(glm::vec3 targetPosition) {
    m_targetPosition = targetPosition;

    if (m_animationState == KanagarooAnimationState::IDLE) {
        m_animationState = KanagarooAnimationState::IDLE_TO_HOP;
        PlayAnimation("Kangaroo_IdleToHop", 1.0f);
    }
}

void Kangaroo::UpdateMovementLogic(float deltaTime) {
    AnimatedGameObject* animatedGameObject = GetAnimatedGameObject();
    if (!animatedGameObject) return;

    std::vector<glm::ivec2> path = GetPath();

    //if (path.size() <= 2) {
    //    // You need to actually transition into idle
    //    // You need to actually transition into idle
    //    // You need to actually transition into idle
    //    // You need to actually transition into idle
    //    m_animationState = KanagarooAnimationState::IDLE;
    //    PlayAndLoopAnimation("Kangaroo", 1.0f);
    //}

    // Chilling idle
    if (m_animationState == KanagarooAnimationState::IDLE) {

    }

    // Hopping
    if (path.size() >= 2) {
        if (m_animationState == KanagarooAnimationState::HOP ||
            m_animationState == KanagarooAnimationState::IDLE_TO_HOP ||
            m_animationState == KanagarooAnimationState::BITE) {

            // Get 2D direction
            glm::vec2 currentCell = GetGridPosition();
            glm::vec2 nextPathCell = path[1];
            glm::vec2 direction2D = glm::normalize(nextPathCell - currentCell);

            // Compute forward vector from 2d direction
            m_forward = glm::vec3(direction2D.x, 0, direction2D.y);

            // Move him
            float speed = 7.5f;

            // Reduce speed as bite ends
            if (m_animationState == KanagarooAnimationState::BITE) {
                float t = m_timeSinceBiteBegan / 1.5f; 
                float easedT = Util::EaseOut(t, 3.0f);
                easedT = glm::clamp(easedT, 0.0f, 1.0f);
                float newSpeed = glm::mix(speed, 0.0f, easedT);
                speed = newSpeed;
                //std::cout << "speed: " << speed << "\n";
            }


            glm::vec3 displacement = m_forward * speed * deltaTime;
            m_position += displacement;
        }
    }

    // Set y from heightmap ray cast
    glm::vec3 rayOrigin = m_position + glm::vec3(0.0f, 100.0f, 0.0f);
    glm::vec3 rayDirection = glm::vec3(0.0f, -1.0f, 0.0f);
    float rayLength = 1000;
    PhysXRayResult rayresult = Physics::CastPhysXRayHeightMap(rayOrigin, rayDirection, rayLength);
    if (rayresult.hitFound) {
        m_position.y = rayresult.hitPosition.y;
    }
}