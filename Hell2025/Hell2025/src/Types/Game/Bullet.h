#pragma once
#include "CreateInfo.h"
#include "glm/vec3.hpp"

struct Bullet {
    Bullet() = default;
    Bullet(BulletCreateInfo& bulletCreateInfo) {
        m_origin = bulletCreateInfo.origin;
        m_direction = bulletCreateInfo.direction;
        m_weaponIndex = bulletCreateInfo.weaponIndex;
        m_damage = bulletCreateInfo.damage;
    }

    glm::vec3 GetOrigin()       { return m_origin; }
    glm::vec3 GetDirection()    { return m_direction; }
    int32_t GetWeaponIndex()    { return m_weaponIndex; }
    uint32_t GetDamage()        { return m_damage; }

private:
    glm::vec3 m_origin = glm::vec3(0);
    glm::vec3 m_direction = glm::vec3(0);
    int32_t m_weaponIndex = 0;
    uint32_t m_damage = 0;
};