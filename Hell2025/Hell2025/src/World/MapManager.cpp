#include "MapManager.h"
#include "CreateInfo.h"
#include "Core/JSON.h"
#include "Editor/Editor.h"
#include "World/SectorManager.h"

namespace MapManager {

    std::vector<MapCreateInfo> g_mapCreateInfoSet;
    MapCreateInfo g_heightMapEditorMap;

    void Init() {
        g_mapCreateInfoSet.clear();

        // Create heightmap editor map
        SetMapName(&g_heightMapEditorMap, "HeightMapEditorMap");
        SetMapWidth(&g_heightMapEditorMap, 3);
        SetMapDepth(&g_heightMapEditorMap, 3);
        SetSectorName(&g_heightMapEditorMap, 0, 0, "HeightMapEditor_SW");
        SetSectorName(&g_heightMapEditorMap, 1, 0, "HeightMapEditor_S");
        SetSectorName(&g_heightMapEditorMap, 2, 0, "HeightMapEditor_SE");
        SetSectorName(&g_heightMapEditorMap, 0, 1, "HeightMapEditor_W");
        SetSectorName(&g_heightMapEditorMap, 1, 1, "HeightMapEditor_Center");
        SetSectorName(&g_heightMapEditorMap, 2, 1, "HeightMapEditor_E");
        SetSectorName(&g_heightMapEditorMap, 0, 2, "HeightMapEditor_NW");
        SetSectorName(&g_heightMapEditorMap, 1, 2, "HeightMapEditor_N");
        SetSectorName(&g_heightMapEditorMap, 2, 2, "HeightMapEditor_NE");

        // Load maps
        // TODO

        // Temporarily hard code a map
        if (!MapExists("TestMap")) {
            NewMap("TestMap");
        }
        MapCreateInfo* createInfo = GetMapCreateInfoByName("TestMap");
        if (createInfo) {
            SetMapWidth(createInfo, 3);
            SetMapDepth(createInfo, 4);
            SetSectorName(createInfo, 0, 0, "TestSector");
            //SetMapSectorLocation(createInfo, 1, 0, "TestSector");
            SetSectorName(createInfo, 0, 1, "Forest");
            SetSectorName(createInfo, 1, 1, "TestSector");
            SetSectorName(createInfo, 2, 1, "TestSector");
            SetSectorName(createInfo, 2, 2, "TestSector");
            SetSectorName(createInfo, 2, 3, "TestSector");
            SetSectorName(createInfo, 1, 3, "TestSector");
            SaveMap("TestMap");
        }
    }

    void NewMap(const std::string& filename) {
        if (!MapExists(filename)) {
            MapCreateInfo& createInfo = g_mapCreateInfoSet.emplace_back();
            SetMapName(&createInfo, filename);
            SetMapWidth(&createInfo, 2);
            SetMapDepth(&createInfo, 3);
            SaveMap(filename);
        } 
        else {
            std::cout << "MapManager::NewMap() failed: map name '" << filename << "' already exists\n";
        }
    }

    void LoadMap(const std::string& filename) {

    }

    void SaveMap(const std::string& filename) {
        MapCreateInfo* createInfo = GetMapCreateInfoByName(filename);
        if (createInfo) {
            JSON::SaveMap("res/maps/" + filename + ".json", *createInfo);
        }
    }

    void SetMapWidth(MapCreateInfo* mapCreateInfo, int width) {
        if (!mapCreateInfo) return;

        // Clamp width between 0 and MAX_MAP_WIDTH
        mapCreateInfo->width = std::min(std::max(0, width), MAX_MAP_WIDTH);

        // Clear all sector names out of the valid range
        for (int x = mapCreateInfo->width; x < MAX_MAP_WIDTH; x++) {
            for (int z = 0; z < MAX_MAP_DEPTH; z++) {
                mapCreateInfo->m_sectorNames[x][z] = "";
            }
        }
    }

    void SetMapDepth(MapCreateInfo* mapCreateInfo, int depth) {
        if (!mapCreateInfo) return;

        // Clamp depth between 0 and MAX_MAP_DEPTH
        mapCreateInfo->depth = std::min(std::max(0, depth), MAX_MAP_DEPTH);

        // Clear all sector names out of the valid range
        for (int x = 0; x < MAX_MAP_WIDTH; x++) {
            for (int z = mapCreateInfo->depth; z < MAX_MAP_DEPTH; z++) {
                mapCreateInfo->m_sectorNames[x][z] = "";
            }
        }
    }

    void SetMapName(MapCreateInfo* createInfo, const std::string& filename) {
        if (!createInfo) return;

        createInfo->name = filename;
    }

    void SetSectorName(MapCreateInfo* mapCreateInfo, int x, int z, const std::string& sectorName) {
        if (!mapCreateInfo) return;

        int mapWidth = mapCreateInfo->width;
        int mapDepth = mapCreateInfo->depth;

        if (x >= 0 && x < mapWidth && z >= 0 && z < mapDepth) {
            mapCreateInfo->m_sectorNames[x][z] = sectorName;
        }
        else {
            std::cout << "MapManager::SetMapSectorLocation() warning: [" << x << "][" << z << "] out of map range [" << mapWidth << "][" << mapDepth << "]\n";
        }
    }

    void ClearSectorName(MapCreateInfo* mapCreateInfo, int x, int z) {
        if (!mapCreateInfo) return;

        int mapWidth = mapCreateInfo->width;
        int mapDepth = mapCreateInfo->depth;

        if (x >= 0 && x < mapWidth && z >= 0 && z < mapDepth) {
            mapCreateInfo->m_sectorNames[x][z] = "";
        }
        else {
            std::cout << "MapManager::ClearSector() warning: [" << x << "][" << z << "] out of map range [" << mapWidth << "][" << mapDepth << "]\n";
        }
    }

    bool MapExists(const std::string& filename) {
        for (const auto& mapCreateInfo : g_mapCreateInfoSet) {
            if (mapCreateInfo.name == filename) {
                return true;
            }
        }
        return false;
    }

    uint32_t GetMapCellCountX(MapCreateInfo* createInfo) {
        return createInfo ? createInfo->width : 0;
    }

    uint32_t GetMapCellCountZ(MapCreateInfo* createInfo) {
        return createInfo ? createInfo->depth : 0;
    }

    bool IsSectorAtLocation(MapCreateInfo* mapCreateInfo, int x, int z) {
        if (!mapCreateInfo) return false;

        int mapWidth = mapCreateInfo->width;
        int mapDepth = mapCreateInfo->depth;

        // Use mapDepth for the z index check.
        if (x >= 0 && x < mapWidth && z >= 0 && z < mapDepth) {
            return mapCreateInfo->m_sectorNames[x][z] != "";
        }
        else {
            std::cout << "MapManager::IsSectorAtLocation() warning: [" << x << "][" << z << "] out of map range [" << mapWidth << "][" << mapDepth << "]\n";
            return false;
        }
    }

    bool IsHeightMapAtLocation(MapCreateInfo* mapCreateInfo, int x, int z) {
        // False if there isn't even a sector
        if (!IsSectorAtLocation(mapCreateInfo, x, z)) return false;

        // Get the sector create info
        const std::string& sectorName = GetSectorNameAtLocation(mapCreateInfo, x, z);
        
        SectorCreateInfo* sectorCreateInfo = SectorManager::GetSectorCreateInfoByName(sectorName);

        // Return true if it has a valid heightmap
        return (sectorCreateInfo && sectorCreateInfo->heightMapName != "");
    }

    int32_t GetHeightMapIndexByHeightMapName(const std::string& heightMapName) {
        int i = 0;
        for (FileInfo& fileInfo : Util::IterateDirectory("res/height_maps/")) {
            if (fileInfo.name == heightMapName) {
                return i;
            }
            i++;
        }

        // Didn't find the heightmap?
        return -1;
    }

    const std::string& GetSectorNameAtLocation(MapCreateInfo* mapCreateInfo, int x, int z) {
        int mapWidth = mapCreateInfo->width;
        int mapDepth = mapCreateInfo->depth;

        // Check if out of range
        if (x < 0 || x >= mapWidth || z < 0 || z >= mapDepth) {
            std::cout << "MapManager::GetSectorNameAtLocation() failed: [" << x << "][" << z << "] out of range of size [" << mapWidth << "][" << mapDepth << "]\n";
            static const std::string emptyStr = "";
            return emptyStr;
        }
        else {
            return mapCreateInfo->m_sectorNames[x][z];
        }
    }

    MapCreateInfo* GetHeightMapEditorMapCreateInfo() {
        return &g_heightMapEditorMap;
    }
    
    MapCreateInfo* GetMapCreateInfoByName(const std::string& name) {
        // Request height map editor map
        if (name == g_heightMapEditorMap.name) {
            return &g_heightMapEditorMap;
        }
        // Requested sector editor map
        //if (name == g_sectorEditorMap.name) {
        //    return &g_sectorEditorMap;
        //}
        // Otherwise, they requested a map from disk
        for (MapCreateInfo& createInfo : g_mapCreateInfoSet) {
            if (createInfo.name == name) {
                return &createInfo;
            }
        }
        return nullptr;
    }
}