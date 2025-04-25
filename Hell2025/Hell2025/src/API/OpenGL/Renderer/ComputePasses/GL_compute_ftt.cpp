#include "API/OpenGL/Renderer/GL_renderer.h"
#include "Core/Game.h"
#include "Ocean/Ocean.h"

namespace OpenGLRenderer {

    void ComputeOceanFFTPass() {
        OpenGLFrameBuffer* fftFrameBuffer = GetFrameBuffer("FFT");
        OpenGLMeshPatch* oceanMeshPatch = GetOceanMeshPatch();
        OpenGLShader* oceanCalculateSpectrumShader = GetShader("OceanCalculateSpectrum");
        OpenGLShader* oceanUpdateMeshShader = GetShader("OceanUpdateMesh");
        OpenGLShader* oceanUpdateNormalsShader = GetShader("OceanUpdateNormals");
        OpenGLSSBO* fftH0SSBO = GetSSBO("ffth0");
        OpenGLSSBO* fftSpectrumInSSBO = GetSSBO("fftSpectrumInSSBO");
        OpenGLSSBO* fftSpectrumOutSSBO = GetSSBO("fftSpectrumOutSSBO");
        OpenGLSSBO* fftDispInXSSBO = GetSSBO("fftDispInXSSBO");
        OpenGLSSBO* fftDispZInSSBO = GetSSBO("fftDispZInSSBO");
        OpenGLSSBO* fftGradXInSSBO = GetSSBO("fftGradXInSSBO");
        OpenGLSSBO* fftGradZInSSBO = GetSSBO("fftGradZInSSBO");
        OpenGLSSBO* fftDispXOutSSBO = GetSSBO("fftDispXOutSSBO");
        OpenGLSSBO* fftDispZOutSSBO = GetSSBO("fftDispZOutSSBO");
        OpenGLSSBO* fftGradXOutSSBO = GetSSBO("fftGradXOutSSBO");
        OpenGLSSBO* fftGradZOutSSBO = GetSSBO("fftGradZOutSSBO");

        if (!fftFrameBuffer) return;
        if (!fftH0SSBO) return;
        if (!fftSpectrumInSSBO) return;
        if (!fftSpectrumOutSSBO) return;
        if (!fftDispInXSSBO) return;
        if (!fftDispZInSSBO) return;
        if (!fftGradXInSSBO) return;
        if (!fftGradZInSSBO) return;
        if (!fftDispXOutSSBO) return;
        if (!fftDispZOutSSBO) return;
        if (!fftGradXOutSSBO) return;
        if (!fftGradZOutSSBO) return;
        if (!oceanMeshPatch) return;
        if (!oceanCalculateSpectrumShader) return;
        if (!oceanUpdateMeshShader) return;
        if (!oceanUpdateNormalsShader) return;

        const glm::uvec2 oceanSize = Ocean::GetOceanSize();
        const glm::uvec2 meshSize = Ocean::GetMeshSize();
        const glm::vec2 oceanLength = Ocean::GetOceanLength();
        const float gravity = Ocean::GetGravity();
        const float displacementFactor = Ocean::GetDisplacementFactor();

        const GLuint blocksPerSide = 16;
        const GLuint fftBlockSizeX = oceanSize.x / blocksPerSide;
        const GLuint fftBlockSizeY = oceanSize.y / blocksPerSide;
        const GLuint meshBlockSizeX = (meshSize.x + 16 - 1) / blocksPerSide;
        const GLuint meshBlockSizeY = (meshSize.y + 16 - 1) / blocksPerSide;

        // Generate spectrum on GPU
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, fftH0SSBO->GetHandle());

        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, fftSpectrumInSSBO->GetHandle());
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, fftDispInXSSBO->GetHandle());
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, fftDispZInSSBO->GetHandle());
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, fftGradXInSSBO->GetHandle());
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, fftGradZInSSBO->GetHandle());

        oceanCalculateSpectrumShader->Bind();
        oceanCalculateSpectrumShader->SetUvec2("oceanSize", oceanSize);
        oceanCalculateSpectrumShader->SetVec2("oceanLength", oceanLength);
        oceanCalculateSpectrumShader->SetFloat("g", gravity);
        oceanCalculateSpectrumShader->SetFloat("t", Game::GetTotalTime());
        glDispatchCompute(fftBlockSizeX, fftBlockSizeY, 1);
        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

        // Perform FFT
        Ocean::ComputeInverseFFT2D(fftSpectrumInSSBO->GetHandle(), fftSpectrumOutSSBO->GetHandle());
        Ocean::ComputeInverseFFT2D(fftDispInXSSBO->GetHandle(), fftDispXOutSSBO->GetHandle());
        Ocean::ComputeInverseFFT2D(fftDispZInSSBO->GetHandle(), fftDispZOutSSBO->GetHandle());
        Ocean::ComputeInverseFFT2D(fftGradXInSSBO->GetHandle(), fftGradXOutSSBO->GetHandle());
        Ocean::ComputeInverseFFT2D(fftGradZInSSBO->GetHandle(), fftGradZOutSSBO->GetHandle());

        // Update mesh position
        glBindImageTexture(0, fftFrameBuffer->GetColorAttachmentHandleByName("Height"), 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_R32F);

        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, fftSpectrumOutSSBO->GetHandle());
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, fftDispXOutSSBO->GetHandle());
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, fftDispZOutSSBO->GetHandle());
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, oceanMeshPatch->GetVBO());
        oceanUpdateMeshShader->Bind();
        oceanUpdateMeshShader->SetUvec2("u_oceanSize", oceanSize);
        oceanUpdateMeshShader->SetUvec2("u_meshSize", meshSize);
        oceanUpdateMeshShader->SetFloat("u_dispFactor", displacementFactor);
        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
        glDispatchCompute(meshBlockSizeX, meshBlockSizeY, 1);

        // Update normals
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, fftSpectrumOutSSBO->GetHandle());
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, fftGradXOutSSBO->GetHandle());
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, fftGradZOutSSBO->GetHandle());
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, oceanMeshPatch->GetVBO());
        oceanUpdateNormalsShader->Bind();
        oceanUpdateNormalsShader->SetUvec2("u_oceanSize", oceanSize);
        oceanUpdateNormalsShader->SetUvec2("u_meshSize", meshSize);
        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT | GL_VERTEX_ATTRIB_ARRAY_BARRIER_BIT);
        glDispatchCompute(meshBlockSizeX, meshBlockSizeY, 1);
        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT | GL_VERTEX_ATTRIB_ARRAY_BARRIER_BIT);   
    }
}