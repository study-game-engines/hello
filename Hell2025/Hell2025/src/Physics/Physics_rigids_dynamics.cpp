#include "Physics.h"
#include "UniqueID.h"
#include "Physics/Types/RigidDynamic.h"
#include <unordered_map>
#include <vector>
#include "Util.h"

namespace Physics {

    std::unordered_map<uint64_t, RigidDynamic> g_rigidDynamics;
    std::vector<AABB> g_activeRigidDynamicAABBs;

    void UpdateActiveRigidDynamicAABBList() {
        g_activeRigidDynamicAABBs.clear();
        for (auto it = g_rigidDynamics.begin(); it != g_rigidDynamics.end(); ) {
            RigidDynamic& rigidDynamic = it->second;
            PxRigidDynamic* pxRigidDynamic = rigidDynamic.GetPxRigidDynamic();
            if (!pxRigidDynamic->getRigidBodyFlags().isSet(PxRigidBodyFlag::eKINEMATIC)) {
                g_activeRigidDynamicAABBs.push_back(rigidDynamic.GetAABB());
            }
            it++;
        }
    }

    void UpdateAllRigidDynamics(float deltaTime) {        
        for (auto it = g_rigidDynamics.begin(); it != g_rigidDynamics.end(); ) {
            RigidDynamic& rigidDynamic = it->second;
            rigidDynamic.Update(deltaTime);
            it++;
        }
    }

    uint64_t CreateRigidDynamicFromConvexMeshVertices(Transform transform, const std::span<Vertex>& vertices, const std::span<uint32_t>& indices, float mass, PhysicsFilterData filterData, glm::vec3 initialForce, glm::vec3 initialTorque) {
        PxPhysics* pxPhysics = Physics::GetPxPhysics();
        PxScene* pxScene = Physics::GetPxScene();
        PxMaterial* material = Physics::GetDefaultMaterial();

        float volume = Util::GetConvexHullVolume(vertices, indices);
        float density = Util::GetDensity(mass, volume);

        PxFilterData pxFilterData;
        pxFilterData.word0 = (PxU32)filterData.raycastGroup;
        pxFilterData.word1 = (PxU32)filterData.collisionGroup;
        pxFilterData.word2 = (PxU32)filterData.collidesWith;

        ///////////////////////////////////////////////////////////////////////////////////////////////
        
        // Create convex shape
        std::vector<PxVec3> pxVertices;
        for (Vertex& vertex : vertices) {
            pxVertices.push_back(Physics::GlmVec3toPxVec3(vertex.position));
        }

        PxConvexMeshDesc convexDesc;
        convexDesc.points.count = pxVertices.size();
        convexDesc.points.stride = sizeof(PxVec3);
        convexDesc.points.data = pxVertices.data();
        convexDesc.flags = PxConvexFlag::eSHIFT_VERTICES | PxConvexFlag::eCOMPUTE_CONVEX;
        //  s
        PxTolerancesScale scale;
        PxCookingParams params(scale);

        PxDefaultMemoryOutputStream buf;
        PxConvexMeshCookingResult::Enum result;
        if (!PxCookConvexMesh(params, convexDesc, buf, &result)) {
            std::cout << "some convex mesh shit failed\n";
            return 0;
        }
        PxDefaultMemoryInputData input(buf.getData(), buf.getSize());
        PxConvexMesh* convexMesh = pxPhysics->createConvexMesh(input);
        PxConvexMeshGeometryFlags flags(~PxConvexMeshGeometryFlag::eTIGHT_BOUNDS);
        PxConvexMeshGeometry geometry(convexMesh, PxMeshScale(PxVec3(1.0f)), flags);

        PxShape* pxShape = pxPhysics->createShape(geometry, *material);
        pxShape->setQueryFilterData(pxFilterData);       // ray casts
        pxShape->setSimulationFilterData(pxFilterData);  // collisions

        ///////////////////////////////////////////////////////////////////////////////////////////////

        // Create rigid dynamic
        PxQuat quat = Physics::GlmQuatToPxQuat(glm::quat(transform.rotation));
        PxTransform pxTransform = PxTransform(PxVec3(transform.position.x, transform.position.y, transform.position.z), quat);
        PxRigidDynamic* pxRigidDynamic = pxPhysics->createRigidDynamic(pxTransform);
        pxRigidDynamic->attachShape(*pxShape);
        PxRigidBodyExt::updateMassAndInertia(*pxRigidDynamic, density);
        pxScene->addActor(*pxRigidDynamic);

        // Apply impulse
        PxVec3 force = PxVec3(initialForce.x, initialForce.y, initialForce.z);
        pxRigidDynamic->addForce(force, PxForceMode::eIMPULSE);

        // Apply torque
        PxVec3 torque = PxVec3(initialTorque.x, initialTorque.y, initialTorque.z);
        pxRigidDynamic->addTorque(torque);

        // Create DynamicBox
        uint64_t physicsID = UniqueID::GetNext();
        RigidDynamic& rigidDynamic = g_rigidDynamics[physicsID];

        // Update its pointers
        rigidDynamic.SetPxRigidDynamic(pxRigidDynamic);
        rigidDynamic.SetPxShape(pxShape);

        return physicsID;
    }

    uint64_t CreateRigidDynamicFromBoxExtents(Transform transform, glm::vec3 boxExtents, float mass, PhysicsFilterData filterData, glm::vec3 initialForce, glm::vec3 initialTorque) {
        PxPhysics* pxPhysics = Physics::GetPxPhysics();
        PxScene* pxScene = Physics::GetPxScene();
        PxMaterial* material = Physics::GetDefaultMaterial();

        float halfWidth = boxExtents.x * 0.5f;
        float halfHeight = boxExtents.y * 0.5f;
        float halfDepth = boxExtents.z * 0.5f;
        float volume = Util::GetCubeVolume(halfWidth, halfHeight, halfDepth);
        float density = Util::GetDensity(mass, volume);

        PxFilterData pxFilterData;
        pxFilterData.word0 = (PxU32)filterData.raycastGroup;
        pxFilterData.word1 = (PxU32)filterData.collisionGroup;
        pxFilterData.word2 = (PxU32)filterData.collidesWith;

        // Create shape
        PxShape* pxShape = pxPhysics->createShape(PxBoxGeometry(halfWidth, halfHeight, halfDepth), *material, true);
        pxShape->setQueryFilterData(pxFilterData);       // ray casts
        pxShape->setSimulationFilterData(pxFilterData);  // collisions

        // Create rigid dynamic
        PxQuat quat = Physics::GlmQuatToPxQuat(glm::quat(transform.rotation));
        PxTransform pxTransform = PxTransform(PxVec3(transform.position.x, transform.position.y, transform.position.z), quat);
        PxRigidDynamic* pxRigidDynamic = pxPhysics->createRigidDynamic(pxTransform);
        pxRigidDynamic->attachShape(*pxShape);
        PxRigidBodyExt::updateMassAndInertia(*pxRigidDynamic, density);
        pxScene->addActor(*pxRigidDynamic);

        // Apply impulse
        PxVec3 force = PxVec3(initialForce.x, initialForce.y, initialForce.z);
        pxRigidDynamic->addForce(force, PxForceMode::eIMPULSE);

        // Apply torque
        PxVec3 torque = PxVec3(initialTorque.x, initialTorque.y, initialTorque.z);
        pxRigidDynamic->addTorque(torque);

        // Create DynamicBox
        uint64_t physicsID = UniqueID::GetNext();
        RigidDynamic& rigidDynamic = g_rigidDynamics[physicsID];

        // Update its pointers
        rigidDynamic.SetPxRigidDynamic(pxRigidDynamic);
        rigidDynamic.SetPxShape(pxShape);

        return physicsID;
    }
       
    bool RigidDynamicExists(uint64_t rigidDynamicId) {
        return (g_rigidDynamics.find(rigidDynamicId) != g_rigidDynamics.end());
    }

    glm::mat4 GetRigidDynamicWorldMatrix(uint64_t rigidDynamicId) {
        if (RigidDynamicExists(rigidDynamicId)) {
            RigidDynamic& rigidDynamic = g_rigidDynamics[rigidDynamicId];
            PxRigidDynamic* pxRigidDynamic = rigidDynamic.GetPxRigidDynamic();
            if (pxRigidDynamic) {
                return Physics::PxMat44ToGlmMat4(pxRigidDynamic->getGlobalPose());
            }
        }
        return glm::mat4(1);
    }

    void MarkRigidDynamicForRemoval(uint64_t rigidDynamicId) {
        if (RigidDynamicExists(rigidDynamicId)) {
            RigidDynamic& rigidDynamic = g_rigidDynamics[rigidDynamicId];
            rigidDynamic.MarkForRemoval();
        }
    }

    void AddFoceToRigidDynamic(uint64_t rigidDynamicId, glm::vec3 force) {
        if (RigidDynamicExists(rigidDynamicId)) {
            RigidDynamic& rigidDynamic = g_rigidDynamics[rigidDynamicId];
            PxRigidDynamic* pxRigidDynamic = rigidDynamic.GetPxRigidDynamic();
            PxVec3 pxForce = Physics::GlmVec3toPxVec3(force);
            pxRigidDynamic->setRigidBodyFlag(PxRigidBodyFlag::eKINEMATIC, false);
            pxRigidDynamic->addForce(pxForce);
        }
    }

    void RemoveRigidDynamic(uint64_t rigidDynamicId) {
        if (RigidDynamicExists(rigidDynamicId)) {
            PxPhysics* pxPhysics = Physics::GetPxPhysics();
            PxScene* pxScene = Physics::GetPxScene();
            RigidDynamic& rigidDynamic = g_rigidDynamics[rigidDynamicId];
            PxRigidDynamic* pxRigidDynamic = rigidDynamic.GetPxRigidDynamic();
            PxShape* pxShape = rigidDynamic.GetPxShape();

            // Remove rigid
            if (pxRigidDynamic) {

                // Clean up its user data
                if (pxRigidDynamic->userData) {
                    delete static_cast<PhysicsUserData*>(pxRigidDynamic->userData);
                    pxRigidDynamic->userData = nullptr;
                }
                // Remove it from PxScene
                if (pxRigidDynamic->getScene() != nullptr) {
                    pxScene->removeActor(*pxRigidDynamic);
                }
                // Release it
                pxRigidDynamic->release();
                pxRigidDynamic = nullptr;
            }

            // Remove shape
            if (pxShape) {
                pxShape->release();
                pxShape = nullptr;
            }

            // Remove from container
            g_rigidDynamics.erase(rigidDynamicId);
        }
    }

    void RemoveAnyRigidDynamicMarkedForRemoval() {
        PxScene* pxScene = Physics::GetPxScene();

        for (auto it = g_rigidDynamics.begin(); it != g_rigidDynamics.end(); ) {
            RigidDynamic& rigidDynamic = it->second;
            if (rigidDynamic.IsMarkedForRemoval()) {
                // Retrieve pointers
                PxRigidDynamic* pxRigidDynamic = rigidDynamic.GetPxRigidDynamic();
                PxShape* pxShape = rigidDynamic.GetPxShape();

                // Remove the actor from the scene if active
                if (pxRigidDynamic && rigidDynamic.HasActivePhysics()) {
                    if (pxRigidDynamic->getScene() != nullptr) {
                        pxScene->removeActor(*pxRigidDynamic);
                    }
                }

                // Release the shape
                if (pxShape) {
                    pxShape->release();
                }

                // Release the actor
                if (pxRigidDynamic) {
                    pxRigidDynamic->release();
                }

                // Remove from container
                it = g_rigidDynamics.erase(it);
            }
            else {
                ++it;
            }
        }
    }

    void ActivateRigidDynamicPhysics(uint64_t m_physicsId) {
        if (RigidDynamicExists(m_physicsId)) {
            RigidDynamic& rigidDynamic = g_rigidDynamics[m_physicsId];
            rigidDynamic.ActivatePhsyics();
        }
    }

    void DeactivateRigidDynamicPhysics(uint64_t m_physicsId) {
        if (RigidDynamicExists(m_physicsId)) {
            RigidDynamic& rigidDynamic = g_rigidDynamics[m_physicsId];
            rigidDynamic.DeactivatePhysics();
        }
    }    

    void SetRigidDynamicUserData(uint64_t m_physicsId, PhysicsUserData physicsUserData) {
        if (RigidDynamicExists(m_physicsId)) {
            RigidDynamic& rigidDynamic = g_rigidDynamics[m_physicsId];
            PxRigidDynamic* pxRigidDynamic = rigidDynamic.GetPxRigidDynamic();
            pxRigidDynamic->userData = new PhysicsUserData(physicsUserData);
        }
    }

    const std::vector<AABB>& GetActiveRididDynamicAABBs() {
        return g_activeRigidDynamicAABBs;
    }
}