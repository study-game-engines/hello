#pragma once
#include "CreateInfo.h"

namespace MapManager {
    void Init();
    void NewMap(const std::string& filename);
    void LoadMap(const std::string& filename);
    void SaveMap(const std::string& filename);

    void SetMapWidth(MapCreateInfo* createInfo, int width);
    void SetMapDepth(MapCreateInfo* createInfo, int depth);
    void SetMapName(MapCreateInfo* createInfo, const std::string& filename);
    void SetSectorName(MapCreateInfo* createInfo, int x, int z, const std::string& sectorName);
    void ClearSectorName(MapCreateInfo* createInfo, int x, int z);

    bool MapExists(const std::string& filename);
    bool IsHeightMapAtLocation(MapCreateInfo* createInfo, int x, int z);
    bool IsSectorAtLocation(MapCreateInfo* createInfo, int x, int z);
    int32_t GetHeightMapIndexByHeightMapName(const std::string& heightMapName);
    uint32_t GetMapCellCountX(MapCreateInfo* createInfo);
    uint32_t GetMapCellCountZ(MapCreateInfo* createInfo);
    const std::string& GetSectorNameAtLocation(MapCreateInfo* createInfo, int x, int z);

    MapCreateInfo* GetHeightMapEditorMapCreateInfo();
    //MapCreateInfo* GetSectorEditorMapCreateInfo();
    MapCreateInfo* GetMapCreateInfoByName(const std::string& name);

    //void PrintMapCreateInfoDebugInfo(MapCreateInfo* createInfo);
}