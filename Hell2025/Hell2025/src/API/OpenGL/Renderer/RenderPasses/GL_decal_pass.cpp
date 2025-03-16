#include "API/OpenGL/Renderer/GL_renderer.h"
#include "Core/Game.h"
#include "Renderer/RenderDataManager.h"
#include "Viewport/ViewportManager.h"`
#include "World/World.h"`

namespace OpenGLRenderer {

    void DecalPass() {
        SetRasterizerState("DecalPass");

        const std::vector<ViewportData>& viewportData = RenderDataManager::GetViewportData();

        OpenGLShader* shader = GetShader("Decals");
        OpenGLFrameBuffer* gBuffer = GetFrameBuffer("GBuffer");

        shader->Use();

        gBuffer->Bind();
        gBuffer->DrawBuffer("FinalLighting");

        glBindTextureUnit(0, gBuffer->GetDepthAttachmentHandle());


        Material* material = AssetManager::GetMaterialByName("BulletHole_Glass");

        std::vector<Decal>& decal = World::GetDecals();

        for (int i = 0; i < 4; i++) {
            Viewport* viewport = ViewportManager::GetViewportByIndex(i);
            if (!viewport->IsVisible()) continue;

            OpenGLRenderer::SetViewport(gBuffer, viewport);
            shader->SetInt("u_viewportIndex", i);

            Player* player = Game::GetLocalPlayerByIndex(i);

            for (Decal& decal : decal) {

                Mesh* mesh = AssetManager::GetQuadZFacingMesh();
                if (!mesh) continue;
                                
                shader->SetMat4("u_modelMatrix", decal.GetModelMatrix());

                 glActiveTexture(GL_TEXTURE0);
                 glBindTexture(GL_TEXTURE_2D, AssetManager::GetTextureByIndex(material->m_basecolor)->GetGLTexture().GetHandle());
                 glActiveTexture(GL_TEXTURE1);
                 glBindTexture(GL_TEXTURE_2D, AssetManager::GetTextureByIndex(material->m_normal)->GetGLTexture().GetHandle());
                 glActiveTexture(GL_TEXTURE2);
                 glBindTexture(GL_TEXTURE_2D, AssetManager::GetTextureByIndex(material->m_rma)->GetGLTexture().GetHandle());

                glDrawElementsBaseVertex(GL_TRIANGLES, mesh->indexCount, GL_UNSIGNED_INT, (void*)(sizeof(unsigned int) * mesh->baseIndex), mesh->baseVertex);

            }
        }
    }
}