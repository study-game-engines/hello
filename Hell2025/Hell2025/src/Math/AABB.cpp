#include "AABB.h"
#include "Util.h"
#include <algorithm>

AABB::AABB(glm::vec3 min, glm::vec3 max) {
    boundsMin = min;
    boundsMax = max;
    CalculateCenter();
}
void AABB::Grow(AABB& b) {
    if (b.boundsMin.x != 1e30f && b.boundsMin.x != -1e30f) {
        Grow(b.boundsMin); Grow(b.boundsMax);
    }
    AABB::CalculateCenter();
}
void AABB::Grow(glm::vec3 p) {
    boundsMin = glm::vec3(std::min(boundsMin.x, p.x), std::min(boundsMin.y, p.y), std::min(boundsMin.z, p.z));
    boundsMax = glm::vec3(std::max(boundsMax.x, p.x), std::max(boundsMax.y, p.y), std::max(boundsMax.z, p.z));
    CalculateCenter();
}
float AABB::Area() {
    glm::vec3 e = boundsMax - boundsMin; // box extent
    return e.x * e.y + e.y * e.z + e.z * e.x;
}
const glm::vec3 AABB::GetCenter() const {
    return center;
}
const glm::vec3 AABB::GetBoundsMin() const {
    return boundsMin;
}
const glm::vec3 AABB::GetBoundsMax() const {
    return boundsMax;
}

void AABB::CalculateCenter() {
    center = { (boundsMin.x + boundsMax.x) / 2, (boundsMin.y + boundsMax.y) / 2, (boundsMin.z + boundsMax.z) / 2 };
}

bool AABB::ContainsPoint(glm::vec3 point) const {
    return (point.x >= boundsMin.x && point.x <= boundsMax.x) &&
        (point.y >= boundsMin.y && point.y <= boundsMax.y) &&
        (point.z >= boundsMin.z && point.z <= boundsMax.z);
}

bool AABB::IntersectsSphere(const glm::vec3& sphereCenter, float radius) const {
    glm::vec3 closestPoint = glm::clamp(sphereCenter, boundsMin, boundsMax);
    float distSq = Util::DistanceSquared(closestPoint, sphereCenter);
    return distSq <= (radius * radius);
}

bool AABB::IntersectsAABB(const AABB& other) const {
    return (boundsMin.x <= other.boundsMax.x && boundsMax.x >= other.boundsMin.x) &&
        (boundsMin.y <= other.boundsMax.y && boundsMax.y >= other.boundsMin.y) &&
        (boundsMin.z <= other.boundsMax.z && boundsMax.z >= other.boundsMin.z);
}

bool AABB::IntersectsAABB(const AABB& other, float threshold) const {
    glm::vec3 inflatedMinA = boundsMin - glm::vec3(threshold);
    glm::vec3 inflatedMaxA = boundsMax + glm::vec3(threshold);
    glm::vec3 inflatedMinB = other.boundsMin - glm::vec3(threshold);
    glm::vec3 inflatedMaxB = other.boundsMax + glm::vec3(threshold);

    return (inflatedMinA.x <= inflatedMaxB.x && inflatedMaxA.x >= inflatedMinB.x) &&
        (inflatedMinA.y <= inflatedMaxB.y && inflatedMaxA.y >= inflatedMinB.y) &&
        (inflatedMinA.z <= inflatedMaxB.z && inflatedMaxA.z >= inflatedMinB.z);
}