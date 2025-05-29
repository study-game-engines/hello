#pragma once
#include "HellFunctionTypes.h"

namespace Callbacks {
    void NewHeightMap(const std::string& filename);
    void NewHouse(const std::string& filename);
    void NewMap(const std::string& filename);
    void NewSector(const std::string& filename);

    void OpenMap(const std::string& filename);
    void OpenHeightMap(const std::string& filename);
    void OpenHouse(const std::string& filename);
    void OpenSector(const std::string& filename);

    void SaveHouse();

    void SaveHeightMaps();

    void TestCurrentMap();

    void OpenHouseEditor();
    void OpenHeightMapEditor();
    void OpenMapEditor();
    void OpenSectorEditor();
    void OpenWeaponsEditor();

    void SaveSector();
    void RevertEditorSector();

    void QuitProgram();

    // House editor
    void BeginAddingDoor();
    void BeginAddingPictureFrame();
    void BeginAddingTree(); 
    void BeginAddingBlackBerries();
    void BeginAddingWall();
    void BeginAddingWindow();
}