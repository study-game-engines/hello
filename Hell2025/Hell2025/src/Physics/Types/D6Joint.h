#pragma once
#include "HellTypes.h"
#pragma warning(push, 0)
#include <physx/extensions/PxD6Joint.h>
#pragma warning(pop)

using namespace physx;

struct D6Joint {
    void SetPxD6Joint(PxD6Joint* pxD6Joint);
    PxD6Joint* GetPxD6Joint() { return m_pxD6Joint; }

private:
    PxD6Joint* m_pxD6Joint = nullptr;
};