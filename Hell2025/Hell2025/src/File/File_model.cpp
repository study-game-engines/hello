#include "File.h"
#include "Util.h"

#include <fstream>
#include <string>
#include <cstdint>
#include <chrono>
#include <cstring> // For std::memset, std::memcpy
#include <string>
#include <cstddef> // For size_t

#define PRINT_MODEL_HEADERS_ON_READ 0
#define PRINT_MODEL_HEADERS_ON_WRITE 0
#define PRINT_MESH_HEADERS_ON_READ 0
#define PRINT_MESH_HEADERS_ON_WRITE 0

namespace File {

    ModelHeaderV2 ReadModelHeaderV2(const std::string& filepath) {
        ModelHeaderV2 header = {};
        std::ifstream file(filepath, std::ios::binary);

        // Check for invalid file AFTER trying to open
        if (!file.is_open()) {
            std::cerr << "ReadModelHeader() failed to open file " << filepath << "\n";
            header = {};
            return header;
        }

        // Read file data
        file.read(reinterpret_cast<char*>(&header), sizeof(ModelHeaderV2));

        // Error check stream state after read attempt
        if (!file) {
            std::cerr << "ReadModelHeader() failed to read file " << filepath << "\n";
            header = {};
            return header;
        }

        return header;
    }

    void ExportModelV2(const ModelData& modelData) {
        std::string outputPath = "res/models/" + modelData.name + ".model";
        std::ofstream file(outputPath, std::ios::binary);
        if (!file.is_open()) {
            std::cout << "Failed to open file for writing: " << outputPath << "\n";
            return;
        }

        // Fill the header
        ModelHeaderV2 modelHeader;
        modelHeader.version = 2;
        modelHeader.meshCount = modelData.meshCount;
        modelHeader.timestamp = modelData.timestamp;
        modelHeader.aabbMin = modelData.aabbMin;
        modelHeader.aabbMax = modelData.aabbMax;

        MemCopyFileSignature(modelHeader.signature, HELL_MODEL_SIGNATURE);

        // Write the header
        file.write(reinterpret_cast<const char*>(&modelHeader), sizeof(ModelHeaderV2));

        //#if PRINT_MODEL_HEADERS_ON_WRITE
        PrintModelHeader(modelHeader, "Wrote model header: " + outputPath);
        //#endif

        // Write the mesh data
        for (const MeshData& meshData : modelData.meshes) {
            MeshHeaderV2 meshHeader;
            meshHeader.vertexCount = (uint32_t)meshData.vertices.size();
            meshHeader.indexCount = (uint32_t)meshData.indices.size();
            meshHeader.aabbMin = meshData.aabbMin;
            meshHeader.aabbMax = meshData.aabbMax;
            meshHeader.parentIndex = -1;
            meshHeader.localTransform = glm::mat4(1.0f);
            meshHeader.inverseBindTransform = glm::mat4(1.0f);
            MemCopyFileSignature(meshHeader.signature, HELL_MESH_SIGNATURE);
            MemCopyName(meshHeader.name, meshData.name);
            std::cout << meshData.name << " == " << meshHeader.name << "\n";

            // Write the mesh header
            file.write(reinterpret_cast<const char*>(&meshHeader), sizeof(MeshHeaderV2));

            // Write the data
            file.write(reinterpret_cast<const char*>(meshData.vertices.data()), meshData.vertices.size() * sizeof(Vertex));
            file.write(reinterpret_cast<const char*>(meshData.indices.data()), meshData.indices.size() * sizeof(uint32_t));

            //#if PRINT_MESH_HEADERS_ON_WRITE
            PrintMeshHeader(meshHeader, "Wrote mesh: " + meshData.name);
            //#endif
        }
        file.close();
        std::cout << "Exported: " << outputPath << "\n";
    }

    void ExportModel(const ModelData& modelData) {
        std::string outputPath = "res/models/" + modelData.name + ".model";
        std::ofstream file(outputPath, std::ios::binary);
        if (!file.is_open()) {
            std::cout << "Failed to open file for writing: " << outputPath << "\n";
            return;
        }

        // Fill the header
        ModelHeader modelHeader;
        modelHeader.version = 1;
        modelHeader.meshCount = modelData.meshCount;
        modelHeader.nameLength = modelData.name.size();
        modelHeader.timestamp = modelData.timestamp;
        modelHeader.aabbMin = modelData.aabbMin;
        modelHeader.aabbMax = modelData.aabbMax;
        std::memcpy(modelHeader.signature, "HELL_MODEL", 10);

        // Write the header
        file.write(modelHeader.signature, 10);
        file.write((char*)&modelHeader.version, sizeof(modelHeader.version));
        file.write((char*)&modelHeader.meshCount, sizeof(modelHeader.meshCount));
        file.write((char*)&modelHeader.nameLength, sizeof(modelHeader.nameLength));
        file.write(reinterpret_cast<char*>(&modelHeader.timestamp), sizeof(modelHeader.timestamp));
        file.write(reinterpret_cast<const char*>(&modelHeader.aabbMin), sizeof(glm::vec3));
        file.write(reinterpret_cast<const char*>(&modelHeader.aabbMax), sizeof(glm::vec3));
        #if PRINT_MODEL_HEADERS_ON_WRITE
        PrintModelHeader(modelHeader, "Wrote model header: " + outputPath);
        #endif

        // Write the name
        file.write(modelData.name.data(), modelHeader.nameLength);

        // Write the mesh data
        for (const MeshData& meshData : modelData.meshes) {
            MeshHeader meshHeader;
            meshHeader.nameLength = (uint32_t)meshData.name.size();
            meshHeader.vertexCount = (uint32_t)meshData.vertices.size();
            meshHeader.indexCount = (uint32_t)meshData.indices.size();
            meshHeader.aabbMin = meshData.aabbMin;
            meshHeader.aabbMax = meshData.aabbMax;
            file.write((char*)&meshHeader.nameLength, sizeof(meshHeader.nameLength));
            file.write((char*)&meshHeader.vertexCount, sizeof(meshHeader.vertexCount));
            file.write((char*)&meshHeader.indexCount, sizeof(meshHeader.indexCount));
            file.write(meshData.name.data(), meshHeader.nameLength);
            file.write(reinterpret_cast<const char*>(&meshHeader.aabbMin), sizeof(glm::vec3));
            file.write(reinterpret_cast<const char*>(&meshHeader.aabbMax), sizeof(glm::vec3));
            file.write(reinterpret_cast<const char*>(meshData.vertices.data()), meshData.vertices.size() * sizeof(Vertex));
            file.write(reinterpret_cast<const char*>(meshData.indices.data()), meshData.indices.size() * sizeof(uint32_t));
            #if PRINT_MESH_HEADERS_ON_WRITE
            PrintMeshHeader(meshHeader, "Wrote mesh: " + meshData.name);
            #endif
        }
        file.close();
        std::cout << "Exported: " << outputPath << "\n";
    }

    ModelHeader ReadModelHeader(const std::string& filepath) {
        ModelHeader header{};
        std::ifstream file(filepath, std::ios::binary);

        // Check for invalid file
        if (!file.is_open()) {
            return header;
        }

        // Check for valid signature
        char fileSignature[10];
        file.read(fileSignature, 10);
        if (std::strncmp(fileSignature, "HELL_MODEL", 10) != 0) {
            return header;
        }

        // Read the rest of header
        file.read(reinterpret_cast<char*>(&header.version), sizeof(header.version));
        file.read(reinterpret_cast<char*>(&header.meshCount), sizeof(header.meshCount));
        file.read(reinterpret_cast<char*>(&header.nameLength), sizeof(header.nameLength));
        file.read(reinterpret_cast<char*>(&header.timestamp), sizeof(header.timestamp));
        file.read(reinterpret_cast<char*>(&header.aabbMin), sizeof(header.aabbMin));
        file.read(reinterpret_cast<char*>(&header.aabbMax), sizeof(header.aabbMax));
        return header;
    }

    ModelData ImportModel(const std::string& filepath) {
        ModelData modelData;

        std::ifstream file(filepath, std::ios::binary);
        if (!file.is_open()) {
            std::cerr << "Failed to open file for reading: " << filepath << "\n";
            return modelData;
        }

        FileInfo fileInfo = Util::GetFileInfoFromPath(filepath);
        ModelHeader modelHeader;

        // Read 10 bytes for HELL_MODEL
        char fileSignature[10];
        file.read(fileSignature, 10);

        // Read the rest of the header
        file.read((char*)&modelHeader.version, sizeof(modelHeader.version));
        file.read((char*)&modelHeader.meshCount, sizeof(modelHeader.meshCount));
        file.read((char*)&modelHeader.nameLength, sizeof(modelHeader.nameLength));
        file.read(reinterpret_cast<char*>(&modelHeader.timestamp), sizeof(modelHeader.timestamp));
        file.read(reinterpret_cast<char*>(&modelHeader.aabbMin), sizeof(glm::vec3));
        file.read(reinterpret_cast<char*>(&modelHeader.aabbMax), sizeof(glm::vec3));
        #if PRINT_MODEL_HEADERS_ON_READ
        PrintModelHeader(modelHeader, "Read model header: '" + filepath + "'");
        #endif

        // Read the name
        std::string modelName(modelHeader.nameLength, '\0');
        file.read(&modelName[0], modelHeader.nameLength);


        // Store header data
        modelData.meshCount = modelHeader.meshCount;
        modelData.meshes.resize(modelHeader.meshCount);
        //modelData.name = modelName;
        modelData.name = fileInfo.name;
        modelData.aabbMin = modelHeader.aabbMin;
        modelData.aabbMax = modelHeader.aabbMax;
        modelData.timestamp = modelHeader.timestamp;

        // Load each mesh
        for (uint32_t i = 0; i < modelHeader.meshCount; ++i) {
            MeshHeader meshHeader;
            file.read((char*)&meshHeader.nameLength, sizeof(meshHeader.nameLength));
            file.read((char*)&meshHeader.vertexCount, sizeof(meshHeader.vertexCount));
            file.read((char*)&meshHeader.indexCount, sizeof(meshHeader.indexCount));
            std::string meshName(meshHeader.nameLength, '\0');
            file.read(&meshName[0], meshHeader.nameLength);
            file.read(reinterpret_cast<char*>(&meshHeader.aabbMin), sizeof(glm::vec3));
            file.read(reinterpret_cast<char*>(&meshHeader.aabbMax), sizeof(glm::vec3));
            MeshData& meshData = modelData.meshes[i];
            meshData.name = meshName;
            meshData.vertexCount = meshHeader.vertexCount;
            meshData.indexCount = meshHeader.indexCount;
            meshData.vertices.resize(meshData.vertexCount);
            meshData.indices.resize(meshData.indexCount);
            meshData.aabbMin = meshHeader.aabbMin;
            meshData.aabbMax = meshHeader.aabbMax;
            file.read(reinterpret_cast<char*>(meshData.vertices.data()), meshHeader.vertexCount * sizeof(Vertex));
            file.read(reinterpret_cast<char*>(meshData.indices.data()), meshHeader.indexCount * sizeof(uint32_t));

            #if PRINT_MESH_HEADERS_ON_READ
            PrintMeshHeader(meshHeader, "Read mesh: " + meshData.name);
            #endif
        }
        file.close();

        ExportModelV2(modelData);

        return modelData;
    }

    ModelData ImportModelv2(const std::string& filepath) {
        ModelData modelData;

        // Bail if file does not exist
        if (!Util::FileExists(filepath)) {
            std::cout << "File::ImportMovel() failed: " << filepath << " does not exist\n";
            return modelData;
        }

        // Attempt to load the file
        std::ifstream file(filepath, std::ios::binary);
        if (!file.is_open()) {
            std::cout << "File::ImportMovel() failed: could not open: " << filepath << "\n";
            return modelData;
        }

        // Read the header
        ModelHeaderV2 modelHeader = ReadModelHeaderV2(filepath);

        // Bail if signature invalid
        if (!CompareFileSignature(modelHeader.signature, HELL_MODEL_SIGNATURE)) {
            std::cout << "File::ImportMovel() failed: invalid Modeleader signature '" << modelHeader.signature << "'\n";
            return modelData;
        }

        #if PRINT_MODEL_HEADERS_ON_READ
        PrintModelHeader(modelHeader, "Read model header in: " + filepath);
        #endif

        // Seek to the end of the header
        file.seekg(sizeof(ModelHeaderV2), std::ios::beg);

        FileInfo fileInfo = Util::GetFileInfoFromPath(filepath);

        // Store header data
        modelData.meshCount = modelHeader.meshCount;
        modelData.meshes.resize(modelHeader.meshCount);
        modelData.name = fileInfo.name;
        modelData.aabbMin = modelHeader.aabbMin;
        modelData.aabbMax = modelHeader.aabbMax;
        modelData.timestamp = modelHeader.timestamp;

        // Resize meshes vector ready for import
        modelData.meshes.resize(modelData.meshCount);

        // Load each mesh
        for (uint32_t i = 0; i < modelHeader.meshCount; ++i) {
            MeshData& meshData = modelData.meshes[i];

            // Read the header
            MeshHeaderV2 meshHeader = {};
            file.read(reinterpret_cast<char*>(&meshHeader), sizeof(MeshHeaderV2));

            #if PRINT_MESH_HEADERS_ON_READ
            //if (modelData.name == "Piano") {
                PrintMeshHeader(meshHeader, "Read mesh: " + meshData.name);
            //}
            #endif

            meshData.name.assign(meshHeader.name, strnlen(meshHeader.name, sizeof(meshHeader.name)));
            //meshData.name = meshHeader.name;
            meshData.vertexCount = meshHeader.vertexCount;
            meshData.indexCount = meshHeader.indexCount;
            meshData.vertices.resize(meshData.vertexCount);
            meshData.indices.resize(meshData.indexCount);
            meshData.aabbMin = meshHeader.aabbMin;
            meshData.aabbMax = meshHeader.aabbMax;
            meshData.parentIndex = meshHeader.parentIndex;
            meshData.localTransform = meshHeader.localTransform;
            meshData.inverseBindTransform = meshHeader.inverseBindTransform;
            file.read(reinterpret_cast<char*>(meshData.vertices.data()), meshHeader.vertexCount * sizeof(Vertex));
            file.read(reinterpret_cast<char*>(meshData.indices.data()), meshHeader.indexCount * sizeof(uint32_t));

        }
        file.close();

        return modelData;
    }
}