#include "HeightMapManager.h"
#include "Backend/Backend.h"
#include "File/File.h"
#include "Tools/ImageTools.h"
#include "Util.h"
#include <glad/glad.h>

namespace HeightMapManager {

    OpenGLTextureArray g_glTextureArray;
    uint32_t g_heightmapCount = 0;
    std::vector<std::string> g_heightMapNames;

    void AllocateMemory(int heightmapCount);


    // Warning, you probably can't create new heightmaps at runtime with the code below, allocate on load heightmaps from disk is solution
    // Actually no, that 16 is the total allowed size

    void Init() {
        AllocateMemory(16);
        LoadHeightMapsFromDisk();
    }

    void AllocateMemory(int heightmapCount) {
        if (BackEnd::GetAPI() == API::OPENGL) {
            g_glTextureArray.AllocateMemory(HEIGHT_MAP_SIZE, HEIGHT_MAP_SIZE, GL_R16F, 1, heightmapCount);
        }
        else if (BackEnd::GetAPI() == API::OPENGL) {
            // TODO
        }
    }

    void LoadHeightMapsFromDisk() {
        std::vector<FileInfo> files = Util::IterateDirectory("res/height_maps/");
        g_heightMapNames.clear();

        // Iterate all found files and upload the data to the gpu
        for (int i = 0; i < files.size(); i++) {
            HeightMapData heightMapData = File::LoadHeightMap(files[i].name + ".heightmap");

            if (BackEnd::GetAPI() == API::OPENGL) {
                g_glTextureArray.SetLayerDataR16(i, heightMapData.data);
            }
            else if (BackEnd::GetAPI() == API::OPENGL) {
                // TODO
            }
            g_heightMapNames.push_back(files[i].name);
        }
    }

    OpenGLTextureArray& GetGLTextureArray() {
        return g_glTextureArray;
    }


    std::vector<std::string>& GetHeigthMapNames() {
        return g_heightMapNames;
    }

    bool HeightMapExists(const std::string& heightMapName) {
        for (const std::string& name : g_heightMapNames) {
            if (name == heightMapName) {
                return true;
            }
        }
        return false;
    }
}