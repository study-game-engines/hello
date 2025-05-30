#include "Kangaroo.h"
#include "Input/Input.h"
#include "Physics/Physics.h"
#include "Pathfinding/AStarMap.h"
#include "World/World.h"
#include "Core/Game.h"
#include "Timer.hpp"

void Kangaroo::Init(KangarooCreateInfo createInfo) {
    m_createInfo = createInfo;

    Respawn();
    
    if (m_animatedGameObjectId == 0) {
        m_animatedGameObjectId = World::CreateAnimatedGameObject();
        AnimatedGameObject* animatedGameObject = GetAnimatedGameObject();
        animatedGameObject->SetSkinnedModel("Kangaroo");
        animatedGameObject->SetRotationY(HELL_PI);
        animatedGameObject->SetAnimationModeToBindPose();
        animatedGameObject->SetName("Roo");
        animatedGameObject->SetRagdoll("Kangaroo", 1500.0f);
        animatedGameObject->SetAllMeshMaterials("Kangaroo");
        animatedGameObject->SetMeshMaterialByMeshName("LeftEye_Iris", "KangarooIris");
        animatedGameObject->SetMeshMaterialByMeshName("RightEye_Iris", "KangarooIris");
        animatedGameObject->DisableDrawingForMeshByMeshName("LeftEye_Sclera");
        animatedGameObject->DisableDrawingForMeshByMeshName("RightEye_Sclera");
        animatedGameObject->SetMeshWoundMaskTextureIndex("Body", 1);

        Ragdoll* ragdoll = Physics::GetRagdollById(animatedGameObject->GetRagdollId());
        if (ragdoll) {
            ragdoll->SetPhysicsData(animatedGameObject->GetRagdollId(), ObjectType::RAGDOLL_ENEMY);
        }

        AnimationPlaybackParams params;
        params.animationSpeed = 1.0f;
        animatedGameObject->PlayAndLoopAnimation("Kangaroo_Hop2", params);

        m_woundMaskIndex = 1;
    }
}

void Kangaroo::Respawn() {
    m_createInfo.position = glm::vec3(45, 30.6, 39);
    m_createInfo.rotation = glm::vec3(0, HELL_PI * -0.5f, 0);

    m_position = m_createInfo.position;
    m_rotation = m_createInfo.rotation;
    m_alive = true;
    m_health = m_maxHealth;

    m_agroState = KanagarooAgroState::CHILLING;
    m_movementState = KanagarooMovementState::IDLE;
    m_animationState = KanagarooAnimationState::IDLE;

    AnimatedGameObject* animatedGameObject = GetAnimatedGameObject();
    if (animatedGameObject) {
        animatedGameObject->SetPosition(m_position);
        animatedGameObject->SetRotationX(m_rotation.x);
        animatedGameObject->SetRotationY(m_rotation.y);
        animatedGameObject->SetRotationZ(m_rotation.z);

        AnimationPlaybackParams params;
        params.animationSpeed = 1.00f;
        animatedGameObject->PlayAndLoopAnimation("Kangaroo_Idle", params);
    }

}

void Kangaroo::Update(float deltaTime) {

    if (m_animationState == KanagarooAnimationState::BITE) {
        m_timeSinceBiteBegan += deltaTime;
    }
    else {
        m_timeSinceBiteBegan = 0.0f;
    }
    if (m_animationState == KanagarooAnimationState::IDLE) {
        m_timeSinceIdleBegan += deltaTime;
    }
    else {
        m_timeSinceIdleBegan = 0.0f;
    }

        
    if (Input::KeyPressed(HELL_KEY_COMMA)) {
        //SetMovementState(KanagarooMovementState::HOP);

        glm::vec3 targetPosition = glm::vec3(42, 32, 40);
        GoToTarget(targetPosition);
    }

    if (Input::KeyPressed(HELL_KEY_PERIOD)) {
        Respawn();
    }

    UpdateAnimationStateMachine();
    UpdateMovementLogic(deltaTime);
    UpdateAudio();
    //DebugDraw();


    if (Input::KeyPressed(HELL_KEY_SLASH) || true || false || true == false) {

        Player* player = Game::GetLocalPlayerByIndex(0);;
        glm::vec3 playerPosition = player->GetCameraPosition();

        glm::ivec2 start = AStarMap::GetCellCoordsFromWorldSpacePosition(m_position);
        glm::ivec2 end = AStarMap::GetCellCoordsFromWorldSpacePosition(playerPosition);
        m_aStar.InitSearch(start.x, start.y, end.x, end.y);
        m_aStar.FindPath();

        //auto path = GetPath();
        //std::cout << "Path size: " << path.size() << "\n";
    }

   // UpdateMovement(deltaTime);

    // Death check
    if (m_health <= 0) {
        Kill();
    }
    m_health = glm::clamp(m_health, 0, 9999999);
}

AnimatedGameObject* Kangaroo::GetAnimatedGameObject(){
    return World::GetAnimatedGameObjectByObjectId(m_animatedGameObjectId);
}

void Kangaroo::Kill() {
    if (m_alive) {
        Audio::PlayAudio("Kangaroo_Death.wav", 1.0f);

        AnimatedGameObject* animatedGameObject = GetAnimatedGameObject();
        animatedGameObject->SetAnimationModeToRagdoll();
        m_health = 0;
        m_alive = false;
        m_agroState = KanagarooAgroState::KANGAROO_DEAD;
        m_animationState = KanagarooAnimationState::RAGDOLL;
        m_movementState = KanagarooMovementState::KANGAROO_DEAD;
        std::cout << "Killed kangaroo\n";
    }
}

void Kangaroo::GiveDamage(int damage) {
    m_health -= damage;
    Player* player = Game::GetLocalPlayerByIndex(0);
    glm::vec3 playerPosition = player->GetCameraPosition();
    GoToTarget(playerPosition);
    PlayFleshAudio();
    m_agroState = KanagarooAgroState::ANGRY;
}

void Kangaroo::CleanUp() {
    AnimatedGameObject* animatedGameObject = GetAnimatedGameObject();
    if (animatedGameObject) {
        animatedGameObject->CleanUp();
    }
}


void Kangaroo::SetAgroState(KanagarooAgroState state) {
    m_agroState = state;
}

void Kangaroo::SetMovementState(KanagarooMovementState state) {
    m_movementState = state;
}

void Kangaroo::PlayAnimation(const std::string& animationName, float speed) {
    AnimatedGameObject* animatedGameObject = GetAnimatedGameObject();
    if (animatedGameObject) {
        auto params = AnimationPlaybackParams::GetDefaultLoopingPararms();
        params.animationSpeed = speed;
        animatedGameObject->PlayAnimation(animationName, params);
    }
}

void Kangaroo::PlayAndLoopAnimation(const std::string& animationName, float speed) {
    AnimatedGameObject* animatedGameObject = GetAnimatedGameObject();
    if (animatedGameObject) {
        auto params = AnimationPlaybackParams::GetDefaultLoopingPararms();
        params.animationSpeed = speed;
        animatedGameObject->PlayAndLoopAnimation(animationName, params);
    }
}

bool Kangaroo::AnimationIsComplete() {
    AnimatedGameObject* animatedGameObject = GetAnimatedGameObject();

    if (animatedGameObject) {
        for (AnimationState& animationState : animatedGameObject->m_animationLayer.m_animationStates) {
            if (!animationState.IsComplete()) {
                return false;
            }
        }
    }
    return true;
}

glm::vec2 Kangaroo::GetGridPosition() {
    return AStarMap::GetCellCoordsFromWorldSpacePosition(m_position);
}

std::vector<glm::ivec2> Kangaroo::GetPath() {
    return m_aStar.GetPath();
}