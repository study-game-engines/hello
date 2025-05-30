#include "Kangaroo.h"
#include "Input/Input.h"
#include "Physics/Physics.h"
#include "World/World.h"

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
        animatedGameObject->SetAllMeshMaterials("CheckerBoard");
        animatedGameObject->SetAllMeshMaterials("Leopard");
        animatedGameObject->PrintMeshNames();
        animatedGameObject->SetRagdoll("Kangaroo", 1500.0f);

        animatedGameObject->PrintMeshNames();
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
    m_position = m_createInfo.position;
    m_rotation = m_createInfo.rotation;
    m_alive = true;
    m_health = m_maxHealth;

    AnimatedGameObject* animatedGameObject = GetAnimatedGameObject();
    if (animatedGameObject) {
        AnimationPlaybackParams params;
        params.animationSpeed = 1.00f;
        animatedGameObject->PlayAndLoopAnimation("Kangaroo_Idle", params);
    }
}

void Kangaroo::Update(float deltaTime) {
    AnimatedGameObject* animatedGameObject = GetAnimatedGameObject();
    if (Input::KeyPressed(HELL_KEY_SLASH)) {
        animatedGameObject->SetAnimationModeToRagdoll();
    }
    if (Input::KeyPressed(HELL_KEY_COMMA)) {
        AnimationPlaybackParams params;
        params.animationSpeed = 1.00f;
        animatedGameObject->PlayAndLoopAnimation("Kangaroo_Idle", params);
    }
    if (Input::KeyPressed(HELL_KEY_PERIOD)) {
        AnimationPlaybackParams params;
        params.animationSpeed = 1.0f;
        animatedGameObject->PlayAndLoopAnimation("Kangaroo_Bite", params);
    }

    // outside spawn
    animatedGameObject->SetPosition(glm::vec3(49.5, 30.4f, 39));
    animatedGameObject->SetRotationY(HELL_PI * -0.5);
    animatedGameObject->SetScale(1.0f);

    // inside room  spawn
    animatedGameObject->SetPosition(glm::vec3(17.1, 30.4f, 41.15));
    animatedGameObject->SetRotationY(HELL_PI);
    animatedGameObject->SetScale(0.85f);

    //roo->SetPosition(glm::vec3(29, 30.4f, 39));
    //roo->SetRotationY(HELL_PI * -0.85);
    animatedGameObject->SetName("Roo");
    animatedGameObject->SetAllMeshMaterials("Kangaroo");
    animatedGameObject->SetMeshMaterialByMeshName("LeftEye_Iris", "KangarooIris");
    animatedGameObject->SetMeshMaterialByMeshName("RightEye_Iris", "KangarooIris");
    animatedGameObject->DisableDrawingForMeshByMeshName("LeftEye_Sclera");
    animatedGameObject->DisableDrawingForMeshByMeshName("RightEye_Sclera");

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
        std::cout << "Killed kangaroo\n";
    }
}

void Kangaroo::GiveDamage(int damage) {
    m_health -= damage;
    std::cout << "Kangaroo health: " << m_health << "\n";
}

void Kangaroo::CleanUp() {
    AnimatedGameObject* animatedGameObject = GetAnimatedGameObject();
    if (animatedGameObject) {
        animatedGameObject->CleanUp();
    }
}