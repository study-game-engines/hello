#include "Player.h"
#include "Core/Audio.h"
#include "Config/Config.h"
#include "Viewport/ViewportManager.h"
#include "Util.h"

// GET THIS OUT OF HERE
// GET THIS OUT OF HERE
// GET THIS OUT OF HERE
#include "API/OpenGL/Renderer/GL_renderer.h"
// GET THIS OUT OF HERE
// GET THIS OUT OF HERE
// GET THIS OUT OF HERE


void Player::UpdateFlashlight(float deltaTime) {
    // Toggle on/off
    if (PressedFlashlight()) {
        Audio::PlayAudio("Flashlight.wav", 1.5f);
        m_flashlightOn = !m_flashlightOn;
    }
    // Modifier
    if (!m_flashlightOn) {
        m_flashLightModifier = 0.0f;
    }
    else {
        m_flashLightModifier = Util::FInterpTo(m_flashLightModifier, 1.0f, deltaTime, 10.5f);
    }

    if (!ViewportIsVisible()) {
        return;
    }

    // Prevent NAN direction, which is the case on first spawn
    if (Util::IsNan(m_flashlightDirection)) {
        m_flashlightDirection = GetCameraForward();
    }

    float distanceToTextureHit = 9999;
    float distanceToPhysxRayHit = 9999;
    float finalDistance = 0.0f;
    glm::vec3 textureHitPos = glm::vec3(0.0f);
    glm::vec3 physxRayHitPos = glm::vec3(0.0f);
    glm::vec3 finalHitPos = glm::vec3(0.0f);

    // World position texture hit position
    if (OpenGLRenderer::IsPlayerRayWorldPositionReadBackReady(m_viewportIndex)) {
        textureHitPos = OpenGLRenderer::GetPlayerRayWorldPostion(m_viewportIndex);
        if (textureHitPos != glm::vec3(0.0f)) {
            distanceToTextureHit = glm::distance(GetCameraPosition(), textureHitPos);
        }
    }

    // PhysX camera ray hit position
    if (m_cameraRayResult.hitFound) {
        physxRayHitPos = m_cameraRayResult.hitPosition;
        distanceToPhysxRayHit = glm::distance(GetCameraPosition(), physxRayHitPos);
    }

    // If not hit was found
    if (textureHitPos == glm::vec3(0.0f) && physxRayHitPos == glm::vec3(0.0f)) {
        m_flashlightPosition = GetCameraPosition();
        m_flashlightDirection = GetCameraForward();
        //std::cout << "no hit \n";
        return;
    }
    // Otherwise take the closest hit
    else if (distanceToTextureHit < distanceToPhysxRayHit) {
        finalHitPos = textureHitPos;
        finalDistance = distanceToTextureHit;
    }
    else {
        finalHitPos = physxRayHitPos;
        finalDistance = distanceToPhysxRayHit;
    }
    
    // Centered pos/dir
    glm::vec3 centeredFlashlightPosition = GetCameraPosition() - GetCameraForward() * glm::vec3(0.2f);
    glm::vec3 centeredFlashlightDirection = GetCameraForward();

    // Offset pos/dir
    glm::vec3 offsetFlashlightPosition = centeredFlashlightPosition;
    offsetFlashlightPosition += GetCameraRight() * glm::vec3(0.1f);
    offsetFlashlightPosition -= GetCameraUp() * glm::vec3(m_bobOffsetY * 2);
    glm::vec3 offsetFlashlightDirection = glm::normalize(finalHitPos - offsetFlashlightPosition);

    // Compute lerp factor
    float maxDistance = 1.0f;
    float t = glm::clamp(finalDistance / maxDistance, 0.0f, 0.75f);

    // Mix between centered and offset based on distance to cam hit
    glm::vec3 flashlightPositionTarget = glm::mix(centeredFlashlightPosition, offsetFlashlightPosition, t);
    glm::vec3 flashlightDirectionTarget = glm::mix(centeredFlashlightDirection, offsetFlashlightDirection, t);

    // If no hit was found then default back to centered
    if (textureHitPos == glm::vec3(0.0f) && physxRayHitPos == glm::vec3(0.0f)) {
        flashlightPositionTarget = centeredFlashlightPosition;
        flashlightDirectionTarget = centeredFlashlightDirection;
    }


    // Lerp between last pos/dir to the new ones
    float interSpeed = 40;
    m_flashlightPosition = Util::LerpVec3(m_flashlightPosition, flashlightPositionTarget, deltaTime, interSpeed);
    m_flashlightDirection = Util::LerpVec3(m_flashlightDirection, flashlightDirectionTarget, deltaTime, interSpeed);

    //OpenGLRenderer::DrawPoint(textureHitPos, RED);

    // Projection view matrix
    float lightRadius = 10.0f;
    float outerAngle = glm::radians(30.0);
    glm::vec3 flashlightTargetPosition = m_flashlightPosition + m_flashlightDirection;
    glm::mat4 flashlightViewMatrix = glm::lookAt(m_flashlightPosition, flashlightTargetPosition, GetCameraUp());
    glm::mat4 spotlightProjection = glm::perspectiveZO(outerAngle * 2, 1.0f, 0.05f, lightRadius);
    m_flashlightProjectionView = spotlightProjection * flashlightViewMatrix;
}

void Player::UpdateFlashlightFrustum() {
    const Resolutions& resolutions = Config::GetResolutions();
    int renderTargetWidth = resolutions.gBuffer.x;
    int renderTargetHeight = resolutions.gBuffer.y;
    Viewport* viewport = ViewportManager::GetViewportByIndex(m_viewportIndex);
    float viewportWidth = viewport->GetSize().x * renderTargetWidth;
    float viewportHeight = viewport->GetSize().y * renderTargetHeight;
    float aspect = viewportWidth / viewportHeight;
    float nearPlane = 0.01f;
    float farPlane = 10.0f;
    glm::mat4 perspectiveMatrix = glm::perspective(m_cameraZoom, aspect, nearPlane, farPlane);
    glm::mat4 projectionView = perspectiveMatrix * m_camera.GetViewMatrix();
    m_flashlightFrustum.Update(m_flashlightProjectionView);
}