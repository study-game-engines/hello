#include "Player.h"

void Player::InitRagdoll() {
    m_characterModelAnimatedGameObject.SetRagdoll("UnisexGuy", 85.0f);
    Ragdoll* ragdoll = Physics::GetRagdollById(m_characterModelAnimatedGameObject.GetRagdollId());

    if (ragdoll) {
        PhysicsFilterData filterData;
        filterData.raycastGroup = RaycastGroup::RAYCAST_ENABLED;
        filterData.collisionGroup = CollisionGroup::RAGDOLL_PLAYER;
        filterData.collidesWith = CollisionGroup(ENVIROMENT_OBSTACLE);

        ragdoll->SetFilterData(filterData);
        ragdoll->SetPhysicsData(GetPlayerId(), ObjectType::RAGDOLL_PLAYER);
        std::cout << "initilzeded player ragdoll\n";
    }
}