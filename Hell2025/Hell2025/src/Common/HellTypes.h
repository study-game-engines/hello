#pragma once
#define GLM_FORCE_SILENT_WARNINGS
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <vector>
#include <iostream>
#include "HellEnums.h"
#include "HellDefines.h"
#include "Input/keycodes.h"

struct BlitRegion {
    int32_t originX = 0;
    int32_t originY = 0;
    int32_t width = 0;
    int32_t height = 0;
};

struct BlitRect {
    int32_t x0 = 0;
    int32_t y0 = 0;
    int32_t x1 = 0;
    int32_t y1 = 0;
};

struct RenderItem {
    glm::mat4 modelMatrix = glm::mat4(1);
    glm::mat4 inverseModelMatrix = glm::mat4(1);
    glm::vec4 aabbMin = glm::vec4(0);
    glm::vec4 aabbMax = glm::vec4(0);

    int meshIndex = 0;
    int baseColorTextureIndex = 0;
    int normalMapTextureIndex = 0;
    int rmaTextureIndex = 0;

    int mousePickType = 0;
    int mousePickIndex = 0;
    int baseSkinnedVertex = 0;
    int ignoredViewportIndex = -1;

    int exclusiveViewportIndex = -1;
    int skinned = 0;    // True or false
    float emissiveR = 0.0f;
    float emissiveG = 0.0f;

    float emissiveB = 0.0f;
    float padding0 = 0.0f;
    float padding1 = 0.0f;
    float padding2 = 0.0f;
};

struct HouseRenderItem {
    int baseColorTextureIndex = 0;
    int normalMapTextureIndex = 0;
    int rmaTextureIndex = 0;
    int baseVertex = 0;
    int baseIndex = 0;
    int vertexCount = 0;
    int indexCount = 0;
    int padding = 0;
    glm::vec4 aabbMin;
    glm::vec4 aabbMax;
};

struct RenderItem2D {
    glm::mat4 modelMatrix = glm::mat4(1);
    float colorTintR = 1.0f;
    float colorTintG = 1.0f;
    float colorTintB = 1.0f;
    int textureIndex = -1;
    int baseVertex = 0;
    int baseIndex = 0;
};

struct SpriteSheetRenderItem {
    glm::vec4 position;
    glm::vec4 rotation;
    glm::vec4 scale;
    int textureIndex;
    int frameIndex;
    int frameIndexNext;
    int rowCount;
    int columnCount;
    int isBillboard;
    float mixFactor;
    float padding;
};

struct Vertex2D {
    glm::vec2 position;
    glm::vec2 uv;
    glm::vec4 color;
    int textureIndex;
};

struct MeshData2D {
    std::vector<Vertex2D> vertices;
    std::vector<uint32_t> indices;
};

struct Vertex {
    Vertex() = default;
    Vertex(glm::vec3 pos) {
        position = pos;
    }
    Vertex(glm::vec3 pos, glm::vec3 norm) {
        position = pos;
        normal = norm;
    }
    Vertex(glm::vec3 pos, glm::vec3 norm, glm::vec2 texCoord) {
        position = pos;
        normal = norm;
        uv = texCoord;
    }
    Vertex(glm::vec3 pos, glm::vec3 norm, glm::vec2 texCoord, glm::vec3 _tangent) {
        position = pos;
        normal = norm;
        uv = texCoord;
        tangent = _tangent;
    }
    glm::vec3 position = glm::vec3(0);
    glm::vec3 normal = glm::vec3(0);
    glm::vec2 uv = glm::vec2(0);
    glm::vec3 tangent = glm::vec3(0);
};

struct WeightedVertex {
    glm::vec3 position = glm::vec3(0);
    glm::vec3 normal = glm::vec3(0);
    glm::vec2 uv = glm::vec2(0);
    glm::vec3 tangent = glm::vec3(0);
    glm::ivec4 boneID = glm::ivec4(0);
    glm::vec4 weight = glm::vec4(0);

    bool operator==(const Vertex& other) const {
        return position == other.position && normal == other.normal && uv == other.uv;
    }
};

#pragma pack(push, 1)
struct DebugVertex {
    glm::vec3 position;
    glm::vec3 color;
    glm::ivec2 pixelOffset;
    int exclusiveViewportIndex = -1;
};
#pragma pack(pop)

struct TextureData {
    int m_width = 0;
    int m_height = 0;
    int m_channelCount = 0;
    int m_dataSize = 0;
    int m_format = 0;
    int m_internalFormat = 0;
    void* m_data = nullptr;
    ImageDataType m_imageDataType;
};

struct FileInfo {
    std::string path;
    std::string name;
    std::string ext;
    std::string dir;
    std::string GetFileNameWithExtension() {
        if (ext.length() > 0) {
            return name + "." + ext;
        }
        else {
            return name;
        }
    }
};

struct Transform {
	glm::vec3 position = glm::vec3(0);
	glm::vec3 rotation = glm::vec3(0);
	glm::vec3 scale = glm::vec3(1);

    Transform() = default;

    explicit Transform(const glm::vec3& position, const glm::vec3& rotation = glm::vec3(0.0f), const glm::vec3& scale = glm::vec3(1.0f)) {
        this->position = position;
        this->rotation = rotation;
        this->scale = scale;
    }

	glm::mat4 to_mat4() const {
		glm::mat4 m = glm::translate(glm::mat4(1), position);
		m *= glm::mat4_cast(glm::quat(rotation));
		m = glm::scale(m, scale);
		return m;
	};
};

struct AnimatedTransform {
    AnimatedTransform() = default;
    AnimatedTransform(glm::mat4 matrix) {
        glm::vec3 skew;
        glm::vec4 perspective;
        glm::decompose(matrix, scale, rotation, translation, skew, perspective);
    }
    glm::mat4 to_mat4() {
        glm::mat4 m = glm::translate(glm::mat4(1), translation);
        m *= glm::mat4_cast(rotation);
        m = glm::scale(m, scale);
        return m;
    };
    glm::vec3 to_forward_vector() {
        glm::quat q = glm::quat(rotation);
        return glm::normalize(q * glm::vec3(0.0f, 0.0f, 1.0f));
    }
    glm::vec3 translation = glm::vec3(0);
    glm::quat rotation = glm::quat(1, 0, 0, 0);
    glm::vec3 scale = glm::vec3(1);
};

struct Material {
    Material() {}
    std::string m_name = UNDEFINED_STRING;
    int m_basecolor = 0;
    int m_normal = 0;
    int m_rma = 0;
};

struct QueuedTextureBake {
    void* texture = nullptr;
    int jobID = 0;
    int width = 0;
    int height = 0;
    int format = 0;
    int internalFormat = 0;
    int mipmapLevel = 0;
    int dataSize = 0;
    const void* data = nullptr;
    bool inProgress = false;
};

struct ViewportData {
    glm::mat4 projection;
    glm::mat4 inverseProjection;
    glm::mat4 view;
    glm::mat4 inverseView;
    glm::mat4 projectionView;
    glm::mat4 inverseProjectionView;
    glm::mat4 skyboxProjectionView;
    glm::mat4 flashlightProjectionView;
    int xOffset;
    int yOffset;
    int width;
    int height;
    float posX;  // 0 t0 1 range
    float posY;  // 0 t0 1 range
    float sizeX; // 0 t0 1 range
    float sizeY; // 0 t0 1 range
    glm::vec4 frustumPlane0;
    glm::vec4 frustumPlane1;
    glm::vec4 frustumPlane2;
    glm::vec4 frustumPlane3;
    glm::vec4 frustumPlane4;
    glm::vec4 frustumPlane5;
    glm::vec4 flashlightDir;
    glm::vec4 flashlightPosition;
    float flashlightModifer;
    bool isOrtho;
    float orthoSize;
    float padding2;
};

struct RendererData {
    float nearPlane;
    float farPlane;
    float gBufferWidth;
    float gBufferHeight;

    float hairBufferWidth;
    float hairBufferHeight;
    float time;
    int splitscreenMode;

    int rendererOverrideState;
    float normalizedMouseX;
    float normalizedMouseY;
};

struct Resolutions {
    glm::ivec2 gBuffer;
    glm::ivec2 finalImage;
    glm::ivec2 ui;
    glm::ivec2 hair;
};

struct DrawIndexedIndirectCommand {
    uint32_t indexCount = 0;
    uint32_t instanceCount = 0;
    uint32_t firstIndex = 0;
    int32_t  baseVertex = 0;
    uint32_t baseInstance = 0;
};

struct DrawArraysIndirectCommand {
    uint32_t vertexCount = 0;
    uint32_t instanceCount = 0;
    uint32_t firstVertex = 0;
    uint32_t baseInstance = 0;
};

struct DrawCommands {
    std::vector<DrawIndexedIndirectCommand> perViewport[4]; // One for each splitscreen player
};

struct DrawCommandsSet {
    DrawCommands geometry;
    DrawCommands geometryBlended;
    DrawCommands geometryAlphaDiscarded;
    DrawCommands hairTopLayer;
    DrawCommands hairBottomLayer;

    DrawCommands skinnedGeometry;
};

struct WaterState {
    float heightBeneathWater = 0;
    float heightAboveWater = 0;

    bool cameraUnderWater = false;
    bool feetUnderWater = false;
    bool wading = false;
    bool swimming = false;

    // Previous frame
    bool cameraUnderWaterPrevious = false;
    bool feetUnderWaterPrevious = false;
    bool wadingPrevious = true;
    bool swimmingPrevious = true;
};

struct PlayerControls {
    unsigned int WALK_FORWARD = HELL_KEY_W;
    unsigned int WALK_BACKWARD = HELL_KEY_S;
    unsigned int WALK_LEFT = HELL_KEY_A;
    unsigned int WALK_RIGHT = HELL_KEY_D;
    unsigned int INTERACT = HELL_KEY_E;
    unsigned int RELOAD = HELL_KEY_R;
    unsigned int FIRE = HELL_MOUSE_LEFT;
    unsigned int ADS = HELL_MOUSE_RIGHT;
    unsigned int JUMP = HELL_KEY_SPACE;
    unsigned int CROUCH = HELL_KEY_WIN_CONTROL; // different mapping to the standard glfw HELL_KEY_LEFT_CONTROL
    unsigned int NEXT_WEAPON = HELL_KEY_Q;
    unsigned int ESCAPE = HELL_KEY_WIN_ESCAPE;
    unsigned int DEBUG_FULLSCREEN = HELL_KEY_G;
    unsigned int DEBUG_ONE = HELL_KEY_1;
    unsigned int DEBUG_TWO = HELL_KEY_2;
    unsigned int DEBUG_THREE = HELL_KEY_3;
    unsigned int DEBUG_FOUR = HELL_KEY_4;
    unsigned int MELEE = HELL_KEY_V;
    unsigned int FLASHLIGHT = HELL_KEY_F;
    unsigned int MISC_WEAPON_FUNCTION = HELL_KEY_T;
};

/*
struct Bullet {
    glm::vec3 spawnPosition;
    glm::vec3 direction;
    //Weapon type;
    uint32_t raycastFlags;
    glm::vec3 parentPlayersViewRotation;
    int damage = 0;
    int parentPlayerIndex = -1;
};*/

struct PhysicsFilterData {
    RaycastGroup raycastGroup = RaycastGroup::RAYCAST_DISABLED;
    CollisionGroup collisionGroup = CollisionGroup::NO_COLLISION;
    CollisionGroup collidesWith = CollisionGroup::ENVIROMENT_OBSTACLE;
};

struct GPULight {
    float posX;
    float posY;
    float posZ;
    float colorR;

    float colorG;
    float colorB;
    float strength;
    float radius;
};

struct SelectionRectangleState {
    int beginX = 0;
    int beginY = 0;
    int currentX = 0;
    int currentY = 0;
};

struct PhysXRayResult {
    std::string hitObjectName;
    glm::vec3 hitPosition;
    glm::vec3 surfaceNormal;
    glm::vec3 rayDirection;
    bool hitFound;
    void* hitActor = nullptr;
    void* userData = nullptr;
};

struct MeshRenderingInfo {
    uint32_t meshIndex;
    uint32_t materialIndex;
    BlendingMode blendingMode;
};

enum struct TreeType {
    TREE_LARGE_0 = 0,
    TREE_LARGE_1,
    TREE_LARGE_2,
    UNDEFINED
};

enum class RendererOverrideState {
    NONE = 0,
    BASE_COLOR,
    NORMALS,
    RMA,
    CAMERA_NDOTL,
    STATE_COUNT,
};  

struct RendererSettings {
    int depthPeelCount = 4;
    bool drawGrass = true;
    RendererOverrideState rendererOverrideState = RendererOverrideState::NONE;
};

struct Node {
    std::string name;
    int parentIndex;
    glm::mat4 inverseBindTransform;
};

struct SpawnOffset {
    float positionX;
    float positionZ;
};

struct vecXZ {
    float x = 0.0f;
    float z = 0.0f;

    vecXZ() : x(0.0f), z(0.0f) {}
    vecXZ(float x, float z) : x(x), z(z) {}

    bool operator==(const vecXZ& other) const {
        return x == other.x && z == other.z;
    }

    bool operator!=(const vecXZ& other) const {
        return !(*this == other);
    }

    bool operator<(const vecXZ& other) const {
        return (x < other.x) || (x == other.x && z < other.z);
    }

    bool operator>(const vecXZ& other) const {
        return other < *this;
    }

    bool operator<=(const vecXZ& other) const {
        return !(other < *this);
    }

    bool operator>=(const vecXZ& other) const {
        return !(*this < other);
    }
};

struct ivecXZ {
    int x = 0;
    int z = 0;

    ivecXZ() : x(0), z(0) {}
    ivecXZ(int x, int z) : x(x), z(z) {}

    bool operator==(const ivecXZ& other) const {
        return x == other.x && z == other.z;
    }

    bool operator!=(const ivecXZ& other) const {
        return !(*this == other);
    }

    bool operator<(const ivecXZ& other) const {
        return (x < other.x) || (x == other.x && z < other.z);
    }

    bool operator>(const ivecXZ& other) const {
        return other < *this;
    }

    bool operator<=(const ivecXZ& other) const {
        return !(other < *this);
    }

    bool operator>=(const ivecXZ& other) const {
        return !(*this < other);
    }
};

struct HeightMapChunk {
    ivecXZ coord;
    int baseIndex = 0;
    int baseVertex = 0;
    glm::vec3 aabbMin;
    glm::vec3 aabbMax;
};

struct PhysicsUserData {
    uint64_t physicsId;
    uint64_t objectId;
    PhysicsType physicsType;
    ObjectType objectType;
};

struct OverlapResult {
    PhysicsUserData userData;
    glm::vec3 objectPosition;
};

struct OverlapReport {
    std::vector<OverlapResult> hits;
    bool HitsFound() {
        return hits.size();
    }
};