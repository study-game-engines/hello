#include "../GL_renderer.h"
#include "AssetManagement/AssetManager.h"
#include "World/World.h"

namespace OpenGLRenderer {

    void PostProcessingPass() {
        OpenGLFrameBuffer* gBuffer = GetFrameBuffer("GBuffer");
        OpenGLShader* shader = GetShader("PostProcessing");

        if (!gBuffer) return;
        if (!shader) return;

        shader->Bind();
        glBindImageTexture(0, gBuffer->GetColorAttachmentHandleByName("FinalLighting"), 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA16F);
        glBindImageTexture(1, gBuffer->GetColorAttachmentHandleByName("Normal"), 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA16F);

        glDispatchCompute(gBuffer->GetWidth() / 8, gBuffer->GetHeight() / 8, 1);
    }
}