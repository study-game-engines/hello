#include "../GL_renderer.h" 
#include "../../GL_backend.h"
#include "AssetManagement/AssetManager.h"
#include "BackEnd/Backend.h"
#include "Viewport/ViewportManager.h"
#include "Editor/Editor.h"
#include "Renderer/RenderDataManager.h"
#include "Modelling/Clipping.h"
#include "Modelling/Unused/Modelling.h"
#include "World/World.h"

namespace OpenGLRenderer {

    void HouseGeometryPass() {
        OpenGLFrameBuffer* gBuffer = GetFrameBuffer("GBuffer");
        OpenGLShader* shader = GetShader("DebugTextured");

        if (!gBuffer) return;
        if (!shader) return;

        gBuffer->Bind();
        gBuffer->DrawBuffers({ "BaseColor", "Normal", "RMA", "WorldPosition", "Emissive" });
        SetRasterizerState("GeometryPass_NonBlended");

        shader->Bind();
        shader->SetMat4("u_model", glm::mat4(1));

        MeshBuffer& houseMeshBuffer = World::GetHouseMeshBuffer();
        OpenGLMeshBuffer& glHouseMeshBuffer = houseMeshBuffer.GetGLMeshBuffer();

        glBindVertexArray(glHouseMeshBuffer.GetVAO());

        for (int i = 0; i < 4; i++) {
            Viewport* viewport = ViewportManager::GetViewportByIndex(i);
            if (!viewport->IsVisible()) continue;
            if (glHouseMeshBuffer.GetIndexCount() <= 0) continue;

            OpenGLRenderer::SetViewport(gBuffer, viewport);
            shader->SetInt("u_viewportIndex", i);

            const std::vector<HouseRenderItem>& renderItems = RenderDataManager::GetHouseRenderItems();

            for (const HouseRenderItem& renderItem : renderItems) {
                int indexCount = renderItem.indexCount;
                int baseVertex = renderItem.baseVertex;
                int baseIndex = renderItem.baseIndex;

                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, AssetManager::GetTextureByIndex(renderItem.baseColorTextureIndex)->GetGLTexture().GetHandle());
                glActiveTexture(GL_TEXTURE1);
                glBindTexture(GL_TEXTURE_2D, AssetManager::GetTextureByIndex(renderItem.normalMapTextureIndex)->GetGLTexture().GetHandle());
                glActiveTexture(GL_TEXTURE2);
                glBindTexture(GL_TEXTURE_2D, AssetManager::GetTextureByIndex(renderItem.rmaTextureIndex)->GetGLTexture().GetHandle());
                glDrawElementsBaseVertex(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, (void*)(sizeof(unsigned int) * baseIndex), baseVertex);
            }
        }
    }

    void GeometryPass() {
        const DrawCommandsSet& drawInfoSet = RenderDataManager::GetDrawInfoSet();

        OpenGLFrameBuffer* gBuffer = GetFrameBuffer("GBuffer");
        OpenGLShader* shader = GetShader("GBuffer");
        OpenGLShader* editorMeshShader = GetShader("EditorMesh");

        if (!gBuffer) return;
        if (!shader) return;
        if (!editorMeshShader) return;

        gBuffer->Bind();
        gBuffer->DrawBuffers({ "BaseColor", "Normal", "RMA", "WorldPosition" });

        const std::vector<ViewportData>& viewportData = RenderDataManager::GetViewportData();

        glBindVertexArray(OpenGLBackEnd::GetVertexDataVAO());

        shader->Bind();
        gBuffer->DrawBuffers({ "BaseColor", "Normal", "RMA", "WorldPosition", "Emissive" });
        SetRasterizerState("GeometryPass_NonBlended");

        OpenGLFrameBuffer* decalMasksFBO = GetFrameBuffer("DecalMasks");
        glActiveTexture(GL_TEXTURE6);
        glBindTexture(GL_TEXTURE_2D, decalMasksFBO->GetColorAttachmentHandleByName("DecalMask0"));
        glActiveTexture(GL_TEXTURE7);
        glBindTexture(GL_TEXTURE_2D, AssetManager::GetTextureByName("KangarooBlood_ALB")->GetGLTexture().GetHandle());
        glActiveTexture(GL_TEXTURE8);
        glBindTexture(GL_TEXTURE_2D, AssetManager::GetTextureByName("KangarooBlood_NRM")->GetGLTexture().GetHandle());
        glActiveTexture(GL_TEXTURE9);
        glBindTexture(GL_TEXTURE_2D, AssetManager::GetTextureByName("KangarooBlood_RMA")->GetGLTexture().GetHandle());

        //glActiveTexture(GL_TEXTURE7);
        //glBindTexture(GL_TEXTURE_2D, AssetManager::GetTextureByName("KangarooFlesh_ALB")->GetGLTexture().GetHandle());
        //glActiveTexture(GL_TEXTURE8);
        //glBindTexture(GL_TEXTURE_2D, AssetManager::GetTextureByName("KangarooFlesh_NRM")->GetGLTexture().GetHandle());
        //glActiveTexture(GL_TEXTURE9);
        //glBindTexture(GL_TEXTURE_2D, AssetManager::GetTextureByName("KangarooFlesh_RMA")->GetGLTexture().GetHandle());

        for (int i = 0; i < 4; i++) {
            Viewport* viewport = ViewportManager::GetViewportByIndex(i);
            if (viewport->IsVisible()) {
                OpenGLRenderer::SetViewport(gBuffer, viewport);
                if (BackEnd::RenderDocFound()) {
                    SplitMultiDrawIndirect(shader, drawInfoSet.geometry[i]);
                }
                else {
                    MultiDrawIndirect(drawInfoSet.geometry[i]);
                }
            }
        }

        gBuffer->DrawBuffers({ "BaseColor" });
        SetRasterizerState("GeometryPass_Blended");

        for (int i = 0; i < 4; i++) {
            Viewport* viewport = ViewportManager::GetViewportByIndex(i);
            if (viewport->IsVisible()) {
                OpenGLRenderer::SetViewport(gBuffer, viewport);
                if (BackEnd::RenderDocFound()) {
                    SplitMultiDrawIndirect(shader, drawInfoSet.geometryBlended[i]);
                }
                else {
                    MultiDrawIndirect(drawInfoSet.geometryBlended[i]);
                }
            }
        }

        shader->Bind();
        gBuffer->DrawBuffers({ "BaseColor", "Normal", "RMA", "WorldPosition", "Emissive" });
        SetRasterizerState("GeometryPass_NonBlended");

        glBindVertexArray(OpenGLBackEnd::GetSkinnedVertexDataVAO());
        glBindBuffer(GL_ARRAY_BUFFER, OpenGLBackEnd::GetSkinnedVertexDataVBO());
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, OpenGLBackEnd::GetWeightedVertexDataEBO());

        for (int i = 0; i < 4; i++) {
            Viewport* viewport = ViewportManager::GetViewportByIndex(i);
            if (viewport->IsVisible()) {
                OpenGLRenderer::SetViewport(gBuffer, viewport);
                if (BackEnd::RenderDocFound()) {
                    SplitMultiDrawIndirect(shader, drawInfoSet.skinnedGeometry.perViewport[i]);
                }
                else {
                    MultiDrawIndirect(drawInfoSet.skinnedGeometry.perViewport[i]);
                }
            }
        }

        glBindVertexArray(0);

        
    }
}

