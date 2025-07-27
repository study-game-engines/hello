#include "GL_renderer.h"

namespace OpenGLRenderer {
    
    void BlitDebugTextures() {
        // Render decal painting shit
        if (false) {
            DebugBlitFrameBufferTexture("DecalPainting", "UVMap", 0, 0, 480, 480);
            DebugBlitFrameBufferTexture("DecalMasks", "DecalMask0", 0, 480, 480, 480);
        }

        // World heightmap
        if (false) {
            DebugBlitFrameBufferTexture("World", "HeightMap", 0, 0, 480, 480);
        }

        // Ocean
        if (false) {
            DebugBlitFrameBufferTexture("FFT_band0", "Displacement", 0, 0, 300, 300);
            DebugBlitFrameBufferTexture("FFT_band0", "Normals", 300, 0, 300, 300);
            DebugBlitFrameBufferTexture("FFT_band1", "Displacement", 0, 300, 300, 300);
            DebugBlitFrameBufferTexture("FFT_band1", "Normals", 300, 300, 300, 300);
        }
    }

    void DebugBlitFrameBufferTexture(const std::string& frameBufferName, const std::string& attachmentName, GLint dstX, GLint dstY, GLint width, GLint height) {
        OpenGLFrameBuffer* frameBuffer = GetFrameBuffer(frameBufferName);
        if (!frameBuffer) {
            std::cout << "DebugBlitFrameBufferTexture() failed because frameBufferName '" << frameBufferName << "' was not found\n";
            return;
        }

        GLenum attachment = frameBuffer->GetColorAttachmentSlotByName(attachmentName.c_str());
        if (attachment == GL_INVALID_VALUE) {
            std::cout << "DebugBlitFrameBufferTexture() failed because attachmentName '" << attachmentName << "' was not found in frameBuffer '" << frameBufferName << "'\n";
            return;
        }

        glBindFramebuffer(GL_READ_FRAMEBUFFER, frameBuffer->GetHandle());
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
        glReadBuffer(attachment);
        glDrawBuffer(GL_BACK);
        glBlitFramebuffer(0, 0, frameBuffer->GetWidth(), frameBuffer->GetHeight(), dstX, dstY, dstX + width, dstY + height, GL_COLOR_BUFFER_BIT, GL_NEAREST);
    }
}