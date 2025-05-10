#include "World.h"
#include "AssetManagement/AssetManager.h"
#include "Core/Game.h"
#include "Renderer/RenderDataManager.h"



#include "Renderer/Renderer.h"
#include "Input/Input.h"

namespace World {

    //std::vector<RenderItem> g_renderItems;
    std::vector<RenderItem> g_renderItemsBlended;
    std::vector<RenderItem> g_renderItemsAlphaDiscarded;
    std::vector<RenderItem> g_renderItemsHairTopLayer;
    std::vector<RenderItem> g_renderItemsHairBottomLayer;
    std::vector<RenderItem> g_skinnedRenderItems;

    void SubmitRenderItems() {
        std::vector<AnimatedGameObject>& animatedGameObjects = GetAnimatedGameObjects();
        std::vector<BulletCasing>& bulletCasings = GetBulletCasings();
        std::vector<Decal>& decals = GetDecals();
        std::vector<Door>& doors = GetDoors();
        std::vector<GameObject>& gameObjects = GetGameObjects();
        std::vector<Light>& lights = GetLights();
        std::vector<PickUp>& pickUps = GetPickUps();
        std::vector<Piano>& pianos = GetPianos();
        std::vector<Tree>& trees = GetTrees();
        std::vector<Wall>& walls = GetWalls();
        std::vector<Window>& windows = GetWindows();

        for (GameObject& gameObject : gameObjects) {
            //gameObject.UpdateRenderItems();
            //gameObject.SubmitRenderItems();

            // Selected outline?
            if (gameObject.IsSelected()) {
                RenderDataManager::SubmitOutlineRenderItems(gameObject.GetRenderItems());
                RenderDataManager::SubmitOutlineRenderItems(gameObject.GetRenderItemsHairTopLayer());
                RenderDataManager::SubmitOutlineRenderItems(gameObject.GetRenderItemsHairBottomLayer());
                break;
            }
        }

        // Clear global render item vectors
        //g_renderItems.clear();
        g_skinnedRenderItems.clear();
        g_renderItemsBlended.clear();
        g_renderItemsAlphaDiscarded.clear();
        g_renderItemsHairTopLayer.clear();
        g_renderItemsHairBottomLayer.clear();

        for (PickUp& pickUp : pickUps) {
            RenderDataManager::SubmitRenderItems(pickUp.GetRenderItems());
        }

        for (Decal& decal : decals) {
            decal.SubmitRenderItem();
        }

        for (Plane& housePlane : GetPlanes()) {
            housePlane.SubmitRenderItem();
        }

        // Doors
        for (Door& door : doors) {
            door.SubmitRenderItems();
        }

        for (Piano& piano : pianos) {
            piano.SubmitRenderItems();
        }

        // Window
        for (Window& window : windows) {
            window.SubmitRenderItems();
        }

        // Trees
        for (Tree& tree : trees) {
            RenderDataManager::SubmitRenderItems(tree.GetRenderItems());

            // Selected outline?
            if (tree.IsSelected()) {
                RenderDataManager::SubmitOutlineRenderItems(tree.GetRenderItems());
            }
        }

        // Update each GameObject and collect render items
        for (GameObject& gameObject : gameObjects) {
            continue;
            continue;
            continue;
            continue;
            // Merge render items into global vectors
            //g_renderItems.insert(g_renderItems.end(), gameObject.GetRenderItems().begin(), gameObject.GetRenderItems().end());
            g_renderItemsBlended.insert(g_renderItemsBlended.end(), gameObject.GetRenderItemsBlended().begin(), gameObject.GetRenderItemsBlended().end());
            g_renderItemsAlphaDiscarded.insert(g_renderItemsAlphaDiscarded.end(), gameObject.GetRenderItemsAlphaDiscarded().begin(), gameObject.GetRenderItemsAlphaDiscarded().end());
            g_renderItemsHairTopLayer.insert(g_renderItemsHairTopLayer.end(), gameObject.GetRenderItemsHairTopLayer().begin(), gameObject.GetRenderItemsHairTopLayer().end());
            g_renderItemsHairBottomLayer.insert(g_renderItemsHairBottomLayer.end(), gameObject.GetRenderItemsHairBottomLayer().begin(), gameObject.GetRenderItemsHairBottomLayer().end());
        }

        // Lights
        for (Light& light : lights) {
            RenderDataManager::SubmitRenderItems(light.GetRenderItems());
        }

        for (BulletCasing& bulletCasing : bulletCasings) {
            bulletCasing.SubmitRenderItem();
        }

        RenderDataManager::ResetBaseSkinnedVertex();
        for (AnimatedGameObject& animatedGameObject : animatedGameObjects) {
            animatedGameObject.UpdateRenderItems();
            animatedGameObject.SubmitForSkinning();
            g_skinnedRenderItems.insert(g_skinnedRenderItems.end(), animatedGameObject.GetRenderItems().begin(), animatedGameObject.GetRenderItems().end());
        }

        for (Wall& wall : GetWalls()) {
            wall.SubmitRenderItems();
        }

        // Hack to render door and window cube transforms
        if (false) {
            int meshIndex = AssetManager::GetMeshIndexByModelNameMeshName("Primitives", "Cube");
            std::vector<Transform>& transforms = GetDoorAndWindowCubeTransforms();
            for (Transform& transform : transforms) {
                Material* material = AssetManager::GetDefaultMaterial();
                RenderItem renderItem;
                renderItem.meshIndex = meshIndex;
                renderItem.modelMatrix = transform.to_mat4();
                renderItem.baseColorTextureIndex = material->m_basecolor;
                renderItem.normalMapTextureIndex = material->m_normal;
                renderItem.rmaTextureIndex = material->m_rma;
                Util::UpdateRenderItemAABB(renderItem);
                RenderDataManager::SubmitRenderItem(renderItem);
            }
        }

        // Animated game objects
        for (int i = 0; i < Game::GetLocalPlayerCount(); i++) {
            Player* player = Game::GetLocalPlayerByIndex(i);
            AnimatedGameObject* viewWeapon = player->GetViewWeaponAnimatedGameObject();
            AnimatedGameObject* characterModel = player->GetCharacterModelAnimatedGameObject();
            viewWeapon->UpdateRenderItems();
            viewWeapon->SubmitForSkinning();
            characterModel->UpdateRenderItems();
            characterModel->SubmitForSkinning();
            g_skinnedRenderItems.insert(g_skinnedRenderItems.end(), viewWeapon->GetRenderItems().begin(), viewWeapon->GetRenderItems().end());
            g_skinnedRenderItems.insert(g_skinnedRenderItems.end(), characterModel->GetRenderItems().begin(), characterModel->GetRenderItems().end());
        }
    }

    //std::vector<RenderItem>& GetRenderItems() { return g_renderItems; }
    std::vector<RenderItem>& GetRenderItemsBlended() { return g_renderItemsBlended; }
    std::vector<RenderItem>& GetRenderItemsAlphaDiscarded() { return g_renderItemsAlphaDiscarded; }
    std::vector<RenderItem>& GetRenderItemsHairTopLayer() { return g_renderItemsHairTopLayer; }
    std::vector<RenderItem>& GetRenderItemsHairBottomLayer() { return g_renderItemsHairBottomLayer; }
    std::vector<RenderItem>& GetSkinnedRenderItems() { return g_skinnedRenderItems; }
}