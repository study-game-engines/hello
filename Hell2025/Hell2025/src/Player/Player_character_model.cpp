#include "Player.h"
#include "Input/Input.h"

void Player::InitCharacterModel() {
    m_characterModelAnimatedGameObject.SetSkinnedModel("UniSexGuyScaled");
    m_characterModelAnimatedGameObject.SetMeshMaterialByMeshName("CC_Base_Body", "UniSexGuyBody");
    m_characterModelAnimatedGameObject.SetMeshMaterialByMeshName("CC_Base_Eye", "UniSexGuyBody");
    m_characterModelAnimatedGameObject.SetMeshMaterialByMeshName("Biker_Jeans", "UniSexGuyJeans");
    m_characterModelAnimatedGameObject.SetMeshMaterialByMeshName("CC_Base_Eye", "UniSexGuyEyes");
    m_characterModelAnimatedGameObject.SetMeshMaterialByMeshName("Glock", "Glock");
    m_characterModelAnimatedGameObject.SetMeshMaterialByMeshName("SM_Knife_01", "Knife");
    m_characterModelAnimatedGameObject.SetMeshMaterialByMeshName("Shotgun_Mesh", "Shotgun");
    m_characterModelAnimatedGameObject.SetMeshMaterialByMeshIndex(13, "UniSexGuyHead");
    m_characterModelAnimatedGameObject.SetMeshMaterialByMeshIndex(14, "UniSexGuyLashes");
    m_characterModelAnimatedGameObject.EnableBlendingByMeshIndex(14);
    m_characterModelAnimatedGameObject.SetMeshMaterialByMeshName("FrontSight_low", "AKS74U_0");
    m_characterModelAnimatedGameObject.SetMeshMaterialByMeshName("Receiver_low", "AKS74U_1");
    m_characterModelAnimatedGameObject.SetMeshMaterialByMeshName("BoltCarrier_low", "AKS74U_1");
    m_characterModelAnimatedGameObject.SetMeshMaterialByMeshName("SafetySwitch_low", "AKS74U_0");
    m_characterModelAnimatedGameObject.SetMeshMaterialByMeshName("MagRelease_low", "AKS74U_0");
    m_characterModelAnimatedGameObject.SetMeshMaterialByMeshName("Pistol_low", "AKS74U_2");
    m_characterModelAnimatedGameObject.SetMeshMaterialByMeshName("Trigger_low", "AKS74U_1");
    m_characterModelAnimatedGameObject.SetMeshMaterialByMeshName("Magazine_Housing_low", "AKS74U_3");
    m_characterModelAnimatedGameObject.SetMeshMaterialByMeshName("BarrelTip_low", "AKS74U_4");
}

void Player::UpdateCharacterModelHacks() {
    WeaponInfo* weaponInfo = GetCurrentWeaponInfo();
    WeaponState* weaponState = GetCurrentWeaponState();

    m_characterModelAnimatedGameObject.EnableDrawingForAllMesh();

    bool isAlive = true;

    if (isAlive) {

        if (weaponInfo->type == WeaponType::MELEE) {
            HideAKS74UMesh();
            HideGlockMesh();
            HideShotgunMesh();
            if (IsMoving()) {
                m_characterModelAnimatedGameObject.PlayAndLoopAnimation("UnisexGuy_Knife_Walk", 1.0f);
            }
            else {
                m_characterModelAnimatedGameObject.PlayAndLoopAnimation("UnisexGuy_Knife_Idle", 1.0f);
            }
            if (IsCrouching()) {
                m_characterModelAnimatedGameObject.PlayAndLoopAnimation("UnisexGuy_Knife_Crouch", 1.0f);
            }
        }
        if (weaponInfo->type == WeaponType::PISTOL) {
            HideAKS74UMesh();
            HideShotgunMesh();
            HideKnifeMesh();
            if (IsMoving()) {
                m_characterModelAnimatedGameObject.PlayAndLoopAnimation("UnisexGuy_Glock_Walk", 1.0f);
            }
            else {
                m_characterModelAnimatedGameObject.PlayAndLoopAnimation("UnisexGuy_Glock_Idle", 1.0f);
            }
            if (IsCrouching()) {
                m_characterModelAnimatedGameObject.PlayAndLoopAnimation("UnisexGuy_Glock_Crouch", 1.0f);
            }
        }
        if (weaponInfo->type == WeaponType::AUTOMATIC) {
            HideShotgunMesh();
            HideKnifeMesh();
            HideGlockMesh();
            if (IsMoving()) {
                m_characterModelAnimatedGameObject.PlayAndLoopAnimation("UnisexGuy_AKS74U_Walk", 1.0f);
            }
            else {
                m_characterModelAnimatedGameObject.PlayAndLoopAnimation("UnisexGuy_AKS74U_Idle", 1.0f);
            }
            if (IsCrouching()) {
                m_characterModelAnimatedGameObject.PlayAndLoopAnimation("UnisexGuy_AKS74U_Crouch", 1.0f);
            }
        }
        if (weaponInfo->type == WeaponType::SHOTGUN) {
            HideAKS74UMesh();
            HideKnifeMesh();
            HideGlockMesh();
            if (IsMoving()) {
                m_characterModelAnimatedGameObject.PlayAndLoopAnimation("UnisexGuy_Shotgun_Walk", 1.0f);
            }
            else {
                m_characterModelAnimatedGameObject.PlayAndLoopAnimation("UnisexGuy_Shotgun_Idle", 1.0f);
            }
            if (IsCrouching()) {
                m_characterModelAnimatedGameObject.PlayAndLoopAnimation("UnisexGuy_Shotgun_Crouch", 1.0f);
            }
        }

        m_characterModelAnimatedGameObject.SetPosition(GetFootPosition());
        m_characterModelAnimatedGameObject.SetRotationY(m_camera.GetEulerRotation().y + HELL_PI);
    }
    else {
        HideKnifeMesh();
        HideGlockMesh();
        HideShotgunMesh();
        HideAKS74UMesh();
    }
}

void Player::HideKnifeMesh() {
    m_characterModelAnimatedGameObject.DisableDrawingForMeshByMeshName("SM_Knife_01");
}
void Player::HideGlockMesh() {
    m_characterModelAnimatedGameObject.DisableDrawingForMeshByMeshName("Glock");
}
void Player::HideShotgunMesh() {
    m_characterModelAnimatedGameObject.DisableDrawingForMeshByMeshName("Shotgun_Mesh");
}
void Player::HideAKS74UMesh() {
    m_characterModelAnimatedGameObject.DisableDrawingForMeshByMeshName("FrontSight_low");
    m_characterModelAnimatedGameObject.DisableDrawingForMeshByMeshName("Receiver_low");
    m_characterModelAnimatedGameObject.DisableDrawingForMeshByMeshName("BoltCarrier_low");
    m_characterModelAnimatedGameObject.DisableDrawingForMeshByMeshName("SafetySwitch_low");
    m_characterModelAnimatedGameObject.DisableDrawingForMeshByMeshName("MagRelease_low");
    m_characterModelAnimatedGameObject.DisableDrawingForMeshByMeshName("Pistol_low");
    m_characterModelAnimatedGameObject.DisableDrawingForMeshByMeshName("Trigger_low");
    m_characterModelAnimatedGameObject.DisableDrawingForMeshByMeshName("Magazine_Housing_low");
    m_characterModelAnimatedGameObject.DisableDrawingForMeshByMeshName("BarrelTip_low");
}