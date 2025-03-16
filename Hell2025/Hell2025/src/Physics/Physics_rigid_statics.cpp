#include "Physics.h"
#include "UniqueID.h"
#include "AssetManagement/AssetManager.h"
#include "Physics/Types/RigidStatic.h"
#include <unordered_map>
#include <vector>
#include "Util.h"

namespace Physics {

    std::unordered_map<uint64_t, RigidStatic> g_rigidStatics;

    uint64_t CreateRigidStaticFromBoxExtents(Transform transform, glm::vec3 boxExtents, PhysicsFilterData filterData) {
        PxPhysics* pxPhysics = Physics::GetPxPhysics();
        PxScene* pxScene = Physics::GetPxScene();
        PxMaterial* material = Physics::GetDefaultMaterial();

        float halfWidth = boxExtents.x * 0.5f;
        float halfHeight = boxExtents.y * 0.5f;
        float halfDepth = boxExtents.z * 0.5f;

        PxFilterData pxFilterData;
        pxFilterData.word0 = (PxU32)filterData.raycastGroup;
        pxFilterData.word1 = (PxU32)filterData.collisionGroup;
        pxFilterData.word2 = (PxU32)filterData.collidesWith;

        // Create shape
        PxShape* pxShape = pxPhysics->createShape(PxBoxGeometry(halfWidth, halfHeight, halfDepth), *material, true);
        pxShape->setQueryFilterData(pxFilterData);       // ray casts
        pxShape->setSimulationFilterData(pxFilterData);  // collisions
        pxShape->setFlag(PxShapeFlag::eSIMULATION_SHAPE, true);
        pxShape->setFlag(PxShapeFlag::eSCENE_QUERY_SHAPE, true);

        // Create rigid dynamic
        PxQuat quat = Physics::GlmQuatToPxQuat(glm::quat(transform.rotation));
        PxTransform pxTransform = PxTransform(PxVec3(transform.position.x, transform.position.y, transform.position.z), quat);
        PxRigidStatic* pxRigidStatic = pxPhysics->createRigidStatic(pxTransform);
        pxRigidStatic->attachShape(*pxShape);
        pxScene->addActor(*pxRigidStatic);

        // Create DynamicBox
        uint64_t physicsID = UniqueID::GetNext();
        RigidStatic& rigidDynamic = g_rigidStatics[physicsID];

        // Update its pointers
        rigidDynamic.SetPxRigidStatic(pxRigidStatic);
        rigidDynamic.AddPxShape(pxShape);

        return physicsID;
    }


   uint64_t CreateRigidStaticFromConvexMeshVertices(Transform transform, const std::span<Vertex>& vertices, PhysicsFilterData filterData) {
       PxPhysics* pxPhysics = Physics::GetPxPhysics();
       PxScene* pxScene = Physics::GetPxScene();
       PxMaterial* material = Physics::GetDefaultMaterial();
  
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
       PxRigidStatic* pxRigidStatic = pxPhysics->createRigidStatic(pxTransform);
       pxRigidStatic->attachShape(*pxShape);
       pxScene->addActor(*pxRigidStatic);
    
       // Create Rigid Static
       uint64_t physicsID = UniqueID::GetNext();
       RigidStatic& rigidStatic = g_rigidStatics[physicsID];
  
       // Update its pointers
       rigidStatic.SetPxRigidStatic(pxRigidStatic);
       rigidStatic.AddPxShape(pxShape);
  
       return physicsID;
   }



   uint64_t CreateRigidStaticFromConvexMeshFromModel(Transform transform, const std::string& modelName, PhysicsFilterData filterData) {


       PxPhysics* pxPhysics = Physics::GetPxPhysics();
       PxScene* pxScene = Physics::GetPxScene();
       PxMaterial* material = Physics::GetDefaultMaterial();

       PxFilterData pxFilterData;
       pxFilterData.word0 = (PxU32)filterData.raycastGroup;
       pxFilterData.word1 = (PxU32)filterData.collisionGroup;
       pxFilterData.word2 = (PxU32)filterData.collidesWith;

       ///////////////////////////////////////////////////////////////////////////////////////////////

       Model* model = AssetManager::GetModelByName(modelName);
       if (!model) {
           std::cout << "Physics::CreateRigidStaticFromConvexMeshFromModel() failed: '" << modelName << "' was not found \"n";
           return 0;
       }


       ///////////////////////////////////////////////////////////////////////////////////////////////

       // Create rigid dynamic
       PxQuat quat = Physics::GlmQuatToPxQuat(glm::quat(transform.rotation));
       PxTransform pxTransform = PxTransform(PxVec3(transform.position.x, transform.position.y, transform.position.z), quat);
       PxRigidStatic* pxRigidStatic = pxPhysics->createRigidStatic(pxTransform);

       // Create Rigid Static
       uint64_t physicsID = UniqueID::GetNext();
       RigidStatic& rigidStatic = g_rigidStatics[physicsID];

       // Create convex shapes
       for (uint32_t meshIndex : model->GetMeshIndices()) {
           Mesh* mesh = AssetManager::GetMeshByIndex(meshIndex);
           std::span<Vertex> vertices = AssetManager::GetVerticesSpan(mesh->baseVertex, mesh->vertexCount);

           std::vector<PxVec3> pxVertices;
           for (Vertex& vertex : vertices) {
               pxVertices.push_back(Physics::GlmVec3toPxVec3(vertex.position));
           }

           PxConvexMeshDesc convexDesc;
           convexDesc.points.count = pxVertices.size();
           convexDesc.points.stride = sizeof(PxVec3);
           convexDesc.points.data = pxVertices.data();
           convexDesc.flags = PxConvexFlag::eSHIFT_VERTICES | PxConvexFlag::eCOMPUTE_CONVEX;

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
           pxRigidStatic->attachShape(*pxShape);
           rigidStatic.AddPxShape(pxShape);
       }

       pxScene->addActor(*pxRigidStatic);

       // Update its pointers
       rigidStatic.SetPxRigidStatic(pxRigidStatic);

       return physicsID;
   }





   void MarkRigidStaticForRemoval(uint64_t rigidStaticId) {
       if (RigidStaticExists(rigidStaticId)) {
           RigidStatic& rigidStatic = g_rigidStatics[rigidStaticId];
           rigidStatic.MarkForRemoval();
       }
   }

   void RemoveRigidStatic(uint64_t rigidStaticId) {
       if (RigidStaticExists(rigidStaticId)) {
           PxPhysics* pxPhysics = Physics::GetPxPhysics();
           PxScene* pxScene = Physics::GetPxScene();
           RigidStatic& rigidStatic = g_rigidStatics[rigidStaticId];
           PxRigidStatic* pxRigidStatic = rigidStatic.GetPxRigidStatic();

           // Remove rigid
           if (pxRigidStatic) {

               // Clean up its user data
               if (pxRigidStatic->userData) {
                   delete static_cast<PhysicsUserData*>(pxRigidStatic->userData);
                   pxRigidStatic->userData = nullptr;
               }
               // Remove it from PxScene
               if (pxRigidStatic->getScene() != nullptr) {
                   pxScene->removeActor(*pxRigidStatic);
               }
               // Release it
               pxRigidStatic->release();
               pxRigidStatic = nullptr;
           }

           // Remove the shapes
           std::vector<PxShape*>& pxShapes = rigidStatic.GetPxShapes();
           for (PxShape* pxShape : pxShapes) {
               if (pxShape) {
                   pxShape->release();
               }
           }

           // Remove from container
           g_rigidStatics.erase(rigidStaticId);
       }
   }

   void RemoveAnyRigidStaticForRemoval() {
       PxScene* pxScene = Physics::GetPxScene();

       for (auto it = g_rigidStatics.begin(); it != g_rigidStatics.end(); ) {
           RigidStatic& rigidStatic = it->second;
           if (rigidStatic.IsMarkedForRemoval()) {
               // Retrieve pointers
               PxRigidStatic* pxRigidStatic = rigidStatic.GetPxRigidStatic();

               // Remove the actor
               if (pxRigidStatic) {
                   if (pxRigidStatic->getScene() != nullptr) {
                       pxScene->removeActor(*pxRigidStatic);
                   }
               }

               // Release the shapes
               std::vector<PxShape*>& pxShapes = rigidStatic.GetPxShapes();
               for (PxShape* pxShape : pxShapes) {
                   if (pxShape) {
                       pxShape->release();
                   }
               }

               // Release the actor
               if (pxRigidStatic) {
                   pxRigidStatic->release();
               }

               // Remove from container
               it = g_rigidStatics.erase(it);
           }
           else {
               ++it;
           }
       }
   }

   void SetRigidStaticGlobalPose(uint64_t rigidStaticId, glm::mat4 globalPoseMatrix) {
       if (!RigidStaticExists(rigidStaticId)) return;
           
       RigidStatic& rigidStatic = g_rigidStatics[rigidStaticId];
       PxRigidStatic* pxRigidStatic = rigidStatic.GetPxRigidStatic();
       PxMat44 pxMatrix = GlmMat4ToPxMat44(globalPoseMatrix);
       PxTransform pxTransform = PxTransform(pxMatrix);
       pxRigidStatic->setGlobalPose(pxTransform);
   }

   glm::mat4 GetRigidStaticGlobalPose(uint64_t rigidStaticId) {
       if (!RigidStaticExists(rigidStaticId)) return glm::mat4(1.0f);

       RigidStatic& rigidStatic = g_rigidStatics[rigidStaticId];
       PxRigidStatic* pxRigidStatic = rigidStatic.GetPxRigidStatic();
       PxTransform pxTransform = pxRigidStatic->getGlobalPose();
       return PxMat44ToGlmMat4(pxTransform);       
   }

   void SetRigidStaticUserData(uint64_t m_physicsId, PhysicsUserData physicsUserData) {
       if (RigidStaticExists(m_physicsId)) {
           RigidStatic& rigidStatic = g_rigidStatics[m_physicsId];
           PxRigidStatic* pxRigidStatic = rigidStatic.GetPxRigidStatic();
           pxRigidStatic->userData = new PhysicsUserData(physicsUserData);
       }
   }

   bool RigidStaticExists(uint64_t rigidStaticId) {
       return (g_rigidStatics.find(rigidStaticId) != g_rigidStatics.end());
   }
}