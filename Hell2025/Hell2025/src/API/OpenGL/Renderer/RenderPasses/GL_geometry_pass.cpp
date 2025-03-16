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

    void GeometryPass() {
        const DrawCommandsSet& drawInfoSet = RenderDataManager::GetDrawInfoSet();

        OpenGLFrameBuffer* gBuffer = GetFrameBuffer("GBuffer");
        OpenGLShader* shader = GetShader("GBuffer");
        OpenGLShader* editorMeshShader = GetShader("EditorMesh");

        if (!gBuffer) return;
        if (!shader) return;
        if (!editorMeshShader) return;

        gBuffer->Bind();
        gBuffer->DrawBuffers({ "BaseColor", "Normal", "RMA", "MousePick", "WorldSpacePosition" });

        const std::vector<ViewportData>& viewportData = RenderDataManager::GetViewportData();

        glBindVertexArray(OpenGLBackEnd::GetVertexDataVAO());

        shader->Use();
        gBuffer->DrawBuffers({ "BaseColor", "Normal", "RMA", "MousePick", "WorldSpacePosition", "Emissive" });
        SetRasterizerState("GeometryPass_NonBlended");

        for (int i = 0; i < 4; i++) {
            Viewport* viewport = ViewportManager::GetViewportByIndex(i);
            if (viewport->IsVisible()) {
                OpenGLRenderer::SetViewport(gBuffer, viewport);
                if (BackEnd::RenderDocFound()) {
                    SplitMultiDrawIndirect(shader, drawInfoSet.geometry.perViewport[i]);
                }
                else {
                    MultiDrawIndirect(drawInfoSet.geometry.perViewport[i]);
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
                    SplitMultiDrawIndirect(shader, drawInfoSet.geometryBlended.perViewport[i]);
                }
                else {
                    MultiDrawIndirect(drawInfoSet.geometryBlended.perViewport[i]);
                }
            }
        }

        shader->Use();
        gBuffer->DrawBuffers({ "BaseColor", "Normal", "RMA", "MousePick", "WorldSpacePosition", "Emissive" });
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


        EditableMesh& editableMesh = Modelling::GetEditableMesh();
        OpenGLDetachedMesh& glMesh = editableMesh.m_glMesh;
        OpenGLShader* debugShader = GetShader("DebugTextured");

        debugShader->Use();

        Material* material = AssetManager::GetDefaultMaterial();
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, AssetManager::GetTextureByIndex(material->m_basecolor)->GetGLTexture().GetHandle());
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, AssetManager::GetTextureByIndex(material->m_normal)->GetGLTexture().GetHandle());
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, AssetManager::GetTextureByIndex(material->m_rma)->GetGLTexture().GetHandle());

        for (int i = 0; i < 4; i++) {
            Viewport* viewport = ViewportManager::GetViewportByIndex(i);
            if (viewport->IsVisible()) {
                OpenGLRenderer::SetViewport(gBuffer, viewport);
                debugShader->SetInt("u_viewportIndex", i);
                debugShader->SetMat4("u_model", glm::mat4(1));
                OpenGLDetachedMesh& mesh = glMesh;
                glBindVertexArray(mesh.GetVAO());
                if (mesh.GetIndexCount() > 0) {
                    glDrawElements(GL_TRIANGLES, mesh.GetIndexCount(), GL_UNSIGNED_INT, 0);
                }
            }
        }




        debugShader->Use();
        debugShader->SetMat4("u_model", glm::mat4(1));

        OpenGLDetachedMesh& wallMesh = World::GetHouseMesh();
        glBindVertexArray(wallMesh.GetVAO());

        //glDisable(GL_CULL_FACE);

        for (int i = 0; i < 4; i++) {
            Viewport* viewport = ViewportManager::GetViewportByIndex(i);
            if (!viewport->IsVisible()) continue;
            if (wallMesh.GetIndexCount() <= 0) continue;

            OpenGLRenderer::SetViewport(gBuffer, viewport);
            debugShader->SetInt("u_viewportIndex", i);

            std::vector<HouseRenderItem> renderItems;

            for (Wall& wall : World::GetWalls()) {
                for (HouseRenderItem& renderItem : wall.GetRenderItems()) {
                    renderItems.push_back(renderItem);
                }
            }
            for (Floor& floor : World::GetFloors()) {
                renderItems.push_back(floor.GetRenderItem());
            }

            for (HouseRenderItem& renderItem : renderItems) {
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


        Transform transform;
        transform.position = glm::vec3(17.0f, 1.5f, 20.0f);
        transform.rotation = glm::vec3(0.5f, 0.1f, 0.5f);
        transform.scale = glm::vec3(1.5f, 1.25f, 1.25);

        debugShader->SetMat4("u_model", transform.to_mat4());

        glBindVertexArray(OpenGLBackEnd::GetVertexDataVAO());


        //Mesh* cubeMesh = AssetManager::GetCubeMesh();
        //glDrawElementsBaseVertex(GL_TRIANGLES, cubeMesh->indexCount, GL_UNSIGNED_INT, (void*)(sizeof(unsigned int)* cubeMesh->baseIndex), cubeMesh->baseVertex);

    }
}

