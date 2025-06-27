#include "API/OpenGL/Renderer/GL_renderer.h"
#include "API/OpenGL/GL_backend.h"
#include "Bvh/Gpu/Bvh.h"
#include "Renderer/RenderDataManager.h"
#include "Renderer/GlobalIllumination.h"
#include "Viewport/ViewportManager.h"
#include "World/World.h"

namespace OpenGLRenderer {

    GLuint g_pointCloudVao = 0;
    GLuint g_pointCloudVbo = 0;

    void UploadPointCloud();
    void RaytraceScene();

    void UpdateGlobalIllumintation() {
        if (GlobalIllumination::PointCloudNeedsGpuUpdate()) {
            UploadPointCloud();
        }

        OpenGLSSBO* triangleDataSSBO = GetSSBO("TriangleData");
        OpenGLSSBO* sceneBvhSSBO = GetSSBO("SceneBvh");
        OpenGLSSBO* meshesBvhSSBO = GetSSBO("MeshesBvh");
        OpenGLSSBO* entityInstancesSSBO = GetSSBO("EntityInstances");
        OpenGLSSBO* lightsSSBO = GetSSBO("Lights");
        
        uint64_t sceneBvhId = GlobalIllumination::GetSceneBvhId();

        const std::vector<BvhNode>& sceneNodes = GlobalIllumination::GetSceneNodes();
        const std::vector<BvhNode>& meshBvhNodes = Bvh::Gpu::GetMeshGpuBvhNodes();
        const std::vector<GpuPrimitiveInstance>& entityInstances = Bvh::Gpu::GetGpuEntityInstances(sceneBvhId);
        const std::vector<float>& triData = Bvh::Gpu::GetTriangleData();

        UpdateSSBO("SceneBvh", sceneNodes.size() * sizeof(BvhNode), &sceneNodes[0]);
        UpdateSSBO("MeshesBvh", meshBvhNodes.size() * sizeof(BvhNode), &meshBvhNodes[0]);
        UpdateSSBO("EntityInstances", entityInstances.size() * sizeof(GpuPrimitiveInstance), &entityInstances[0]);
        UpdateSSBO("TriangleData", triData.size() * sizeof(float), &triData[0]);

        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, g_pointCloudVbo);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, triangleDataSSBO->GetHandle());
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, sceneBvhSSBO->GetHandle());
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, meshesBvhSSBO->GetHandle());
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, entityInstancesSSBO->GetHandle());
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, lightsSSBO->GetHandle());
    }

    void UploadPointCloud() {
        if (g_pointCloudVao == 0) {
            glGenVertexArrays(1, &g_pointCloudVao);
            glGenBuffers(1, &g_pointCloudVbo);
        }

        std::vector<CloudPoint>& pointCloud = GlobalIllumination::GetPointClound();

        glBindBuffer(GL_ARRAY_BUFFER, g_pointCloudVbo);
        glBindVertexArray(g_pointCloudVao);
        glBufferData(GL_ARRAY_BUFFER, sizeof(CloudPoint) * pointCloud.size(), pointCloud.data(), GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(CloudPoint), (void*)offsetof(CloudPoint, position));
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(CloudPoint), (void*)offsetof(CloudPoint, normal));
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(CloudPoint), (void*)offsetof(CloudPoint, directLighting));

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);

        GlobalIllumination::SetPointCloudNeedsGpuUpdateState(false);
        std::cout << "Uploaded point cloud to GPU\n";
    }

    void PointCloudDirectLighting() {
        OpenGLShader* shader = GetShader("PointCloudLighting");
        shader->Bind();
        shader->SetInt("u_lightCount", World::GetLightCount());

        GLuint numGroupsX = (GlobalIllumination::GetPointClound().size() + 127) / 128;

        glDispatchCompute(numGroupsX, 1, 1);
        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

        glFinish();

        RaytraceScene();
    }

    void RaytraceScene() {
        OpenGLShader* shader = GetShader("RaytraceScene");
        OpenGLFrameBuffer* miscFullSizeFBO = GetFrameBuffer("MiscFullSize");

        if (!miscFullSizeFBO) return;
        if (!shader) return;

        const std::vector<ViewportData>& viewportData = RenderDataManager::GetViewportData();

        shader->Bind();
        shader->SetMat4("u_projectionMatrix", viewportData[0].projection);
        shader->SetMat4("u_viewMatrix", viewportData[0].view);

        glBindImageTexture(0, miscFullSizeFBO->GetColorAttachmentHandleByName("RaytracedScene"), 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA8);

        glDispatchCompute(miscFullSizeFBO->GetWidth() / 8, miscFullSizeFBO->GetHeight() / 8, 1);
    }

    void DrawPointCloud() {
        if (g_pointCloudVao == 0) return;

        OpenGLShader* shader = GetShader("DebugPointCloud");
        OpenGLFrameBuffer* gBuffer = GetFrameBuffer("GBuffer");

        if (!gBuffer) return;
        if (!shader) return;

        const std::vector<ViewportData>& viewportData = RenderDataManager::GetViewportData();

        gBuffer->Bind();
        gBuffer->DrawBuffer("FinalLighting");

        shader->Bind();

        glEnable(GL_DEPTH_TEST);
        glDisable(GL_CULL_FACE);
        glDisable(GL_BLEND);

        std::vector<CloudPoint>& pointCloud = GlobalIllumination::GetPointClound();
        int pointCloudCount = pointCloud.size();

        for (int i = 0; i < 4; i++) {
            Viewport* viewport = ViewportManager::GetViewportByIndex(i);
            if (!viewport->IsVisible()) continue;

            OpenGLRenderer::SetViewport(gBuffer, viewport);
            shader->SetInt("u_viewportIndex", i);
            shader->SetMat4("u_projectionView", viewportData[i].projectionView);

            glBindVertexArray(g_pointCloudVao);
            glDrawArrays(GL_POINTS, 0, pointCloudCount);
            glBindVertexArray(0);
        }
    }

}