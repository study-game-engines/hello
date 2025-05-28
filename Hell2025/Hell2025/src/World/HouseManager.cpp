#include "HouseManager.h"
#include "Core/JSON.h"
#include "Util.h"

#include <map>

namespace HouseManager {

    std::map<std::string, HouseCreateInfo> g_houses;

    void Init() {
        LoadAllHouseFilesFromDisk();
    }

    void LoadAllHouseFilesFromDisk() {
        g_houses.clear();
        for (FileInfo& fileInfo : Util::IterateDirectory("res/houses/", { "json" })) {
            g_houses[fileInfo.name] = JSON::LoadHouse(fileInfo.path);
        }
    }

    HouseCreateInfo* GetHouseCreateInfoByFilename(const std::string& filename) {
        auto it = g_houses.find(filename);
        if (it == g_houses.end()) {
            return nullptr;
        }
        else {
            return &(it->second);
        }
    }
}