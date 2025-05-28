#pragma once
#include "Types/Game/AnimatedGameObject.h"
#include <vector>
#include <string>

struct Ragdoll {
    Ragdoll() = default;

    //void SetKinematicState(bool state);
    //
    //void EnableVisualization();
    //void DisableVisualization();
    //void EnableCollision();
    //void DisableCollision();
    //void CleanUp();

    void SetFilterData(PhysicsFilterData physicsFilterData);
    void SetPhysicsData(uint64_t objectId, ObjectType objectType);
    void ActivatePhysics();
    void PrintCorrespondingBoneNames();
    void SetRigidGlobalPosesFromAnimatedGameObject(AnimatedGameObject* animatedGameObject);

    
    glm::mat4 GetRigidWorlTransform(const std::string& correspondingBoneName);

    std::string m_name = "";
    std::vector<std::string> m_correspondingBoneNames;
    std::vector<uint64_t> m_rigidDynamicIds;
    std::vector<uint64_t> m_d6JointIds;
};