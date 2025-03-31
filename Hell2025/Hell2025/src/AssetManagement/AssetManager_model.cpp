#include "AssetManager.h"
#include "File/AssimpImporter.h"
#include "Util/Util.h"
#include <future>

namespace AssetManager {

    void PrintModelMeshNames(Model* model) {
        std::cout << model->GetName() << "\n";
        for (const uint32_t& meshIndex : model->GetMeshIndices()) {
            Mesh* mesh = AssetManager::GetMeshByIndex(meshIndex);
            if (mesh) {
                std::cout << " - " << mesh->GetName() << "\n";
            }
            else {
                std::cout << " - INVALID MESH SOMEHOW\n";
            }
        }
    }

    void LoadPendingModelsAsync() {
        for (Model& model : GetModels()) {
            if (model.GetLoadingState() == LoadingState::AWAITING_LOADING_FROM_DISK) {
                model.SetLoadingState(LoadingState::LOADING_FROM_DISK);
                AddItemToLoadLog(model.GetFileInfo().path);
                std::async(std::launch::async, LoadModel, &model);
                return;
            }
        }
    }

    void BakePendingModels() {
        for (Model& model : GetModels()) {
            if (model.GetLoadingState() == LoadingState::LOADING_COMPLETE) {
                BakeModel(model);
                AddItemToLoadLog("Baking: " + model.GetName());
            }
        }
    }

    void LoadModel(Model* model) {
        const FileInfo& fileInfo = model->GetFileInfo();
        std::string filepath = "res/models/" + fileInfo.name + "." + fileInfo.ext;
        model->m_modelData = File::ImportModel(filepath);
        model->SetLoadingState(LoadingState::LOADING_COMPLETE);
    }

    void BakeModel(Model& model) {
        model.SetName(model.m_modelData.name);
        model.SetAABB(model.m_modelData.aabbMin, model.m_modelData.aabbMax);
        for (MeshData& meshData : model.m_modelData.meshes) {
            int meshIndex = CreateMesh(meshData.name, meshData.vertices, meshData.indices, meshData.aabbMin, meshData.aabbMax);
            model.AddMeshIndex(meshIndex);
        }
    }

    void ExportMissingModels() {
        //return;
        // Scan for new obj and fbx and export custom model format
        for (FileInfo& fileInfo : Util::IterateDirectory("res/models_raw", { "obj", "fbx" })) {
            std::string assetPath = "res/models/" + fileInfo.name + ".model";

            // If the file exists but timestamps don't match, re-export
            if (Util::FileExists(assetPath)) {
                uint64_t lastModified = File::GetLastModifiedTime(fileInfo.path);
                ModelHeader modelHeader = File::ReadModelHeader(assetPath);
                if (modelHeader.timestamp != lastModified) {
                    File::DeleteFile(assetPath);
                    ModelData modelData = AssimpImporter::ImportFbx(fileInfo.path);
                    File::ExportModel(modelData);
                }
            }
            // File doesn't even exist yet, so export it
            else {
                ModelData modelData = AssimpImporter::ImportFbx(fileInfo.path);
                File::ExportModel(modelData);
            }
        }
    }

    Model* CreateModel(const std::string& name) {
        std::vector<Model>& models = GetModels();
        models.emplace_back();
        Model* model = &models[models.size() - 1];
        model->SetName(name);
        return model;
    }

    Model* GetModelByName(const std::string& name) {
        int index = GetModelIndexByName(name);
        return GetModelByIndex(index);
    }

    Model* GetModelByIndex(int index) {
        if (index >= 0 && index < GetModels().size()) {
            return &GetModels()[index];
        }
        return nullptr;
    }

    int GetModelIndexByName(const std::string& name) {
        std::unordered_map<std::string, int>& modelIndexMap = GetModelIndexMap();
        auto it = modelIndexMap.find(name);
        if (it != modelIndexMap.end()) {
            return it->second;
        }
        std::cout << "AssetManager::GetModelIndexByName() failed because name '" << name << "' was not found in g_models!\n";
        return -1;
    }

    void BuildPrimitives() {
        Model* model = CreateModel("Primitives");

        /* Quad */ {
            std::vector<Vertex> vertices = {
                // Position               Normal               UV            Tangent
                {{-0.5f, -0.5f, 0.0f},    {0.0f, 0.0f, 1.0f},  {0.0f, 0.0f}, {1.0f, 0.0f, 0.0f}}, // Bottom-left
                {{ 0.5f, -0.5f, 0.0f},    {0.0f, 0.0f, 1.0f},  {1.0f, 0.0f}, {1.0f, 0.0f, 0.0f}}, // Bottom-right
                {{ 0.5f,  0.5f, 0.0f},    {0.0f, 0.0f, 1.0f},  {1.0f, 1.0f}, {1.0f, 0.0f, 0.0f}}, // Top-right
                {{-0.5f,  0.5f, 0.0f},    {0.0f, 0.0f, 1.0f},  {0.0f, 1.0f}, {1.0f, 0.0f, 0.0f}}  // Top-left
            };

            std::vector<uint32_t> indices = { 0, 1, 2, 2, 3, 0 };

            int meshIndex = CreateMesh("Quad", vertices, indices);
            model->AddMeshIndex(meshIndex);
        }

        /* Cube */ {
            std::vector<Vertex> cubeVertices = {
                // FRONT FACE
                {{-0.5f, -0.5f,  0.5f}, { 0, 0, 1}, {0, 0}, {1, 0, 0}},
                {{ 0.5f, -0.5f,  0.5f}, { 0, 0, 1}, {1, 0}, {1, 0, 0}},
                {{ 0.5f,  0.5f,  0.5f}, { 0, 0, 1}, {1, 1}, {1, 0, 0}},
                {{-0.5f,  0.5f,  0.5f}, { 0, 0, 1}, {0, 1}, {1, 0, 0}},

                // BACK FACE
                {{ 0.5f, -0.5f, -0.5f}, { 0, 0, -1}, {0, 0}, {-1, 0, 0}},
                {{-0.5f, -0.5f, -0.5f}, { 0, 0, -1}, {1, 0}, {-1, 0, 0}},
                {{-0.5f,  0.5f, -0.5f}, { 0, 0, -1}, {1, 1}, {-1, 0, 0}},
                {{ 0.5f,  0.5f, -0.5f}, { 0, 0, -1}, {0, 1}, {-1, 0, 0}},

                // LEFT FACE
                {{-0.5f, -0.5f, -0.5f}, {-1, 0, 0}, {0, 0}, {0, 0, 1}},
                {{-0.5f, -0.5f,  0.5f}, {-1, 0, 0}, {1, 0}, {0, 0, 1}},
                {{-0.5f,  0.5f,  0.5f}, {-1, 0, 0}, {1, 1}, {0, 0, 1}},
                {{-0.5f,  0.5f, -0.5f}, {-1, 0, 0}, {0, 1}, {0, 0, 1}},

                // RIGHT FACE
                {{ 0.5f, -0.5f,  0.5f}, { 1, 0, 0}, {0, 0}, {0, 0, -1}},
                {{ 0.5f, -0.5f, -0.5f}, { 1, 0, 0}, {1, 0}, {0, 0, -1}},
                {{ 0.5f,  0.5f, -0.5f}, { 1, 0, 0}, {1, 1}, {0, 0, -1}},
                {{ 0.5f,  0.5f,  0.5f}, { 1, 0, 0}, {0, 1}, {0, 0, -1}},

                // TOP FACE
                {{-0.5f,  0.5f,  0.5f}, { 0, 1, 0}, {0, 0}, {1, 0, 0}},
                {{ 0.5f,  0.5f,  0.5f}, { 0, 1, 0}, {1, 0}, {1, 0, 0}},
                {{ 0.5f,  0.5f, -0.5f}, { 0, 1, 0}, {1, 1}, {1, 0, 0}},
                {{-0.5f,  0.5f, -0.5f}, { 0, 1, 0}, {0, 1}, {1, 0, 0}},

                // BOTTOM FACE
                {{-0.5f, -0.5f, -0.5f}, { 0,-1, 0}, {0, 0}, {1, 0, 0}},
                {{ 0.5f, -0.5f, -0.5f}, { 0,-1, 0}, {1, 0}, {1, 0, 0}},
                {{ 0.5f, -0.5f,  0.5f}, { 0,-1, 0}, {1, 1}, {1, 0, 0}},
                {{-0.5f, -0.5f,  0.5f}, { 0,-1, 0}, {0, 1}, {1, 0, 0}} 
            };

            std::vector<uint32_t> cubeIndices = {
                0, 1, 2,  2, 3, 0,      // Front face
                4, 5, 6,  6, 7, 4,      // Back face
                8, 9, 10, 10, 11, 8,    // Left face
                12, 13, 14, 14, 15, 12, // Right face
                16, 17, 18, 18, 19, 16, // Top face
                20, 21, 22, 22, 23, 20  // Bottom face
            };

            int meshIndexCube = CreateMesh("Cube", cubeVertices, cubeIndices);
            model->AddMeshIndex(meshIndexCube);
        }
        model->SetLoadingState(LoadingState::LOADING_COMPLETE);
    }
}