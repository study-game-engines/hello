#include "World.h"

namespace World {

    bool ObjectTypeIsInteractable(ObjectType objectType, uint64_t objectId, glm::vec3 playerCameraPosition, glm::vec3 rayHitPosition) {
        float doorInteractDistance = 1.875f;
        float pickupInteractDistance = 2.0f;
        float pianoKeyInteractDistance = 1.25f;

        if (objectType == ObjectType::DOOR) {
            Door* door = GetDoorByObjectId(objectId);
            return (door && glm::distance(door->GetInteractPosition(), playerCameraPosition) < doorInteractDistance);
        }

        if (objectType == ObjectType::PICK_UP) {
            PickUp* pickUp = GetPickUpByObjectId(objectId);
            return (pickUp && glm::distance(pickUp->GetPosition(), playerCameraPosition) < pickupInteractDistance);
        }

        if (objectType == ObjectType::PIANO_KEY) {
            PianoKey* pianoKey = GetPianoKeyByObjectId(objectId);
            return (pianoKey && glm::distance(pianoKey->m_worldSpaceCenter, playerCameraPosition) < pianoKeyInteractDistance);
        }

        if (objectType == ObjectType::PIANO_KEYBOARD_COVER) {
            return (glm::distance(rayHitPosition, playerCameraPosition) < 2.0f);
        }
        if (objectType == ObjectType::PIANO_SHEET_MUSIC_REST) {
            return (glm::distance(rayHitPosition, playerCameraPosition) < 2.0f);
        }
        if (objectType == ObjectType::PIANO_TOP_COVER) {
            return (glm::distance(rayHitPosition, playerCameraPosition) < 2.0f);
        }



        return false;
    }
}