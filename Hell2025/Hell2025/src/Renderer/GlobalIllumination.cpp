#include "GlobalIllumination.h"
#include "AssetManagement/AssetManager.h"
#include "Bvh/Gpu/Bvh.h"
#include "Input/Input.h"
#include "Renderer/Renderer.h"
#include "Physics/Physics.h"
#include "World/World.h"

#include "Core/Game.h" 

namespace GlobalIllumination {

    struct Triangle {
        glm::vec3 v0;
        glm::vec3 v1;
        glm::vec3 v2;
        glm::vec3 normal;
    };

    void CreatePointCloud();
    void CreateDoorBvh();
    void CreateHouseBvh();
    //void CreateWallSegmentBvhs();
    void UpdateSceneBVh();

    inline float RoundUp(float value, float spacing)    { return std::ceil(value / spacing) * spacing; }
    inline float RoundDown(float value, float spacing)  { return std::floor(value / spacing) * spacing; }

    uint64_t g_houseBvhId = 0;
    uint64_t g_doorBvhId = 0;
    uint64_t g_sceneBvhId = 0;

   // std::vector<uint64_t> g_wallSegmentBvhIds;

    glm::vec3 g_houseMinBounds = glm::vec3(0.0f);
    glm::vec3 g_houseMaxBounds = glm::vec3(0.0f);

    std::vector<Triangle> g_triangles;
    std::vector<CloudPoint> g_pointCloud;
    bool g_globalIlluminationStructuresDirty = false;
    bool g_pointCloudNeedsGpuUpdate = false;

    void Update() {
        if (g_globalIlluminationStructuresDirty) {
            GlobalIllumination::CreatePointCloud();
            GlobalIllumination::CreateHouseBvh();
            if (g_doorBvhId == 0) {
                CreateDoorBvh();
            }
            if (g_sceneBvhId == 0) {
                g_sceneBvhId = Bvh::Gpu::CreateNewSceneBvh();
            }
            Bvh::Gpu::FlatternMeshBvhNodes();
            g_globalIlluminationStructuresDirty = false;
        }

        UpdateSceneBVh();

        //Bvh::Gpu::RenderMeshBvh(g_houseBvh, YELLOW, glm::mat4(1.0f));

        //for (Door& door : World::GetDoors()) {
        //    glm::mat4 modelMatrix = door.GetDoorModelMatrix();
        //    Bvh::Gpu::RenderMeshBvh(g_doorBvh, YELLOW, modelMatrix);
        //}

        //Bvh::Gpu::RenderSceneBvh(g_sceneBvhId, YELLOW);

        Player* player = Game::GetLocalPlayerByIndex(0);
        if (player) {

            glm::vec3 rayOrigin = player->GetCameraPosition();
            glm::vec3 rayDir = player->GetCameraForward();
            float maxDistance = 10000.0f;

            BvhRayResult result = Bvh::Gpu::ClosestHit(g_sceneBvhId, rayOrigin, rayDir, maxDistance);
            if (result.hitFound) {

               // Bvh::Gpu::RenderRayResultTriangle(result, GREEN);
             //   Bvh::Gpu::RenderRayResultNode(result, YELLOW);

              //  glm::vec3 color = RED;
              //  if (result.objectId == 999) {
              //      color = GREEN;
              //  }

               //AABB aabb(result.nodeBoundsMin, result.nodeBoundsMax);
               //
               //Renderer::DrawPoint(result.hitPosition, color);
               //Renderer::DrawAABB(aabb, YELLOW, result.primitiveTransform);

                //bool hitFound = false;
                //size_t primtiviveId = 0;
                //uint64_t objectId = 0;
                //ObjectType objectType = ObjectType::NONE;
                //float distanceToHit = std::numeric_limits<float>::max();
                //glm::vec3 hitPosition = glm::vec3(0);
                //glm::mat4 primitiveTransform = glm::mat4(1.0f);
                //glm::vec3 nodeBoundsMin = glm::vec3(0.0f);
                //glm::vec3 nodeBoundsMax = glm::vec3(0.0f);

            }
        }

    }

    void CreatePointCloud() {
        g_triangles.clear();

        // Store floor and ceilings triangles
        for (Plane& plane : World::GetPlanes()) {
            for (uint32_t i = 0; i < plane.GetIndices().size(); i+=3) {
                Triangle& triangle = g_triangles.emplace_back();

                int idx0 = plane.GetIndices()[i + 0];
                int idx1 = plane.GetIndices()[i + 1];
                int idx2 = plane.GetIndices()[i + 2];

                triangle.v0 = plane.GetVertices()[idx0].position;
                triangle.v1 = plane.GetVertices()[idx1].position;
                triangle.v2 = plane.GetVertices()[idx2].position;

                triangle.normal = plane.GetVertices()[i].normal;
            }
        }

        int wallSegmentCount = 0;

        // Store wall triangles
        for (Wall& wall : World::GetWalls()) {
            for (WallSegment& wallSegment : wall.GetWallSegments()) {

                wallSegmentCount++;

                for (uint32_t i = 0; i < wallSegment.GetIndices().size(); i += 3) {
                    Triangle& triangle = g_triangles.emplace_back();
                    
                    int idx0 = wallSegment.GetIndices()[i + 0];
                    int idx1 = wallSegment.GetIndices()[i + 1];
                    int idx2 = wallSegment.GetIndices()[i + 2];
                    
                    triangle.v0 = wallSegment.GetVertices()[idx0].position;
                    triangle.v1 = wallSegment.GetVertices()[idx1].position;
                    triangle.v2 = wallSegment.GetVertices()[idx2].position;
                    
                    triangle.normal = wallSegment.GetVertices()[i].normal;
                }
            }
        }

        g_pointCloud.clear();

        for (Triangle& triangle : g_triangles) {

            // Make sure normal is valid
            glm::vec3 edge1 = triangle.v1 - triangle.v0;
            glm::vec3 edge2 = triangle.v2 - triangle.v0;
            triangle.normal = glm::cross(edge1, edge2);
            triangle.normal = glm::normalize(triangle.normal);

            // Calculate the normal of the triangle
            const glm::vec3& normal = triangle.normal;

            // Choose the up vector based on the normal
            glm::vec3 up = (std::abs(normal.z) > 0.999f) ? glm::vec3(1.0f, 0.0f, 0.0f) : glm::vec3(0.0f, 0.0f, 1.0f);

            // Calculate right and up vectors
            glm::vec3 right = glm::normalize(glm::cross(up, normal));
            up = glm::cross(normal, right);  // No need to normalize again as the cross product of two unit vectors is already normalized

            glm::mat3 transform(right.x, right.y, right.z,up.x, up.y, up.z,normal.x, normal.y, normal.z);

            glm::vec2 v0_2d(glm::dot(right, triangle.v0), glm::dot(up, triangle.v0));
            glm::vec2 v1_2d(glm::dot(right, triangle.v1), glm::dot(up, triangle.v1));
            glm::vec2 v2_2d(glm::dot(right, triangle.v2), glm::dot(up, triangle.v2));

            // Determine the bounding box of the 2D triangle
            glm::vec2 min = glm::min(glm::min(v0_2d, v1_2d), v2_2d);
            glm::vec2 max = glm::max(glm::max(v0_2d, v1_2d), v2_2d);

            // Round min and max values
            min.x = RoundDown(min.x, POINT_CLOUD_SPACING) - POINT_CLOUD_SPACING * 0.5f;
            min.y = RoundDown(min.y, POINT_CLOUD_SPACING) - POINT_CLOUD_SPACING * 0.5f;
            max.x = RoundUp(max.x, POINT_CLOUD_SPACING) + POINT_CLOUD_SPACING * 0.5f;
            max.y = RoundUp(max.y, POINT_CLOUD_SPACING) + POINT_CLOUD_SPACING * 0.5f;

            float theshold = 0.05f;
            min.x += theshold;
            min.y += theshold;
            max.x -= theshold;
            max.y -= theshold;

            // Generate points within the bounding box
            for (float x = min.x; x <= max.x; x += POINT_CLOUD_SPACING) {
                for (float y = min.y; y <= max.y; y += POINT_CLOUD_SPACING) {
                    glm::vec2 pt(x, y);
                    if (Util::IsPointInTriangle2D(pt, v0_2d, v1_2d, v2_2d)) {
                        glm::vec3 pt3d = triangle.v0 + right * (pt.x - v0_2d.x) + up * (pt.y - v0_2d.y);

                        CloudPoint& cloudPoint = g_pointCloud.emplace_back();
                        cloudPoint.position = glm::vec4(pt3d, 0.0f);
                        cloudPoint.normal = glm::vec4(triangle.normal, 0.0f);
                    }
                }
            }
        }

        g_pointCloudNeedsGpuUpdate = true;

        std::cout << "Recreated point cloud: " << g_pointCloud.size() << " points \n";
    }

   //void CreateWallSegmentBvhs() {
   //
   //    // Destroy and old wall segment bvhs
   //    for (uint64_t bvhId : g_wallSegmentBvhIds) {
   //        Bvh::Gpu::DestroyMeshBvh(bvhId);
   //    }
   //
   //    for (Wall& wall : World::GetWalls()) {
   //        for (WallSegment& wallSegment : wall.GetWallSegments()) {
   //
   //           // std::cout << "wall segment\n";
   //
   //            const std::vector<Vertex>& vertices = wallSegment.GetVertices();
   //            const std::vector<uint32_t>& indices = wallSegment.GetIndices();
   //
   //         //  for (const Vertex& vertex : vertices) {
   //         //      std::cout << " - " << vertex.position << "     " << vertex.normal << "\n";
   //         //  }
   //
   //            uint64_t bvhId = Bvh::Gpu::CreateMeshBvhFromVertexData(vertices, indices);
   //            g_wallSegmentBvhIds.push_back(bvhId);
   //
   //        }
   //    }
   //
   //}

    void CreateHouseBvh() {     

        //CreateWallSegmentBvhs();
        //return;


        // Destroy any previous house bvh
        if (g_houseBvhId != 0) {
            Bvh::Gpu::DestroyMeshBvh(g_houseBvhId);
        }


        g_houseMinBounds = glm::vec3(std::numeric_limits<float>::max());
        g_houseMaxBounds = glm::vec3(-std::numeric_limits<float>::max());

        // Create house vertices
        std::vector<Vertex> vertices;
        for (Triangle& triangle : g_triangles) {
            Vertex v0, v1, v2;
            v0.position = triangle.v0;
            v1.position = triangle.v1;
            v2.position = triangle.v2;
            v0.normal = triangle.normal;
            v1.normal = triangle.normal;
            v2.normal = triangle.normal;
            vertices.push_back(triangle.v0);
            vertices.push_back(triangle.v1);
            vertices.push_back(triangle.v2);

            g_houseMinBounds.x = std::min(g_houseMinBounds.x, v0.position.x);
            g_houseMinBounds.y = std::min(g_houseMinBounds.y, v0.position.y);
            g_houseMinBounds.z = std::min(g_houseMinBounds.z, v0.position.z);
            g_houseMaxBounds.x = std::max(g_houseMaxBounds.x, v0.position.x);
            g_houseMaxBounds.y = std::max(g_houseMaxBounds.y, v0.position.y);
            g_houseMaxBounds.z = std::max(g_houseMaxBounds.z, v0.position.z);

            g_houseMinBounds.x = std::min(g_houseMinBounds.x, v1.position.x);
            g_houseMinBounds.y = std::min(g_houseMinBounds.y, v1.position.y);
            g_houseMinBounds.z = std::min(g_houseMinBounds.z, v1.position.z);
            g_houseMaxBounds.x = std::max(g_houseMaxBounds.x, v1.position.x);
            g_houseMaxBounds.y = std::max(g_houseMaxBounds.y, v1.position.y);
            g_houseMaxBounds.z = std::max(g_houseMaxBounds.z, v1.position.z);

            g_houseMinBounds.x = std::min(g_houseMinBounds.x, v2.position.x);
            g_houseMinBounds.y = std::min(g_houseMinBounds.y, v2.position.y);
            g_houseMinBounds.z = std::min(g_houseMinBounds.z, v2.position.z);
            g_houseMaxBounds.x = std::max(g_houseMaxBounds.x, v2.position.x);
            g_houseMaxBounds.y = std::max(g_houseMaxBounds.y, v2.position.y);
            g_houseMaxBounds.z = std::max(g_houseMaxBounds.z, v2.position.z);
        }

        // Create house indices
        std::vector<uint32_t> indices(vertices.size());
        for (int i = 0; i < vertices.size(); i++) {
            indices[i] = i;
        }

        g_houseBvhId = Bvh::Gpu::CreateMeshBvhFromVertexData(vertices, indices);
    }

    void CreateDoorBvh() {
        Mesh* cubeMesh = AssetManager::GetCubeMesh();
        std::vector<Vertex> vertices = AssetManager::GetMeshVertices(cubeMesh);

        for (Vertex& vertex : vertices) {
            vertex.position *= glm::vec3(DOOR_DEPTH, DOOR_HEIGHT, DOOR_WIDTH);
            vertex.position.x += -0.005f;
            vertex.position.z -= DOOR_WIDTH / 2;
            vertex.position.y += DOOR_HEIGHT / 2;
            vertex.position.x -= DOOR_DEPTH / 2;
        }

        std::vector<uint32_t> indices(vertices.size());
        for (int i = 0; i < vertices.size(); i++) {
            indices[i] = i;
        }

        g_doorBvhId = Bvh::Gpu::CreateMeshBvhFromVertexData(vertices, indices);
    }
    
    void UpdateSceneBVh() {

        std::vector<Door>& doors = World::GetDoors();

        std::vector<PrimitiveInstance> instances;

        //int instanceCount = 1 + doors.size(); 

        bool addHouse = true;
        bool addDoors = true;

        //instances.reserve(instanceCount);


        // Add wall segment bvhs as primitive instances
       // for (int i = 0; i < g_wallSegmentBvhIds.size(); i++) {
      // std::cout << "\n";
      // std::cout << "Wall segment Bvh AABBs\n";
      //  int i = 0;
          // for (uint64_t bvhId : g_wallSegmentBvhIds) {
          //
          //     MeshBvh* meshBvh = Bvh::Gpu::GetMeshBvhById(bvhId);
          //     glm::vec3 aabbMin = meshBvh->m_nodes[0].boundsMin;
          //     glm::vec3 aabbMax = meshBvh->m_nodes[0].boundsMax;
          //
          //     PrimitiveInstance& instance = instances.emplace_back();
          //     instance.worldAabbBoundsMin.x = aabbMin.x;
          //     instance.worldAabbBoundsMin.y = aabbMin.y;
          //     instance.worldAabbBoundsMin.z = aabbMin.z;
          //     instance.worldAabbBoundsMax.x = aabbMax.x;
          //     instance.worldAabbBoundsMax.y = aabbMax.y;
          //     instance.worldAabbBoundsMax.z = aabbMax.z;
          //     instance.objectId = 0;
          //     instance.worldTransform = glm::mat4(1.0f);
          //     instance.meshBvhId = bvhId;
          //     instance.worldAabbCenter = (instance.worldAabbBoundsMin + instance.worldAabbBoundsMax) * 0.5f;
          //
          // //    std::cout << "i: " << i << " " << aabbMin << "  " << aabbMax << "\n";
          //  //   i++;
          // }
      //  }




        // Add the house
        if (addHouse) {
            PrimitiveInstance& instance = instances.emplace_back();
            instance.worldAabbBoundsMin.x = g_houseMinBounds.x;
            instance.worldAabbBoundsMin.y = g_houseMinBounds.y;
            instance.worldAabbBoundsMin.z = g_houseMinBounds.z;
            instance.worldAabbBoundsMax.x = g_houseMaxBounds.x;
            instance.worldAabbBoundsMax.y = g_houseMaxBounds.y;
            instance.worldAabbBoundsMax.z = g_houseMaxBounds.z;
            instance.objectId = 0;
            instance.worldTransform = glm::mat4(1.0f);
            instance.meshBvhId = g_houseBvhId;
            instance.worldAabbCenter = (instance.worldAabbBoundsMin + instance.worldAabbBoundsMax) * 0.5f;
        }

        //AABB aabb(g_houseMinBounds, g_houseMaxBounds);
        //Renderer::DrawAABB(aabb, BLUE);

       // std::cout << g_houseMinBounds << " " << g_houseMaxBounds << "\n";

        if (addDoors) {
            int objectId = 1;
        // Add all the doors
            for (Door& door : doors) {
                uint64_t rigidStaticId = door.GetPhysicsId();
                RigidStatic* rigidStatic = Physics::GetRigidStaitcById(rigidStaticId);
                PxRigidStatic* pxRigidStatic = rigidStatic->GetPxRigidStatic();
                PxRigidActor* pxRigidActor = static_cast<PxRigidActor*>(pxRigidStatic);
                PxBounds3 bounds = pxRigidActor->getWorldBounds();
                PxVec3 minBounds = bounds.minimum;
                PxVec3 maxBounds = bounds.maximum;

                PrimitiveInstance& instance = instances.emplace_back();
                instance.worldAabbBoundsMin.x = minBounds.x;
                instance.worldAabbBoundsMin.y = minBounds.y;
                instance.worldAabbBoundsMin.z = minBounds.z;
                instance.worldAabbBoundsMax.x = maxBounds.x;
                instance.worldAabbBoundsMax.y = maxBounds.y;
                instance.worldAabbBoundsMax.z = maxBounds.z;
                instance.objectId = objectId;// door.GetObjectId();
                instance.worldTransform = door.GetDoorModelMatrix();
                instance.meshBvhId = g_doorBvhId;
                instance.worldAabbCenter = (instance.worldAabbBoundsMin + instance.worldAabbBoundsMax) * 0.5f;
                objectId++;
            }
        }

        Bvh::Gpu::UpdateSceneBvh(g_sceneBvhId, instances);
    }

    uint64_t GetSceneBvhId() {
        return g_sceneBvhId;
    }

    const std::vector<BvhNode>& GetSceneNodes() {
        static std::vector<BvhNode> empty;
        if (g_sceneBvhId == 0) {
            return empty;
        }

        SceneBvh* sceneBvh = Bvh::Gpu::GetSceneBvhById(g_sceneBvhId);
        if (!sceneBvh) return empty;

        return sceneBvh->m_nodes;
    }

    std::vector<CloudPoint>& GetPointClound() {
        return g_pointCloud;
    }

    void SetGlobalIlluminationStructuresDirtyState(bool state) {
        g_globalIlluminationStructuresDirty = state;
    }

    bool GlobalIlluminationStructuresAreDirty() {
        return g_globalIlluminationStructuresDirty;
    }

    void SetPointCloudNeedsGpuUpdateState(bool state) {
        g_pointCloudNeedsGpuUpdate = state;
    }

    bool PointCloudNeedsGpuUpdate() {
        return g_pointCloudNeedsGpuUpdate;
    }
}