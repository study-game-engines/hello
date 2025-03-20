#pragma once
#include "HellTypes.h"
#include "Renderer/Types/Model.hpp"

struct Trim {
    void Init(Transform transform, const std::string& modelName, const std::string& materialName);
    void SubmitRenderItem();

private:
    Transform m_transform;
    Material* m_material;
    Model* m_model;
    RenderItem m_renderItem;

    void UpdateRenderItem();
};