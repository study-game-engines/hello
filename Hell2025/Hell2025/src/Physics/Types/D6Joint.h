#pragma once
#include "HellTypes.h"
#include "PhysX/include/extensions/PxD6Joint.h"

using namespace physx;

struct D6Joint {
    void SetPxD6Joint(PxD6Joint* pxD6Joint);
    PxD6Joint* GetPxD6Joint() { return m_pxD6Joint; }

private:
    PxD6Joint* m_pxD6Joint = nullptr;
};