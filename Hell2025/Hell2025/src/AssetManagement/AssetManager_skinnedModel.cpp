#include "AssetManager.h"
#include "Util/Util.h"
#include "File/AssimpImporter.h"
#include <assimp/Importer.hpp> 
#include <assimp/scene.h> 
#include <assimp/postprocess.h> 
#include <assimp/matrix3x3.h>
#include <assimp/matrix4x4.h>
#include <future>
#include <numeric>
#include "HellTypes.h"

namespace AssetManager {

    void LoadPendingSkinnedModelsAsync() {
        for (SkinnedModel& skinnedModel : GetSkinnedModels()) {
            if (skinnedModel.GetLoadingState() == LoadingState::AWAITING_LOADING_FROM_DISK) {
                skinnedModel.SetLoadingState(LoadingState::LOADING_FROM_DISK);
                AddItemToLoadLog(skinnedModel.GetFileInfo().path);
                std::async(std::launch::async, LoadSkinnedModel, &skinnedModel);
                return;
            }
        }
    }
    
    void GrabSkeleton(std::vector<Node>& nodes, const aiNode* pNode, int parentIndex) {
        // Create the joint node
        Node node;
        node.name = Util::CopyConstChar(pNode->mName.C_Str());
        node.inverseBindTransform = Util::aiMatrix4x4ToGlm(pNode->mTransformation);
        node.parentIndex = parentIndex;

        // Determine the current node's index and push it
        int currentIndex = static_cast<int>(nodes.size());
        nodes.push_back(node);

        // Recursively process children using the current node's index as parentIndex
        for (unsigned int i = 0; i < pNode->mNumChildren; i++) {
            GrabSkeleton(nodes, pNode->mChildren[i], currentIndex);
        }
    }

    void AssetManager::LoadSkinnedModel(SkinnedModel* skinnedModel) {
        const FileInfo& fileInfo = skinnedModel->GetFileInfo();
        std::string assetPath = "res/skinned_models/" + fileInfo.name + ".skinnedmodel";
        SkinnedModelData skinnedModelData = File::ImportSkinnedModel(assetPath);

        skinnedModel->Load(skinnedModelData);
        skinnedModel->SetLoadingState(LoadingState::LOADING_COMPLETE);
    }

    void ExportMissingSkinnedModels() {
        // Scan for new obj and fbx and export custom model format
        for (FileInfo& fileInfo : Util::IterateDirectory("res/skinned_models_raw", { "obj", "fbx" })) {
            std::string assetPath = "res/skinned_models/" + fileInfo.name + ".skinnedmodel";

            // If the file exists but timestamps don't match, re-export
            if (Util::FileExists(assetPath)) {
                uint64_t lastModified = File::GetLastModifiedTime(fileInfo.path);
                SkinnedModelHeader modelHeader = File::ReadSkinnedModelHeader(assetPath);
                //if (modelHeader.timestamp != lastModified || fileInfo.name == "Knife") {
                if (modelHeader.timestamp != lastModified) {
                    File::DeleteFile(assetPath);
                    SkinnedModelData modelData = AssimpImporter::ImportSkinnedFbx(fileInfo.path);
                    File::ExportSkinnedModel(modelData);
                }
            }
            // File doesn't even exist yet, so export it
            else {
                SkinnedModelData modelData = AssimpImporter::ImportSkinnedFbx(fileInfo.path);
                File::ExportSkinnedModel(modelData);
            }
        }
    }

    SkinnedModel* AssetManager::GetSkinnedModelByName(const std::string& name) {
        std::vector<SkinnedModel>& skinnedModels = GetSkinnedModels();
        for (auto& skinnedModel : skinnedModels) {
            if (name == skinnedModel.GetName()) {
                return &skinnedModel;
            }
        }
        std::cout << "AssetManager::GetSkinnedModelByName(const std::string& name) failed because '" << name << "' does not exist!\n";
        return nullptr;
    }

    SkinnedModel* AssetManager::GetSkinnedModelByIndex(int index) {
        std::vector<SkinnedModel>& skinnedModels = GetSkinnedModels();
        if (index >= 0 && index < skinnedModels.size()) {
            return &skinnedModels[index];
        }
        else {
            std::cout << "AssetManager::GetSkinnedModelByIndex(int index) failed because index '" << index << "' is out of range. Size is " << skinnedModels.size() << "!\n";
            return nullptr;
        }
    }

    int AssetManager::GetSkinnedModelIndexByName(const std::string& name) {
        std::vector<SkinnedModel>& skinnedModels = GetSkinnedModels();
        for (int i = 0; i < skinnedModels.size(); i++) {
            if (name == skinnedModels[i].GetName()) {
                return i;
            }
        }
        std::cout << "AssetManager::GetSkinnedModelIndexByName(const std::string& name) failed because '" << name << "' does not exist!\n";
        return -1;
    }
}