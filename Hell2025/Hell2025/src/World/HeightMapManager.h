#pragma once
#include "Renderer/Types/TextureArray.h"
#include <vector>
#include <string>

namespace HeightMapManager {
    void Init();
    bool HeightMapExists(const std::string& heightMapName);
    OpenGLTextureArray& GetGLTextureArray();
    std::vector<std::string>& GetHeigthMapNames();
}