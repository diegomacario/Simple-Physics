#ifndef WORLD_H
#define WORLD_H

#include <vector>

#include "Triangle.h"
#include "SimpleMesh.h"
#include "RigidBody.h"
#include "DecalRenderer.h"

class World
{
public:

   World(const std::shared_ptr<DecalRenderer>& decalRenderer);
   ~World() = default;

   void                          initializeRigidBodies();
   void                          initializeWorldTriangles();

   bool                          simulate(float deltaTime, bool gravity, int velocityChange);

   const std::vector<RigidBody>& getRigidBodies() const { return mRigidBodies; };

   void changeScene(int sceneIndex);

private:

   enum class CollisionState : unsigned int
   {
      penetrating = 0,
      colliding   = 1,
      clear       = 2,
   };

   void                          computeForces(bool gravity);

   void                          integrateUsingEulers(float deltaTime);
   void                          integrateUsingRK4(float deltaTime);

   CollisionState                checkForCollisions();

   void                          resolveCollisions();

   void                          orthonormalizeOrientation(glm::mat3& orientation);

   std::vector<Triangle>         getTrianglesFromMeshes(std::vector<SimpleMesh>& meshes);

   std::vector<RigidBody>             mRigidBodies;
   std::vector<std::vector<Triangle>> mWorldTriangles;
   int                                mCurrentScene;

   CollisionState                     mCollisionState;
   int                                mCollidingRigidBodyIndex;
   int                                mCollidingVertexIndex;
   glm::vec3                          mCollisionNormal;

   glm::vec3                          mDecalNormal;
   bool                               mFirstCheckForCollisions;

   std::shared_ptr<DecalRenderer>     mDecalRenderer;
};

#endif
