#include "ScreenSpaceBloodDecal.h"

void ScreenSpaceBloodDecal::Init(ScreenSpaceBloodDecalCreateInfo createInfo) {
    Transform transform;
    transform.position = createInfo.position;
    transform.rotation = createInfo.rotation;
    transform.scale = createInfo.scale;

    m_modelMatrix = transform.to_mat4();
    m_inverseModelMatrix = glm::inverse(m_modelMatrix);

    m_type = createInfo.type;
}

