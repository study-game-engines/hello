#include "World.h"
#include "Core/JSON.h"

namespace World {

    SectorCreateInfo CreateSectorInfoFromWorldObjects() {
        SectorCreateInfo sectorCreateInfo;

        for (Light& light : GetLights()) {
            sectorCreateInfo.lights.emplace_back(light.GetCreateInfo());
        }
        for (PickUp& pickUp : GetPickUps()) {
            sectorCreateInfo.pickUps.emplace_back(pickUp.GetCreateInfo());
        }
        for (Tree& tree : GetTrees()) {
            sectorCreateInfo.trees.emplace_back(tree.GetCreateInfo());
        }

        return sectorCreateInfo;
    }

    void SaveSector(SectorCreateInfo sectorCreateInfo) {
        std::string filePath = "res/sectors/" + sectorCreateInfo.sectorName + ".json";
        JSON::SaveSector(filePath, sectorCreateInfo);
    }
}