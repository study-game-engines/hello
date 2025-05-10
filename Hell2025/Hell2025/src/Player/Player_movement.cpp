#include "Player.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "Editor/Editor.h"
#include "Input/Input.h"
#include "Ocean/Ocean.h"
#include "World/World.h"
#include "Util.h"

void Player::UpdateMovement(float deltaTime) {
    if (World::HasOcean() && GetCameraPosition().y < Ocean::GetWaterHeight() + 0.1f) {
        UpdateSwimmingMovement(deltaTime);
    }
    else {
        UpdateWalkingMovement(deltaTime);
    }
}

void Player::UpdateWalkingMovement(float deltaTime) {

    m_moving = false;
    m_crouching = PressingCrouch();
    m_groundedLastFrame = m_grounded;

    if (!ViewportIsVisible()) {
        //return;
    }
    
    if (!Editor::IsOpen() && m_controlEnabled) {
        // Speed factor
        float speedFactor = 1.0f;
        if (Input::KeyDown(GLFW_KEY_LEFT_SHIFT)) {
            speedFactor = 5.0f;
        }

        // View height
        float heightSpeed = 3.0f;
        if (Input::KeyDown(HELL_KEY_EQUAL)) {
            m_position.y += deltaTime * heightSpeed * speedFactor;
        }
        if (Input::KeyDown(GLFW_KEY_MINUS)) {
            m_position.y -= deltaTime * heightSpeed * speedFactor;
        }

        // WSAD
        glm::vec3 displacement = glm::vec3(0);
        if (PressingWalkLeft()) {
            displacement -= m_camera.GetRight();
            m_moving = true;
        }
        if (PressingWalkRight()) {
            displacement += m_camera.GetRight();
            m_moving = true;
        }
        if (PressingWalkForward()) {
            displacement += m_camera.GetForwardXZ();
            m_moving = true;
        }
        if (PressingWalkBackward()) {
            displacement -= m_camera.GetForwardXZ();
            m_moving = true;
        }

        // Calculate movement speed
        float targetSpeed = m_crouching ? m_crouchingSpeed : m_walkingSpeed;
        float interpolationSpeed = 18.0f;
        if (!IsMoving()) {
            targetSpeed = 0.0f;
            interpolationSpeed = 22.0f;
        }
        m_currentSpeed = Util::FInterpTo(m_currentSpeed, targetSpeed, deltaTime, interpolationSpeed);

        displacement *= m_currentSpeed * deltaTime * speedFactor;


        // Jump
        if (PresingJump() && HasControl() && m_grounded) {
            m_yVelocity = 4.9f;  // better magic value for jump strength
            m_grounded = false;
        }
        //if (IsOverlappingLadder()) {
        //    m_grounded = true;
        //}

        // Gravity
        if (m_grounded) {
            m_yVelocity = -3.5f;
        }
        else {
            float gravity = 15.75f; // 9.8 feels like the moon
            m_yVelocity -= gravity * deltaTime;
        }
        displacement.y += m_yVelocity * deltaTime;
        MoveCharacterController(glm::vec3(displacement.x, displacement.y, displacement.z));

        // Check grounded state
        m_grounded = false;
        for (CharacterCollisionReport& report : Physics::GetCharacterCollisionReports()) {
            m_grounded = (report.characterController == m_characterController && report.hitNormal.y > 0.5f);
        }
        Physics::ClearCharacterControllerCollsionReports();

        // Character controller AABB
        PxRigidDynamic* actor = m_characterController->getActor();
        PxBounds3 bounds = actor->getWorldBounds();
        glm::vec3 aabbMin = glm::vec3(bounds.minimum.x, bounds.minimum.y, bounds.minimum.z);
        glm::vec3 aabbMax = glm::vec3(bounds.maximum.x, bounds.maximum.y, bounds.maximum.z);
        m_characterControllerAABB = AABB(aabbMin, aabbMax);

        // Piano hacks
        if (IsPlayingPiano()) {
            m_crouching = true;
            m_moving = false;
        }
    }
}

void Player::UpdateSwimmingMovement(float deltaTime) {
    m_moving = false;

    // WSAD movement
    glm::vec3 displacement = glm::vec3(0);
    if (PressingWalkForward()) {
        displacement += m_camera.GetForward();
        m_moving = true;
    }
    if (PressingWalkBackward()) {
        displacement -= m_camera.GetForward();
        m_moving = true;
    }
    if (PressingWalkLeft()) {
        displacement -= m_camera.GetRight();
        m_moving = true;
    }
    if (PressingWalkRight()) {
        displacement += m_camera.GetRight();;
        m_moving = true;
    }
    // Calculate speed
    float targetSpeed = m_swimmingSpeed;
    float interpolationSpeed = 18.0f;
    if (!IsMoving()) {
        targetSpeed = 0.0f;
        interpolationSpeed = 22.0f;
    }
    m_currentSpeed = Util::FInterpTo(m_currentSpeed, targetSpeed, deltaTime, interpolationSpeed);

    // Normalize displacement vector and include player speed
    float len = length(displacement);
    if (len != 0.0) {
        displacement = (displacement / len) * m_currentSpeed * deltaTime;
    }
    float yDisplacement = m_yVelocity * deltaTime;
    displacement.y += yDisplacement;
    float yVelocityCancelationInterpolationSpeed = 15;
    m_yVelocity = Util::FInterpTo(m_yVelocity, 0, deltaTime, yVelocityCancelationInterpolationSpeed);

    float m_swimVerticalInterpolationSpeed = 15.0f;
    float m_swimMaxVerticalAcceleration = 0.05f;

    // Vertical movement
    if (PressingCrouch()) {
        m_swimVerticalAcceleration = Util::FInterpTo(m_swimVerticalAcceleration, -m_swimMaxVerticalAcceleration, deltaTime, m_swimVerticalInterpolationSpeed);
    }
    else if (PresingJump()) {
        m_swimVerticalAcceleration = Util::FInterpTo(m_swimVerticalAcceleration, m_swimMaxVerticalAcceleration, deltaTime, m_swimVerticalInterpolationSpeed);
    }
    else {
        m_swimVerticalAcceleration = Util::FInterpTo(m_swimVerticalAcceleration, 0.0f, deltaTime, 20.0f);
    }
    displacement.y += m_swimVerticalAcceleration;

    MoveCharacterController(glm::vec3(displacement.x, displacement.y, displacement.z));
}

//bool Player::IsUnderWater() {
//    return m_underWater;
//}

bool Player::IsPlayingPiano() {
    return m_isPlayingPiano;
}

bool Player::IsMoving() {
    return m_moving;
}

bool Player::IsCrouching() {
    return m_crouching;
}

bool Player::IsGrounded() {
    return m_grounded;
}

bool Player::FeetEnteredUnderwater() {
    return !m_waterState.feetUnderWaterPrevious && m_waterState.feetUnderWater;
}

bool Player::FeetExitedUnderwater() {
    return m_waterState.feetUnderWaterPrevious && !m_waterState.feetUnderWater;
}

bool Player::CameraEnteredUnderwater() {
    return !m_waterState.cameraUnderWaterPrevious && m_waterState.cameraUnderWater;
}

bool Player::CameraExitedUnderwater() {
    return m_waterState.cameraUnderWaterPrevious && !m_waterState.cameraUnderWater;
}

bool Player::CameraIsUnderwater() {
    return m_waterState.cameraUnderWater;
}

bool Player::FeetBelowWater() {
    return m_waterState.feetUnderWater;
}

bool Player::IsSwimming() {
    return m_waterState.cameraUnderWater && IsMoving();
}

bool Player::IsWading() {
    return m_waterState.wading;
}

bool Player::StartedSwimming() {
    return !m_waterState.swimmingPrevious && m_waterState.swimming;
}

bool Player::StoppedSwimming() {
    return m_waterState.swimmingPrevious && !m_waterState.swimming;
}

bool Player::StartingWading() {
    return !m_waterState.wadingPrevious && m_waterState.wading;
}

bool Player::StoppedWading() {
    return m_waterState.wadingPrevious && !m_waterState.wading;
}
