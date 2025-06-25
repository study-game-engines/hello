#include "GlobalIllumination.h"
#include "Input/Input.h"
#include "Renderer/Renderer.h"
#include "World/World.h"

namespace GlobalIllumination {

    struct Triangle {
        glm::vec3 v0;
        glm::vec3 v1;
        glm::vec3 v2;
        glm::vec3 normal;
    };

    inline float RoundUp(float value, float spacing)    { return std::ceil(value / spacing) * spacing; }
    inline float RoundDown(float value, float spacing)  { return std::floor(value / spacing) * spacing; }

    std::vector<CloudPoint> g_pointCloud;

    void CreatePointCloud() {

        std::vector<Triangle> triangles;

        // Store floor and ceilings triangles
        for (Plane& plane : World::GetPlanes()) {
            for (uint32_t i = 0; i < plane.GetIndices().size(); i+=3) {
                Triangle& triangle = triangles.emplace_back();

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
                    Triangle& triangle = triangles.emplace_back();
                    
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


      //std::cout << "Wall segment count: " << wallSegmentCount << "\n";
      //std::cout << "Wall count: " << World::GetWalls().size() << "\n";
      //std::cout << "Point cloud triangle count: " << triangles.size() << "\n";

        for (Triangle& triangle : triangles) {

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
    }
        

    void Update() {

        if (Input::KeyPressed(HELL_KEY_9)) {
            GlobalIllumination::CreatePointCloud();
        }


        for (CloudPoint& cloudPound : g_pointCloud) {

            Renderer::DrawPoint(cloudPound.position, cloudPound.normal);
        }
    }

    std::vector<CloudPoint>& GetPointClound() {
        return g_pointCloud;
    }
}