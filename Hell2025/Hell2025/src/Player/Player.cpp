#include "Player.h"
#include "HellDefines.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "BackEnd/BackEnd.h"
#include "Audio/Audio.h"
#include "Core/Game.h"
#include "Editor/Editor.h"
#include "Input/Input.h"
#include "Viewport/ViewportManager.h"
#include "Ocean/Ocean.h"
#include "UniqueID.h"

// Get me out of here
#include "World/World.h"
// Get me out of here

void Player::Init(glm::vec3 position, glm::vec3 rotation, int32_t viewportIndex) {
    m_playerId = UniqueID::GetNext();

    m_position = position;
    m_camera.SetPosition(m_position + glm::vec3(0.0f, m_viewHeightStanding, 0.0f));
    m_camera.SetEulerRotation(rotation);
    m_viewportIndex = viewportIndex;

    AnimatedGameObject* viewWeapon = GetViewWeaponAnimatedGameObject();
    viewWeapon->SetExclusiveViewportIndex(viewportIndex);

    AnimatedGameObject* characterModel = GetCharacterModelAnimatedGameObject();

    SpriteSheetObjectCreateInfo createInfo;
    createInfo.textureName = "MuzzleFlash_4x5";
    createInfo.loop = false;
    createInfo.billboard = true;
    createInfo.renderingEnabled = false;
    m_muzzleFlash.Init(createInfo);

    CreateCharacterController(m_position);
    InitCharacterModel();
    InitRagdoll();
}

void Player::BeginFrame() {
    m_interactFound = false;
    m_interactObjectId = 0;
    m_interactObjectType = ObjectType::NONE;
    //m_interactPhysicsId = 0;
    //m_interactPhysicsType = PhysicsType::NONE;
}

void Player::Update(float deltaTime) {
    if (Editor::IsOpen()) {
        return;
    }

    if (IsAwaitingSpawn()) Respawn();

    if (World::HasOcean()) {
        float feetHeight = GetFootPosition().y;
        float waterHeight = Ocean::GetWaterHeightAtPlayer(m_viewportIndex);
        m_waterState.feetUnderWaterPrevious = m_waterState.feetUnderWater;
        m_waterState.cameraUnderWaterPrevious = m_waterState.cameraUnderWater;
        m_waterState.wadingPrevious = m_waterState.wading;
        m_waterState.swimmingPrevious = m_waterState.swimming;
        m_waterState.cameraUnderWater = GetCameraPosition().y < waterHeight;
        m_waterState.feetUnderWater = GetFootPosition().y < waterHeight;
        m_waterState.heightAboveWater = (GetFootPosition().y > waterHeight) ? (GetFootPosition().y - waterHeight) : 0.0f;
        m_waterState.heightBeneathWater = (GetFootPosition().y < waterHeight) ? (waterHeight - GetFootPosition().y) : 0.0f;
        m_waterState.swimming = m_waterState.cameraUnderWater && IsMoving();
        m_waterState.wading = !m_waterState.cameraUnderWater && m_waterState.feetUnderWater && IsMoving() && feetHeight < waterHeight - 0.5f;
    } 
    else {
        m_waterState.feetUnderWaterPrevious = false;
        m_waterState.cameraUnderWaterPrevious = false;
        m_waterState.wadingPrevious = false;
        m_waterState.swimmingPrevious = false;
        m_waterState.cameraUnderWater = false;
        m_waterState.feetUnderWater = false;
        m_waterState.heightAboveWater = 0.0f;
        m_waterState.heightBeneathWater = 0.0f;
        m_waterState.swimming = false;
        m_waterState.wading = false;
    }

    // Weapon audio frequency (for under water)
    m_weaponAudioFrequency = CameraIsUnderwater() ? 0.4f : 1.0f;
 
    UpdateMovement(deltaTime);
    UpdateCharacterController();
    UpdateHeadBob(deltaTime);
    UpdateBreatheBob(deltaTime);
    UpdateCamera(deltaTime);
    UpdateCursorRays();
    UpdateInteract();
    UpdateWeaponLogic(deltaTime);
    UpdateViewWeapon(deltaTime);
    UpdateAnimatedGameObjects(deltaTime);
    UpdateWeaponSlide();
    UpdateSpriteSheets(deltaTime);
    UpdateAudio();
    UpdateUI();
    UpdateFlashlight(deltaTime);
    UpdateFlashlightFrustum();
    UpdatePlayingPiano(deltaTime);

    UpdateCharacterModelHacks();

    if (m_infoTextTimer > 0) {
        m_infoTextTimer -= deltaTime;
    }
    else {
        m_infoTextTimer = 0;
        m_infoText = "";
    }

    if (IsAlive()) {
        m_timeSinceDeath = 0.0f;
    }
    else {
        m_timeSinceDeath += deltaTime;
    }
}

void Player::Respawn() {
    AnimatedGameObject* viewWeapon = GetViewWeaponAnimatedGameObject();
    //viewWeapon->SetSkinnedModel("Glock");
    //viewWeapon->PlayAndLoopAnimation("Glock_Reload");
    //viewWeapon->SetSkinnedModel("SPAS");
    //viewWeapon->PlayAndLoopAnimation("SPAS_Reload2Shells");
    //viewWeapon->SetSkinnedModel("Knife");
    //viewWeapon->PlayAndLoopAnimation("Knife_Idle");

    m_alive = true;

    if (m_viewportIndex == 0) {
        SetFootPosition(glm::vec3(17.0f, 30.7f, 41.5f));
        SetFootPosition(glm::vec3(27.0f, 32.7f, 36.5f)); // roo
    }
    if (m_viewportIndex == 1) {
        SetFootPosition(glm::vec3(12.5f, 30.6f, 45.5f));
        m_camera.SetEulerRotation(glm::vec3(0, 0, 0));
    }

    m_weaponStates.clear();
    for (int i = 0; i < WeaponManager::GetWeaponCount(); i++) {
        WeaponState& state = m_weaponStates.emplace_back();
        state.name = WeaponManager::GetWeaponInfoByIndex(i)->name;
        state.has = false;
        state.ammoInMag = 0;
    }

    m_ammoStates.clear();
    for (int i = 0; i < WeaponManager::GetAmmoTypeCount(); i++) {
        AmmoState& state = m_ammoStates.emplace_back();
        state.name = WeaponManager::GetAmmoInfoByIndex(i)->name;
        state.ammoOnHand = 0;
    }

    GiveDefaultLoadout();
    SwitchWeapon("Knife", WeaponAction::DRAW_BEGIN);

    m_flashlightOn = false;
    m_awaitingSpawn = false; 

    m_camera.Update();
    m_flashlightDirection = m_camera.GetForward();

}



void Player::EnableControl() {
    m_controlEnabled = true;
}
void Player::DisableControl() {
    m_controlEnabled = false;
}

const bool Player::IsAwaitingSpawn() {
    return m_awaitingSpawn;
}

const bool Player::HasControl() {
    return m_controlEnabled;
}

const bool Player::IsLocal() const {
    return m_viewportIndex != -1;
}

const bool Player::IsOnline() const {
    return m_viewportIndex == -1;
}

const glm::mat4& Player::GetProjectionMatrix() const {
    int width = BackEnd::GetCurrentWindowWidth();
    int height = BackEnd::GetCurrentWindowHeight();
    return glm::perspective(1.0f, float(width) / float(height), NEAR_PLANE, FAR_PLANE);
}

const glm::mat4& Player::GetViewMatrix() const {
    return m_camera.GetViewMatrix();
}

const glm::mat4& Player::GetInverseViewMatrix() const {
    return m_camera.GetInverseViewMatrix();
}

const glm::vec3& Player::GetCameraPosition() const {
    return m_camera.GetPosition();
}

const glm::vec3& Player::GetCameraRotation() const {
    return m_camera.GetEulerRotation();
}

const glm::vec3& Player::GetCameraForward() const {
    return m_camera.GetForward();
}

const glm::vec3& Player::GetCameraRight() const {
    return m_camera.GetRight();
}

const glm::vec3& Player::GetCameraUp() const {
    return m_camera.GetUp();
}

const int32_t Player::GetViewportIndex() const {
    return m_viewportIndex;
}

const glm::vec3 Player::GetFootPosition() const {
    PxExtendedVec3 pxPos = m_characterController->getFootPosition();
    return glm::vec3(
        static_cast<float>(pxPos.x),
        static_cast<float>(pxPos.y),
        static_cast<float>(pxPos.z)
    );

    //return m_position;
}

Camera& Player::GetCamera() {
    return m_camera;
}

AnimatedGameObject* Player::GetCharacterModelAnimatedGameObject() {
    return &m_characterModelAnimatedGameObject;
}

AnimatedGameObject* Player::GetViewWeaponAnimatedGameObject() {
    return &m_viewWeaponAnimatedGameObject;
}

bool Player::ViewportIsVisible() {
    Viewport* viewport = ViewportManager::GetViewportByIndex(m_viewportIndex);
    if (!viewport) {
        return false;
    }
    else {
        return viewport->IsVisible();
    }
}

bool Player::ViewModelAnimationsCompleted() {
    AnimatedGameObject* viewWeapon = GetViewWeaponAnimatedGameObject();
    if (!viewWeapon) {
        std::cout << "WARNING!!! Player::ViewModelAnimationsCompleted() failed coz viewWeapon was nullptr\n";
        return true;
    }
    for (AnimationState& animationState : viewWeapon->m_animationLayer.m_animationStates) {
        if (!animationState.IsComplete()) {
            return false;
        }
    }
    return true;
}

float Player::GetWeaponAudioFrequency() {
    return m_weaponAudioFrequency;
}

glm::mat4& Player::GetViewWeaponCameraMatrix() {
    return m_viewWeaponCameraMatrix;
}

glm::mat4& Player::GetCSMViewMatrix() {
    return m_csmViewMatrix;
}

void Player::DisplayInfoText(const std::string& text) {
    m_infoTextTimer = 2.0f;
    m_infoText = text;
}

void Player::UpdateAnimatedGameObjects(float deltaTime) {
    m_viewWeaponAnimatedGameObject.Update(deltaTime);
    m_characterModelAnimatedGameObject.Update(deltaTime);

    m_viewWeaponAnimatedGameObject.m_exclusiveViewportIndex = m_viewportIndex;
    m_characterModelAnimatedGameObject.m_ignoredViewportIndex = m_viewportIndex;
}

const float Player::GetFov() {
    return m_cameraZoom;
}

void Player::Kill() {
    m_alive = false;
    m_characterModelAnimatedGameObject.SetAnimationModeToRagdoll();
    Audio::PlayAudio("Death0.wav", 1.0f);
}

glm::vec3 Player::GetViewportColorTint() {
    glm::vec3 color = glm::vec3(1, 1, 1);

    if (IsDead()) {
        color.r = 2.0;
        color.g = 0.2f;
        color.b = 0.2f;

        float waitTime = 3;
        if (m_timeSinceDeath > waitTime) {
            float val = (m_timeSinceDeath - waitTime) * 10;
            color.r -= val;
        }
    }

    if (m_viewportIndex == 0) {
        std::cout << color << "\n";
    }

    return color;
}

bool Player::RespawnAllowed() {
    return IsDead() && m_timeSinceDeath > 3.25f;
}


float Player::GetViewportContrast() {
    if (IsAlive()) {
        return 1.0f;
    }
    else {
        return 1.1f;
    }
}

Ragdoll* Player::GetRagdoll() {
    return Physics::GetRagdollById(m_characterModelAnimatedGameObject.GetRagdollId());
}

uint64_t Player::GetRadollId() {
    return m_characterModelAnimatedGameObject.GetRagdollId();
}

