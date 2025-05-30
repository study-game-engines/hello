#pragma once
#include "HellTypes.h"
#include "Types/Game/AnimatedGameObject.h"
#include "CreateInfo.h"
#include "HellDefines.h"

struct Kangaroo {
    void Init(KangarooCreateInfo createInfo);
    void Update(float deltaTime);
    void Kill();
    void GiveDamage(int damage);
    void CleanUp();
    void Respawn();

    AnimatedGameObject* GetAnimatedGameObject();

    int GetHealth() { return m_health;  }

private:
    bool m_alive = true;
    int m_maxHealth = KANGAROO_MAX_HEALTH;
    int m_health = 500;
    int32_t m_woundMaskIndex = -1;
    glm::vec3 m_position;
    glm::vec3 m_rotation;
    uint64_t m_animatedGameObjectId = 0;
    KangarooCreateInfo m_createInfo;
};