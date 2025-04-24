#include "../GL_renderer.h"
#include "AssetManagement/AssetManager.h"
#include "World/World.h"

namespace OpenGLRenderer {

    void LightCullingPass() {
        OpenGLFrameBuffer* gBuffer = GetFrameBuffer("GBuffer");
        OpenGLShader* shader = GetShader("LightCulling");

        if (!gBuffer) return;
        if (!shader) return;

        size_t lightCount = World::GetLights().size();

        shader->Use();
        shader->SetFloat("u_viewportWidth", gBuffer->GetWidth());
        shader->SetFloat("u_viewportHeight", gBuffer->GetHeight());
        shader->SetInt("u_lightCount", lightCount);
        shader->SetInt("u_tileXCount", gBuffer->GetWidth() / TILE_SIZE);
        shader->SetInt("u_tileYCount", gBuffer->GetHeight() / TILE_SIZE);
        
        glBindTextureUnit(0, gBuffer->GetColorAttachmentHandleByName("WorldSpacePosition"));
        glBindTextureUnit(1, gBuffer->GetColorAttachmentHandleByName("Normal"));

        glDispatchCompute(gBuffer->GetWidth() / TILE_SIZE, gBuffer->GetHeight() / TILE_SIZE, 1);
    }

    void LightingPass() {
        OpenGLFrameBuffer* gBuffer = GetFrameBuffer("GBuffer");
        OpenGLFrameBuffer* finalImageFBO = GetFrameBuffer("FinalImage");
        OpenGLShadowMap* flashLightShadowMapsFBO = GetShadowMap("FlashlightShadowMaps");
        OpenGLShadowCubeMapArray* hiResShadowMaps = GetShadowMapArray("HiRes");
        OpenGLShader* lightingShader = GetShader("Lighting");

        if (!gBuffer) return;
        if (!finalImageFBO) return;
        if (!lightingShader) return;

        lightingShader->Use();

        lightingShader->SetFloat("u_viewportWidth", gBuffer->GetWidth());
        lightingShader->SetFloat("u_viewportHeight", gBuffer->GetHeight());
        lightingShader->SetInt("u_tileXCount", gBuffer->GetWidth() / TILE_SIZE);
        lightingShader->SetInt("u_tileYCount", gBuffer->GetHeight() / TILE_SIZE);

        glBindTextureUnit(0, gBuffer->GetColorAttachmentHandleByName("BaseColor"));
        glBindTextureUnit(1, gBuffer->GetColorAttachmentHandleByName("Normal"));
        glBindTextureUnit(2, gBuffer->GetColorAttachmentHandleByName("RMA"));
        glBindTextureUnit(3, gBuffer->GetDepthAttachmentHandle());
        glBindTextureUnit(4, gBuffer->GetColorAttachmentHandleByName("WorldSpacePosition"));
        glBindTextureUnit(5, finalImageFBO->GetColorAttachmentHandleByName("ViewportIndex"));
        glBindTextureUnit(6, gBuffer->GetColorAttachmentHandleByName("Emissive"));
        glBindTextureUnit(7, AssetManager::GetTextureByName("Flashlight2")->GetGLTexture().GetHandle());
        glBindTextureUnit(8, flashLightShadowMapsFBO->GetDepthTextureHandle());
        //glBindTextureUnit(9, hiResShadowMaps->GetDepthTexture());

        glActiveTexture(GL_TEXTURE9);
        glBindTexture(GL_TEXTURE_CUBE_MAP_ARRAY, hiResShadowMaps->GetDepthTexture());

        glBindImageTexture(0, gBuffer->GetColorAttachmentHandleByName("FinalLighting"), 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA16F);

        //glDispatchCompute((gBuffer->GetWidth() + 7) / 8, (gBuffer->GetHeight() + 7) / 8, 1);

        glDispatchCompute(gBuffer->GetWidth() / TILE_SIZE, gBuffer->GetHeight() / TILE_SIZE, 1);
    }
}