#pragma once
#include "Types/Renderer/TextureArray.h"
#include <vector>
#include <string>

namespace HeightMapManager {
    void Init();
    bool HeightMapExists(const std::string& heightMapName);
    OpenGLTextureArray& GetGLTextureArray();
    std::vector<std::string>& GetHeigthMapNames();
}