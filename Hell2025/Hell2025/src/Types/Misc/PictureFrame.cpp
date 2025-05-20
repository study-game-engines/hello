#include "PictureFrame.h"
#include "AssetManagement/AssetManager.h"
#include "Util.h"

const std::vector<const char*> bigLandscapeImages = {
    "Picture_RainbowMage",
    "Picture_SHNakedLady",
    "Picture_Minotaur"
};

void PictureFrame::Init(PictureFrameCreateInfo createInfo) {
    m_transform.position = createInfo.position;
    m_transform.rotation = createInfo.rotation;
    m_transform.scale = createInfo.scale;
    m_type = createInfo.type;

    m_renderItems.clear();

    Material* material = AssetManager::GetMaterialByName("PictureFrame0");
    Material* defaultMaterial = AssetManager::GetDefaultMaterial();

    if (m_type == PictureFrameType::BIG_LANDSCAPE) {

        Model* model = AssetManager::GetModelByName("PictureFrame_BigLandscape");

        for (int i = 0; i < 2; i++) {
            uint32_t meshIndex = model->GetMeshIndices()[i];     
            RenderItem& renderItem = m_renderItems.emplace_back();
            renderItem.modelMatrix = m_transform.to_mat4();
            renderItem.inverseModelMatrix = glm::inverse(renderItem.modelMatrix);
            renderItem.meshIndex = meshIndex;
            renderItem.baseColorTextureIndex = material->m_basecolor;
            renderItem.rmaTextureIndex = material->m_rma;
            renderItem.normalMapTextureIndex = material->m_normal;
            renderItem.objectType = (int)ObjectType::PICTURE_FRAME;

            if (i == 0) {
                int random = rand() % bigLandscapeImages.size();
                renderItem.baseColorTextureIndex = AssetManager::GetTextureIndexByName(bigLandscapeImages[random]);
                renderItem.rmaTextureIndex = defaultMaterial->m_rma;
                renderItem.normalMapTextureIndex = defaultMaterial->m_normal;
            }

            Util::UpdateRenderItemAABB(renderItem);
            Util::PackUint64(m_objectId, renderItem.objectIdLowerBit, renderItem.objectIdUpperBit);
        }        
    }

}