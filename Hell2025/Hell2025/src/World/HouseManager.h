#pragma once
#include "CreateInfo.h"

namespace HouseManager {

    void Init();
    void LoadAllHouseFilesFromDisk();

    HouseCreateInfo* GetHouseCreateInfoByFilename(const std::string& filename);

    //HouseCreateInfo GetTestHouseCreateInfo();
}