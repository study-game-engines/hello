#include "File.h"
#include <fstream>
#include "Math/Bvh.h"

#define PRINT_MODEL_BVH_HEADERS_ON_READ 1
#define PRINT_MODEL_BVH_HEADERS_ON_WRITE 1
#define PRINT_MESH_BVH_HEADERS_ON_READ 1
#define PRINT_MESH_BVH_HEADERS_ON_WRITE 1

namespace File {

    void ExportMeshBvh(const ModelData& modelData) {

        std::string outputPath = "res/models/" + modelData.name + ".model";
        std::ofstream file(outputPath, std::ios::binary);
        if (!file.is_open()) {
            std::cout << "Failed to open file for writing: " << outputPath << "\n";
            return;
        }

        // Fill the header
        ModelBvhHeader modelHeader;
        modelHeader.version = 1;
        modelHeader.meshCount = modelData.meshCount;
        modelHeader.timestamp = modelData.timestamp;
        std::memcpy(modelHeader.signature, "HELL_MODEL_BVH", 14);

        // Write the header
        file.write(modelHeader.signature, 14);
        file.write((char*)&modelHeader.version, sizeof(modelHeader.version));
        file.write((char*)&modelHeader.meshCount, sizeof(modelHeader.meshCount)); 
        file.write(reinterpret_cast<const char*>(&modelHeader.timestamp), sizeof(modelHeader.timestamp));

        #if PRINT_MODEL_BVH_HEADERS_ON_WRITE
        PrintModelBvhHeader(modelHeader, "Wrote model bvh header: " + outputPath);
        #endif

        // Write the mesh data
        for (const MeshData& meshData : modelData.meshes) {

            // Get flat vertices
            std::vector<Vertex> vertices(meshData.indices.size());
            for (int i = 0; i < meshData.indices.size(); i++) {
                vertices[i] = meshData.vertices[meshData.indices[i]];
            }

            uint64_t meshbvhId = BVH::CreateMeshBvhFromVertices(vertices);
            MeshBvh* meshBvh = BVH::GetMeshBvhById(meshbvhId);

            MeshBvhHeader meshHeader;
            meshHeader.nodeCount = meshBvh->m_nodes.size();
            meshHeader.floatCount = meshBvh->m_triangleData.size();
            file.write((char*)&meshHeader.nodeCount, sizeof(meshHeader.nodeCount));
            file.write((char*)&meshHeader.floatCount, sizeof(meshHeader.floatCount));
            file.write(reinterpret_cast<const char*>(meshBvh->m_nodes.data()), meshBvh->m_nodes.size() * sizeof(BvhNode));
            file.write(reinterpret_cast<const char*>(meshBvh->m_triangleData.data()), meshBvh->m_triangleData.size() * sizeof(float));
            #if PRINT_MESH_BVH_HEADERS_ON_WRITE
            PrintMeshBvhHeader(meshHeader, "Wrote mesh bvh: " + meshData.name);
            #endif

            BVH::DestroyMeshBvh(meshbvhId);
        }
        file.close();
        std::cout << "Exported: " << outputPath << "\n";
    }

    MeshBvh LoadMeshBvh(const std::string& filePath) {

        MeshBvh meshBvh;

        return meshBvh;
    }
}