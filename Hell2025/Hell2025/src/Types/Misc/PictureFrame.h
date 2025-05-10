#pragma once
#include "HellTypes.h"
#include "CreateInfo.h"

struct PictureFrame {
    PictureFrame() = default;
    void Init(PictureFrameCreateInfo createInfo);

    const uint64_t& GetObjectId() const                     { return m_objectId; }
    const Transform& GetTransform() const                   { return m_transform; }
    const PictureFrameType& GetType() const                 { return m_type; }
    const std::vector<RenderItem>& GetRenderItems() const   { return m_renderItems; }

private:
    uint64_t m_objectId = 0;
    Transform m_transform;  
    PictureFrameType m_type;
    std::vector<RenderItem> m_renderItems;
};