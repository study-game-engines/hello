#include "API/OpenGL/Renderer/GL_Renderer.h"
#include "AssetManagement/AssetManager.h"
#include "Renderer/RenderDataManager.h"
#include "Viewport/ViewportManager.h"

namespace OpenGLRenderer {

    GLuint g_cubeVao = 0;
    void InitDecalCube();

    void BloodScreenSpaceDecalsPass() {
        glFinish();
        glFinish();

        const std::vector<ViewportData>& viewportData = RenderDataManager::GetViewportData();

        if (g_cubeVao == 0) {
            InitDecalCube();
        }

        OpenGLFrameBuffer* gBuffer = GetFrameBuffer("GBuffer");
        OpenGLShader* shader = GetShader("BloodScreenSpaceDecals");

        if (!gBuffer) return;
        if (!shader) return;

        gBuffer->Bind();
        gBuffer->DrawBuffers({ "BaseColor", "RMA" });

        shader->Bind();

        SetRasterizerState("GeometryPass_NonBlended");

        glBindTextureUnit(0, gBuffer->GetColorAttachmentHandleByName("WorldPosition"));
        glBindTextureUnit(1, gBuffer->GetColorAttachmentHandleByName("Normal"));
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, AssetManager::GetTextureByName("BloodDecal7")->GetGLTexture().GetHandle());


        for (int i = 0; i < 4; i++) {
            Viewport* viewport = ViewportManager::GetViewportByIndex(i);

            if (viewport->IsVisible()) {
                OpenGLRenderer::SetViewport(gBuffer, viewport);

                shader->SetMat4("u_projectionView", viewportData[i].projectionView);


                // glDepthMask(GL_FALSE);
                // glEnable(GL_BLEND);
                // glBlendFunc(GL_DST_COLOR, GL_SRC_COLOR);
                // glBlendFunc(GL_DST_COLOR, GL_SRC_COLOR);
                // glBlendEquation(GL_FUNC_ADD);
                // glEnablei(GL_BLEND, 1);
                // glBlendFunci(1, GL_DST_COLOR, GL_SRC_COLOR);
                // glDisablei(GL_BLEND, 1);
                // glCullFace(GL_FRONT);
                // glDisable(GL_DEPTH_TEST);
                //

                // DRAWWWWWWWW A CUBE
                Transform transform;
                transform.position = glm::vec3(2, 0, 2);

                glm::mat4 modelMatrix = transform.to_mat4();

                shader->SetMat4("u_model", modelMatrix);
                shader->SetMat4("u_inverseModel", glm::inverse(modelMatrix));


                glBindVertexArray(g_cubeVao);
                glDrawArrays(GL_TRIANGLES, 0, 36);

            }
        }
    }


    void InitDecalCube() {
        float vertices[] = {
            // positions
            0.5f,  0.5f,  0.5f,
            0.5f,  0.5f, -0.5f,
            -0.5f,  0.5f, -0.5f,
            -0.5f,  0.5f, -0.5f,
            -0.5f,  0.5f,  0.5f,
            0.5f,  0.5f,  0.5f,

            0.5f,  0.5f, -0.5f,
            0.5f, -0.5f, -0.5f,
            -0.5f, -0.5f, -0.5f,
            -0.5f, -0.5f, -0.5f,
            -0.5f,  0.5f, -0.5f,
            0.5f, 0.5f, -0.5f,

            -0.5f, -0.5f,  0.5f,
            0.5f, -0.5f,  0.5f,
            0.5f,  0.5f,  0.5f,
            0.5f,  0.5f,  0.5f,
            -0.5f,  0.5f,  0.5f,
            -0.5f, -0.5f,  0.5f,

            -0.5f,  0.5f,  0.5f,
            -0.5f,  0.5f, -0.5f,
            -0.5f, -0.5f, -0.5f,
            -0.5f, -0.5f, -0.5f,
            -0.5f, -0.5f,  0.5f,
            -0.5f,  0.5f,  0.5f,

            0.5f, -0.5f, -0.5f,
            0.5f,  0.5f, -0.5f,
            0.5f,  0.5f,  0.5f,
            0.5f,  0.5f,  0.5f,
            0.5f, -0.5f,  0.5f,
            0.5f, -0.5f, -0.5f,

            -0.5f, -0.5f, -0.5f,
            0.5f, -0.5f, -0.5f,
            0.5f, -0.5f,  0.5f,
            0.5f, -0.5f,  0.5f,
            -0.5f, -0.5f,  0.5f,
            -0.5f, -0.5f, -0.5f,
        };
        unsigned int VBO;
        glGenVertexArrays(1, &g_cubeVao);
        glGenBuffers(1, &VBO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
        glBindVertexArray(g_cubeVao);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
    }
}