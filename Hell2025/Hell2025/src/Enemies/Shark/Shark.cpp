#include "Shark.h"
#include "Renderer/Renderer.h"
#include "Input/Input.h"
#include "World/World.h"

void Shark::Init() {
    //glm::vec3 initialPosition = glm::vec3(38.0f, 11.48f, 24.0f);
    glm::vec3 initialPosition = glm::vec3(27.0f, 9.05f, 22.0f);

    g_animatedGameObjectObjectId = World:: CreateAnimatedGameObject();

    AnimatedGameObject* animatedGameObject = GetAnimatedGameObject();
    animatedGameObject->SetSkinnedModel("Shark");
    animatedGameObject->SetName("GreatestGreatWhiteShark");
    animatedGameObject->SetAllMeshMaterials("Shark");
    animatedGameObject->PlayAndLoopAnimation("Shark_Swim", 1.0f);
    animatedGameObject->SetScale(0.01);

    SkinnedModel* skinnedModel = animatedGameObject->m_skinnedModel;
    std::vector<Node>& nodes = skinnedModel->m_nodes;
    std::map<std::string, unsigned int>& boneMapping = skinnedModel->m_boneMapping;


    // Extract spine positions
    int nodeCount = nodes.size();
    std::vector<glm::mat4> skinnedTransformations(nodeCount);

    for (int i = 0; i < skinnedModel->m_nodes.size(); i++) {
        glm::mat4 nodeTransformation = glm::mat4(1);
        std::string& nodeName = skinnedModel->m_nodes[i].name;
        nodeTransformation = skinnedModel->m_nodes[i].inverseBindTransform;
        unsigned int parentIndex = skinnedModel->m_nodes[i].parentIndex;
        glm::mat4 ParentTransformation = (parentIndex == -1) ? glm::mat4(1) : skinnedTransformations[parentIndex];
        glm::mat4 GlobalTransformation = ParentTransformation * nodeTransformation;
        skinnedTransformations[i] = AnimatedTransform(GlobalTransformation).to_mat4();

        float scale = 0.01f;
        glm::vec3 position = skinnedTransformations[i][3] * scale;

        if (nodeName == "BN_Head_00") {
            m_spinePositions[0] = position;
            m_spineBoneNames[0] = nodeName;
        }
        else if (nodeName == "BN_Neck_01") {
            m_spinePositions[1] = position;
            m_spineBoneNames[1] = nodeName;
        }
        else if (nodeName == "BN_Neck_00") {
            m_spinePositions[2] = position;
            m_spineBoneNames[2] = nodeName;
        }
        else if (nodeName == "Spine_00") {
            m_spinePositions[3] = position;
            m_spineBoneNames[3] = nodeName;
        }
        else if (nodeName == "BN_Spine_01") {
            m_spinePositions[4] = position;
            m_spineBoneNames[4] = nodeName;
        }
        else if (nodeName == "BN_Spine_02") {
            m_spinePositions[5] = position;
            m_spineBoneNames[5] = nodeName;
        }
        else if (nodeName == "BN_Spine_03") {
            m_spinePositions[6] = position;
            m_spineBoneNames[6] = nodeName;
        }
        else if (nodeName == "BN_Spine_04") {
            m_spinePositions[7] = position;
            m_spineBoneNames[7] = nodeName;
        }
        else if (nodeName == "BN_Spine_05") {
            m_spinePositions[8] = position;
            m_spineBoneNames[8] = nodeName;
        }
        else if (nodeName == "BN_Spine_06") {
            m_spinePositions[9] = position;
            m_spineBoneNames[9] = nodeName;
        }
        else if (nodeName == "BN_Spine_07") {
            m_spinePositions[10] = position;
            m_spineBoneNames[10] = nodeName;
        }
    }

    // Compute distances between spine segments
    m_spinePositions[0].y = 0.0f;

    // Reset height
    for (int i = 1; i < SHARK_SPINE_SEGMENT_COUNT; i++) {
        m_spinePositions[i].y = m_spinePositions[0].y;
    }
    // Print names
    for (int i = 0; i < SHARK_SPINE_SEGMENT_COUNT; i++) {
        std::cout << i << ": " << m_spineBoneNames[i] << "\n";
    }
    // Calculate distances
    for (int i = 0; i < SHARK_SPINE_SEGMENT_COUNT - 1; i++) {
        m_spineSegmentLengths[i] = glm::distance(m_spinePositions[i], m_spinePositions[i + 1]);
    }

    m_forward = glm::normalize(m_spinePositions[0] - m_spinePositions[1]);

    SetPosition(initialPosition);

}

void Shark::Update(float deltaTime) {

    // Put these somewhere better!
    m_right = glm::cross(m_forward, glm::vec3(0, 1, 0));
    m_left = -m_right;



    // Move this somewhere
    //animatedGameObject->SetPosition(initialPosition);

    glm::mat4 rootTranslationMatrix = glm::translate(glm::mat4(1), m_spinePositions[3]);
    //glm::mat4 rootRotationMatrix = Util::GetRotationMat4FromForwardVector(m_forward);


      // Root to the end of the spine
    float rot0 = Util::YRotationBetweenTwoPoints(m_spinePositions[3], m_spinePositions[2]) + HELL_PI * 0.5f;
    float rot1 = Util::YRotationBetweenTwoPoints(m_spinePositions[4], m_spinePositions[3]) + HELL_PI * 0.5f;
    float rot2 = Util::YRotationBetweenTwoPoints(m_spinePositions[5], m_spinePositions[4]) + HELL_PI * 0.5f;
    float rot3 = Util::YRotationBetweenTwoPoints(m_spinePositions[6], m_spinePositions[5]) + HELL_PI * 0.5f;
    float rot4 = Util::YRotationBetweenTwoPoints(m_spinePositions[7], m_spinePositions[6]) + HELL_PI * 0.5f;
    float rot5 = Util::YRotationBetweenTwoPoints(m_spinePositions[8], m_spinePositions[7]) + HELL_PI * 0.5f;
    float rot6 = Util::YRotationBetweenTwoPoints(m_spinePositions[9], m_spinePositions[8]) + HELL_PI * 0.5f;
    float rot7 = Util::YRotationBetweenTwoPoints(m_spinePositions[10], m_spinePositions[9]) + HELL_PI * 0.5f;

    // From the neck to the head
    float rot8 = Util::YRotationBetweenTwoPoints(m_spinePositions[3], m_spinePositions[2]) + HELL_PI * 0.5f;
    float rot9 = Util::YRotationBetweenTwoPoints(m_spinePositions[2], m_spinePositions[1]) + HELL_PI * 0.5f;
    float rot10 = Util::YRotationBetweenTwoPoints(m_spinePositions[1], m_spinePositions[0]) + HELL_PI * 0.5f;

    // Calculate root bone matrix
    Transform rotationTransform;
    rotationTransform.rotation.y = rot0;
    glm::mat4 rootRotationMatrix = rotationTransform.to_mat4();
    glm::mat4 rootBoneMatrix = rootTranslationMatrix * rootRotationMatrix;

    std::unordered_map<std::string, glm::mat4> additiveBoneTransforms;
    additiveBoneTransforms["Spine_00"] = rootBoneMatrix;
    additiveBoneTransforms["BN_Spine_01"] = glm::rotate(glm::mat4(1.0f), rot1 - rot0, glm::vec3(0, 1, 0));
    additiveBoneTransforms["BN_Spine_02"] = glm::rotate(glm::mat4(1.0f), rot2 - rot1, glm::vec3(0, 1, 0));
    additiveBoneTransforms["BN_Spine_03"] = glm::rotate(glm::mat4(1.0f), rot3 - rot2, glm::vec3(0, 1, 0));
    additiveBoneTransforms["BN_Spine_04"] = glm::rotate(glm::mat4(1.0f), rot4 - rot3, glm::vec3(0, 1, 0));
    additiveBoneTransforms["BN_Spine_05"] = glm::rotate(glm::mat4(1.0f), rot5 - rot4, glm::vec3(0, 1, 0));
    additiveBoneTransforms["BN_Spine_06"] = glm::rotate(glm::mat4(1.0f), rot6 - rot5, glm::vec3(0, 1, 0));
    additiveBoneTransforms["BN_Spine_07"] = glm::rotate(glm::mat4(1.0f), rot7 - rot6, glm::vec3(0, 1, 0));

    additiveBoneTransforms["BN_Neck_00"] = glm::rotate(glm::mat4(1.0f), rot8 - rot1, glm::vec3(0, 1, 0));
    additiveBoneTransforms["BN_Neck_01"] = glm::rotate(glm::mat4(1.0f), rot9 - rot8, glm::vec3(0, 1, 0));
    additiveBoneTransforms["BN_Head_00"] = glm::rotate(glm::mat4(1.0f), rot10 - rot9, glm::vec3(0, 1, 0));


    // Update animation
    AnimatedGameObject* animatedGameObject = GetAnimatedGameObject();
    if (animatedGameObject) {
        animatedGameObject->Update(deltaTime, additiveBoneTransforms);
        animatedGameObject->UpdateRenderItems();
    }

    if (Input::KeyDown(HELL_KEY_UP)) {
        CalculateForwardVectorFromArrowKeys(deltaTime);
        std::cout << "Shark forward: " << m_forward << "\n";

        for (int i = 0; i < m_logicSubStepCount; i++) {
            MoveShark(deltaTime);
        }
    }

    for (int i = 1; i < SHARK_SPINE_SEGMENT_COUNT; ++i) {
        Renderer::DrawPoint(m_spinePositions[i], RED);
    }
    for (int i = 0; i < 1; ++i) {
        Renderer::DrawPoint(m_spinePositions[i], WHITE);
    }
}

void Shark::CleanUp() {
    World::RemoveObject(g_animatedGameObjectObjectId);
}

void Shark::SetPosition(glm::vec3 position) {
    m_spinePositions[0] = position;
    for (int i = 1; i < SHARK_SPINE_SEGMENT_COUNT; i++) {
        m_spinePositions[i].x = m_spinePositions[0].x;
        m_spinePositions[i].y = m_spinePositions[0].y;
        m_spinePositions[i].z = m_spinePositions[i - 1].z - m_spineSegmentLengths[i - 1];
        //m_rotation = 0;           TRIPLE CHECK YOU DON'T NEED THIS !!!!!!!!!!!!!!!!!!!!!!!!!!
    }
    m_forward = glm::vec3(0, 0, 1);
}


void Shark::MoveShark(float deltaTime) {

    //m_mouthPositionLastFrame = GetMouthPosition2D();
    //m_headPositionLastFrame = GetHeadPosition2D();
    //m_evadePointPositionLastFrame = GetEvadePoint2D();
    // Move head
    m_spinePositions[0] += m_forward * m_swimSpeed * deltaTime / (float)m_logicSubStepCount;
    // Move spine segments
    for (int i = 1; i < SHARK_SPINE_SEGMENT_COUNT; ++i) {
        glm::vec3 direction = glm::normalize(m_spinePositions[i - 1] - m_spinePositions[i]);
        m_spinePositions[i] = m_spinePositions[i - 1] - direction * m_spineSegmentLengths[i - 1];
    }
}

void Shark::CalculateForwardVectorFromArrowKeys(float deltaTime) {
    float maxRotation = 5.0f;
    if (Input::KeyDown(HELL_KEY_LEFT)) {
        float blendFactor = glm::clamp(glm::abs(-maxRotation) / 90.0f, 0.0f, 1.0f);
        m_forward = glm::normalize(glm::mix(m_forward, m_left, blendFactor));
        std::cout << "PRESSED LEFT\n";
    }
    if (Input::KeyDown(HELL_KEY_RIGHT)) {
        float blendFactor = glm::clamp(glm::abs(maxRotation) / 90.0f, 0.0f, 1.0f);
        m_forward = glm::normalize(glm::mix(m_forward, m_right, blendFactor));
        std::cout << "PRESSED RIGHT\n";
    }
}

AnimatedGameObject* Shark::GetAnimatedGameObject() {
   return World::GetAnimatedGameObjectByObjectId(g_animatedGameObjectObjectId);
}

/*

#include "Shark.h"
#include "SharkPathManager.h"
#include "../Game/Game.h"
#include "../Game/Scene.h"
#include "../Game/Water.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "glm/gtx/hash.hpp"
#include "../Input/Input.h"
#include "../Math/LineMath.hpp"

void Shark::Init() {
    glm::vec3 initialPosition = glm::vec3(7.0f, -0.1f, -15.7);

    m_animatedGameObjectIndex = Scene::CreateAnimatedGameObject();
    AnimatedGameObject* animatedGameObject = Scene::GetAnimatedGameObjectByIndex(m_animatedGameObjectIndex);
    animatedGameObject->SetFlag(AnimatedGameObject::Flag::NONE);
    animatedGameObject->SetPlayerIndex(1);
    animatedGameObject->SetSkinnedModel("SharkSkinned");
    animatedGameObject->SetName("Shark");
    animatedGameObject->SetAnimationModeToBindPose();
    animatedGameObject->SetAllMeshMaterials("Shark");
    animatedGameObject->PlayAndLoopAnimation("Shark_Swim", 1.0f);

    // Create ragdoll
    PxU32 raycastFlag = RaycastGroup::RAYCAST_ENABLED;
    PxU32 collsionGroupFlag = CollisionGroup::SHARK;
    PxU32 collidesWithGroupFlag = CollisionGroup::ENVIROMENT_OBSTACLE | CollisionGroup::GENERIC_BOUNCEABLE | CollisionGroup::RAGDOLL | CollisionGroup::PLAYER;
    animatedGameObject->LoadRagdoll("Shark.rag", raycastFlag, collsionGroupFlag, collidesWithGroupFlag);
    m_init = true;

    // Find head position
    SkinnedModel* skinnedModel = animatedGameObject->_skinnedModel;
    std::vector<Joint>& joints = skinnedModel->m_joints;
    std::map<std::string, unsigned int>& boneMapping = skinnedModel->m_BoneMapping;
    std::vector<BoneInfo> boneInfo = skinnedModel->m_BoneInfo;

    for (int i = 0; i < joints.size(); i++) {
        const char* nodeName = joints[i].m_name;
        glm::mat4 NodeTransformation = joints[i].m_inverseBindTransform;
        unsigned int parentIndex = joints[i].m_parentIndex;
        glm::mat4 ParentTransformation = (parentIndex == -1) ? glm::mat4(1) : joints[parentIndex].m_currentFinalTransform;
        glm::mat4 GlobalTransformation = ParentTransformation * NodeTransformation;
        joints[i].m_currentFinalTransform = GlobalTransformation;
        if (boneMapping.find(nodeName) != boneMapping.end()) {
            unsigned int BoneIndex = boneMapping[nodeName];
            boneInfo[BoneIndex].FinalTransformation = GlobalTransformation * boneInfo[BoneIndex].BoneOffset;
            boneInfo[BoneIndex].ModelSpace_AnimatedTransform = GlobalTransformation;

            // No idea why this scale is required
            float scale = 0.01f;
            glm::vec3 position = Util::GetTranslationFromMatrix(GlobalTransformation) * scale;
            if (Util::StrCmp(nodeName, "BN_Head_00")) {
                m_spinePositions[0] = position;
                m_spineBoneNames[0] = nodeName;
            }
            else if (Util::StrCmp(nodeName, "BN_Neck_01")) {
                m_spinePositions[1] = position;
                m_spineBoneNames[1] = nodeName;
            }
            else if (Util::StrCmp(nodeName, "BN_Neck_00")) {
                m_spinePositions[2] = position;
                m_spineBoneNames[2] = nodeName;
            }
            else if (Util::StrCmp(nodeName, "Spine_00")) {
                m_spinePositions[3] = position;
                m_spineBoneNames[3] = nodeName;
            }
            else if (Util::StrCmp(nodeName, "BN_Spine_01")) {
                m_spinePositions[4] = position;
                m_spineBoneNames[4] = nodeName;
            }
            else if (Util::StrCmp(nodeName, "BN_Spine_02")) {
                m_spinePositions[5] = position;
                m_spineBoneNames[5] = nodeName;
            }
            else if (Util::StrCmp(nodeName, "BN_Spine_03")) {
                m_spinePositions[6] = position;
                m_spineBoneNames[6] = nodeName;
            }
            else if (Util::StrCmp(nodeName, "BN_Spine_04")) {
                m_spinePositions[7] = position;
                m_spineBoneNames[7] = nodeName;
            }
            else if (Util::StrCmp(nodeName, "BN_Spine_05")) {
                m_spinePositions[8] = position;
                m_spineBoneNames[8] = nodeName;
            }
            else if (Util::StrCmp(nodeName, "BN_Spine_06")) {
                m_spinePositions[9] = position;
                m_spineBoneNames[9] = nodeName;
            }
            else if (Util::StrCmp(nodeName, "BN_Spine_07")) {
                m_spinePositions[10] = position;
                m_spineBoneNames[10] = nodeName;
            }
        }
    }
    m_spinePositions[0].y = 0.0f;

    // Reset height
    for (int i = 1; i < SHARK_SPINE_SEGMENT_COUNT; i++) {
        m_spinePositions[i].y = m_spinePositions[0].y;
    }
    // Print names
    for (int i = 0; i < SHARK_SPINE_SEGMENT_COUNT; i++) {
        //std::cout << i << ": " << m_spineBoneNames[i] << "\n";
    }
    // Calculate distances
    for (int i = 0; i < SHARK_SPINE_SEGMENT_COUNT - 1; i++) {
        m_spineSegmentLengths[i] = glm::distance(m_spinePositions[i], m_spinePositions[i+1]);
    }

    // Store PxRigidBody pointers
    Ragdoll* ragdoll = GetRadoll();
    for (int i = 0; i < SHARK_SPINE_SEGMENT_COUNT; i++) {
        for (int j = 0; j < ragdoll->m_rigidComponents.size(); j++) {
            RigidComponent& rigidComponent = ragdoll->m_rigidComponents[j];        
            if (rigidComponent.correspondingJointName == m_spineBoneNames[i]) {
                m_rigidComponents[i] = &rigidComponent;
                //std::cout << i << ": " << m_spineBoneNames[i] << " " << rigidComponent.name << "\n";
            }
        }
    }

    if (ragdoll) {
        for (RigidComponent& rigidComponent : ragdoll->m_rigidComponents) {
            if (rigidComponent.name == "rMarker_BN_Head_00") {
                m_headPxRigidDynamic = rigidComponent.pxRigidBody;
                break;
            }
        }
    }
    Reset();
}

void Shark::SetPosition(glm::vec3 position) {
    m_spinePositions[0] = position;
    for (int i = 1; i < SHARK_SPINE_SEGMENT_COUNT; i++) {
        m_spinePositions[i].x = m_spinePositions[0].x;
        m_spinePositions[i].y = m_spinePositions[0].y;
        m_spinePositions[i].z = m_spinePositions[i - 1].z - m_spineSegmentLengths[i - 1];
        m_rotation = 0;
    }
    m_forward = glm::vec3(0, 0, 1);
}

void Shark::GiveDamage(int playerIndex, int damageAmount) {
    m_health -= damageAmount;
    std::cout << "Gave shark damage\n";
    if (m_huntedPlayerIndex == -1) {
        std::cout << "Hunting player: " << m_huntedPlayerIndex << "\n";
        HuntPlayer(playerIndex);
    }
}

void Shark::CleanUp() {
    // to do: move this to ragdoll class, and also destroy the PxShape
    AnimatedGameObject* animatedGameObject = Scene::GetAnimatedGameObjectByIndex(m_animatedGameObjectIndex);
    if (animatedGameObject) {
        Ragdoll& ragdoll = animatedGameObject->m_ragdoll;
        for (RigidComponent& rigidComponent : ragdoll.m_rigidComponents) {
            Physics::Destroy(rigidComponent.pxRigidBody);
        }
        ragdoll.m_rigidComponents.clear();
    }
    m_animatedGameObjectIndex = -1;
}

void Shark::Respawn() {
    SetPositionToBeginningOfPath();
    m_movementState = SharkMovementState::FOLLOWING_PATH;
    m_health = SHARK_HEALTH_MAX;
    m_isDead = false;
    m_hasBitPlayer = false;
    PlayAndLoopAnimation("Shark_Swim", 1.0f);
}

void Shark::Kill() {
    m_health = 0;
    m_isDead = true;
    Audio::PlayAudio("Shark_Death.wav", 1.0f);
    PlayAndLoopAnimation("Shark_Die", 1.0f);
}

void Shark::HuntPlayer(int playerIndex) {
    m_huntedPlayerIndex = playerIndex;
    m_movementState = SharkMovementState::HUNT_PLAYER;
    m_huntingState = SharkHuntingState::CHARGE_PLAYER;
}


// Helper functions




float CalculateAngle(const glm::vec3& from, const glm::vec3& to) {
    return atan2(to.x - from.x, to.z - from.z);

}

float NormalizeAngle(float angle) {
    angle = fmod(angle + HELL_PI, 2 * HELL_PI);
    if (angle < 0) angle += 2 * HELL_PI;
    return angle - HELL_PI;
}

void RotateYTowardsTarget(glm::vec3 objectPosition, float& objectYRotation, const glm::vec3& targetPosition, float rotationSpeed) {
    float desiredAngle = CalculateAngle(objectPosition, targetPosition);
    float angleDifference = NormalizeAngle(desiredAngle - objectYRotation);
    std::cout << "angleDifference: " << angleDifference << "\n";
    if (fabs(angleDifference) < rotationSpeed) {
        objectYRotation = desiredAngle;
    }
    else {
        if (angleDifference > 0) {
            objectYRotation += rotationSpeed;
        }
        else {
            objectYRotation -= rotationSpeed;
        }
    }
    objectYRotation = fmod(objectYRotation, 2 * HELL_PI);
    if (objectYRotation < 0) {
        objectYRotation += 2 * HELL_PI;
    }
}

float Shark::GetTurningRadius() const {
    float turningRadius = m_swimSpeed / m_rotationSpeed;
    return turningRadius;
}

bool Shark::TargetIsOnLeft(glm::vec3 targetPosition) {
    glm::vec3 lineStart = GetHeadPosition2D();
    glm::vec3 lineEnd = GetCollisionLineEnd();
    glm::vec3 lineNormal = LineMath::GetLineNormal(lineStart, lineEnd);
    glm::vec3 midPoint = LineMath::GetLineMidPoint(lineStart, lineEnd);
    return LineMath::IsPointOnOtherSideOfLine(lineStart, lineEnd, lineNormal, targetPosition);
}

void Shark::SetPositionToBeginningOfPath() {
    AnimatedGameObject* animatedGameObject = Scene::GetAnimatedGameObjectByIndex(m_animatedGameObjectIndex);
    if (SharkPathManager::PathExists()) {
        SharkPath* path = SharkPathManager::GetSharkPathByIndex(0);
        glm::vec3 position = path->m_points[0].position;
        position.y = 1.4f;
        SetPosition(position);
        m_nextPathPointIndex = 1;
    }
}

void Shark::Reset() {
    SetPositionToBeginningOfPath();
    m_movementState = SharkMovementState::FOLLOWING_PATH;
    m_huntingState = SharkHuntingState::UNDEFINED;
    m_health = SHARK_HEALTH_MAX;
    m_forward = glm::vec3(0, 0, 1.0f);
    m_huntedPlayerIndex = -1;
    m_nextPathPointIndex = 1;
    PlayAndLoopAnimation("Shark_Swim", 1.0f);
}
*/