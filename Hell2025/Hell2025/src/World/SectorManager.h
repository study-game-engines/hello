#pragma once
#include "CreateInfo.h"

namespace SectorManager {

    void Init();
    void NewSector(const std::string& name);
    void UpdateSectorFromDisk(const std::string& filename);
    //void LoadSector();
    void SaveSector(const std::string& name);

    const std::string& GetSectorHeightMapName(const std::string& sectorName);
    void SetSectorHeightMap(const std::string& sectorName, const std::string& heightMapName);
    SectorCreateInfo* GetSectorCreateInfoByName(const std::string& name);

}