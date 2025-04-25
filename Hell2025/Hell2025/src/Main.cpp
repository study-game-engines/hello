/*

███    █▄  ███▄▄▄▄    ▄█        ▄██████▄   ▄█    █▄     ▄████████ ████████▄
███    ███ ███▀▀▀██▄ ███       ███    ███ ███    ███   ███    ███ ███   ▀███
███    ███ ███   ███ ███       ███    ███ ███    ███   ███    █▀  ███    ███
███    ███ ███   ███ ███       ███    ███ ███    ███  ▄███▄▄▄     ███    ███
███    ███ ███   ███ ███       ███    ███ ███    ███ ▀▀███▀▀▀     ███    ███
███    ███ ███   ███ ███       ███    ███ ███    ███   ███    █▄  ███    ███
███    ███ ███   ███ ███▌    ▄ ███    ███ ███    ███   ███    ███ ███   ▄███
████████▀   ▀█   █▀  █████▄▄██  ▀██████▀   ▀██████▀    ██████████ ████████▀

*/

#include "AssetManagement/AssetManager.h"
#include "Backend/Backend.h"
#include "Core/Game.h"
#include "Editor/Editor.h"
#include "Renderer/Renderer.h"
#include "UI/UIBackEnd.h"
#include <iostream>

#include "API/Vulkan/vk_backend.h"

int main2() {
    VulkanBackEnd::Init();
    return 0;
}

int main() {
    std::cout << "We are all alone on life's journey, held captive by the limitations of human consciousness.\n";

    // Init the back-end, sub-systems, and the minimum to render loading screen
    if (!BackEnd::Init(API::OPENGL, WindowedMode::WINDOWED)) {
        std::cout << "BackEnd::Init() FAILED!\n";
        return -1;
    }

    File::ImportModel("res/models/WeatherBoard.model");

    // Program loop
    while (BackEnd::WindowIsOpen()) {

        BackEnd::UpdateSubSystems();
        BackEnd::BeginFrame();

        // Render loading screen
        if (!AssetManager::LoadingComplete()) {
            AssetManager::UpdateLoading();
            Renderer::RenderLoadingScreen();
        
            // Loading complete?
            if (AssetManager::LoadingComplete()) {
                Game::Create();
            }
        }
        // Update/render game
        else {
            BackEnd::UpdateGame();
            Renderer::RenderGame();
        }
        BackEnd::EndFrame();
    }
    return 0;
}