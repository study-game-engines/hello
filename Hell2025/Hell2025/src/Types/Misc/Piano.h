#pragma once
#include "HellTypes.h"
#include "Types/Game/BasicDoor.h"
#include "Types/Renderer/Model.h"
#include <unordered_map>
#include "CreateInfo.h"

struct PianoKey {
    int32_t m_note = 0;
    uint32_t m_meshIndex = 0;
    Material* m_material = nullptr;
    glm::vec3 m_worldSpaceCenter = glm::vec3(0.0f);
    float m_xRotation = 0;
    float m_yTranslation = 0;
    bool m_isSharp = false;
    glm::mat4 m_localOffsetMatrix = glm::mat4(1.0f);
    float m_timeRemaining = 0.0f;
    bool m_sustain = false;

    void Update(float deltaTime);
    void UpdateWorldSpaceCenter(glm::mat4 parentPianoModelMatrix);
    void PressKey(int velocity = 127, float duration = 0.05f);
    //void ReleaseKey();

    enum struct State {
        IDLE,
        KEY_PRESSED,
    } m_state = State::IDLE;
};

struct PianoBodyPart {
    Material* m_material = nullptr;
    uint32_t m_meshIndex = 0;

    //const Material* GetMaterial() const { return m_material; }
}; 

struct Piano {

    Piano() = default;
    void Init(PianoCreateInfo& createInfo);
    void SetPosition(glm::vec3 position);
    void SetSustain(bool value);
    void Update(float deltaTime);
    void CleanUp();
    void SubmitRenderItems();
    void TriggerInternalNoteFromExternalBulletHit(glm::vec3 bulletHitPositon);
    
    void PlayMajorFirstInversion(int rootNote); 
    void PlayMajor7th(int rootNote);
    void PlayMinor(int rootNote);
    void PlayMajor(int rootNote);
    void PlayKey(int note, int velocity = 127, float duration = 0.05f);

    bool PianoKeyExists(uint64_t pianoKeyId);
    bool PianoBodyPartKeyExists(uint64_t pianoBodyPartId);
    PianoKey* GetPianoKey(uint64_t pianoKeyId);
    PianoBodyPart* GetPianoBodyPart(uint64_t pianoBodyPartId);

    const std::vector<RenderItem>& GetRenderItems() const   { return m_renderItems; }
    const uint64_t& GetObjectId() const                     { return m_pianoObjectId; };
    const glm::vec3 GetPosition() const                     { return m_createInfo.position; }
    const glm::vec3& GetSeatPosition() const                { return m_seatPosition; }
    const PianoCreateInfo GetCreateInfo() const             { return m_createInfo; }

    static uint32_t MeshNameToNote(const std::string& meshName);

private:
    void UpdateRenderItems();

    glm::vec3 m_seatPosition = glm::vec3(0.0f);
    uint64_t m_pianoObjectId = 0;
    uint64_t m_rigidStaticId = 0;
    Model* m_model = nullptr;
    Transform m_transform;
    std::vector<RenderItem> m_renderItems;
    glm::mat4 m_modelMatrix;
    PianoCreateInfo m_createInfo;

    std::unordered_map<uint64_t, PianoBodyPart> m_bodyParts; // Mapped to object Id
    std::unordered_map<uint64_t, PianoKey> m_keys;           // Mapped to object Id
};