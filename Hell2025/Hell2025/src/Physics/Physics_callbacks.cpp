#include "Physics.h"

PxQueryHitType::Enum RaycastFilterCallback::preFilter(const PxFilterData& filterData, const PxShape* shape, const PxRigidActor* actor, PxHitFlags& queryFlags) {
    const PxFilterData sf = shape->getQueryFilterData();
    //if (!((filterData.word0 & sf.word1) && (filterData.word1 & sf.word0))) {
    //    return PxQueryHitType::eNONE;
    //}
    for (const PxRigidActor* pxRigidActor : m_ignoredActors) {
        if (actor == pxRigidActor) {
            return PxQueryHitType::eNONE;
        }
    }
    return PxQueryHitType::eBLOCK;
}

PxQueryHitType::Enum RaycastFilterCallback::postFilter(const PxFilterData& filterData, const PxQueryHit& hit, const PxShape* shape, const PxRigidActor* actor) {
    return PxQueryHitType::eBLOCK;
}


void RaycastFilterCallback::AddIgnoredActor(PxRigidDynamic* pxRigidDynamic) {
    m_ignoredActors.push_back(pxRigidDynamic);
}

void RaycastFilterCallback::AddIgnoredActors(std::vector<PxRigidDynamic*> pxRigidDynamics) {
    m_ignoredActors.reserve(m_ignoredActors.size() + pxRigidDynamics.size());
    for (PxRigidDynamic* pxRigidDynamic : pxRigidDynamics) {
        if (pxRigidDynamic) m_ignoredActors.push_back(pxRigidDynamic);
    }
}

PxQueryHitType::Enum RaycastHeightFieldFilterCallback::preFilter(const PxFilterData& filterData, const PxShape* shape, const PxRigidActor* actor, PxHitFlags& queryFlags) {
    const PxGeometryHolder geomHolder = shape->getGeometry();
    if (geomHolder.getType() != PxGeometryType::eHEIGHTFIELD) {
        return PxQueryHitType::eNONE;
    }
    return PxQueryHitType::eBLOCK;
}

PxQueryHitType::Enum RaycastHeightFieldFilterCallback::postFilter(const PxFilterData& filterData, const PxQueryHit& hit, const PxShape* shape, const PxRigidActor* actor) {
    return PxQueryHitType::eBLOCK;
}

