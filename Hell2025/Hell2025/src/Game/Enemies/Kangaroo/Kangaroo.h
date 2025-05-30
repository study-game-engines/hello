#pragma once
#include "HellTypes.h"
#include "Types/Game/AnimatedGameObject.h"
#include "CreateInfo.h"
#include "HellDefines.h"
#include "Pathfinding/AStar.h"

enum struct KanagarooAgroState {
    CHILLING_IDLE,
    ATTACKING
};

enum struct KanagarooAnimationState {
    IDLE,
    HOP_TO_IDLE,
    IDLE_TO_HOP,
    HOP
};

enum struct KanagarooMovementState {
    IDLE,
    HOP
};

struct Kangaroo {
    void Init(KangarooCreateInfo createInfo);
    void Update(float deltaTime);
    void Kill();
    void GiveDamage(int damage);
    void CleanUp();
    void Respawn();

    void SetAgroState(KanagarooAgroState state);
    void SetMovementState(KanagarooMovementState state);

    void GoToTarget(glm::vec3 targetPosition);

    AnimatedGameObject* GetAnimatedGameObject();

    const std::string GetAnimationStateAsString();
    std::vector<glm::ivec2> GetPath();
    glm::vec2 GetGridPosition();

    int GetHealth()             { return m_health; }
    glm::vec3 GetPosition()     { return m_position; }

private:
    void UpdateMovement(float deltaTime);
    void UpdateAnimationStateMachine();
    void PlayAnimation(const std::string& animationName, float speed);
    void PlayAndLoopAnimation(const std::string& animationName, float speed);
    bool AnimationIsComplete();
    

    AStar m_aStar;
    glm::vec3 m_targetPosition = glm::vec3(0.0f);
    bool m_alive = true;
    int m_maxHealth = KANGAROO_MAX_HEALTH;
    int m_health = 500;
    int32_t m_woundMaskIndex = -1;
    glm::vec3 m_position;
    glm::vec3 m_rotation;
    uint64_t m_animatedGameObjectId = 0;
    KangarooCreateInfo m_createInfo;
    KanagarooAgroState m_agroState = KanagarooAgroState::CHILLING_IDLE;
    KanagarooMovementState m_movementState = KanagarooMovementState::IDLE;
    KanagarooAnimationState m_animationState = KanagarooAnimationState::IDLE;
};