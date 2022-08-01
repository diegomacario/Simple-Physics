#ifndef WORLD_H
#define WORLD_H

#include <vector>

#include "RigidBody.h"
#include "Wall.h"

class World
{
public:

   World();
   ~World() = default;

   void                          initializeRigidBodies();
   void                          initializeWalls();

   bool                          simulate(float deltaTime);

   const std::vector<RigidBody>& getRigidBodies() const { return mRigidBodies; };
   const std::vector<Wall>&      getWalls() const { return mWalls; };

private:

   enum class CollisionState : unsigned int
   {
      penetrating = 0,
      colliding   = 1,
      clear       = 2,
   };

   void                          computeForces();

   void                          integrateUsingEulers(float deltaTime);
   void                          integrateUsingRK4(float deltaTime);

   CollisionState                checkForCollisions();

   void                          resolveCollisions();

   void                          orthonormalizeOrientation(glm::mat3& orientation);

   std::vector<RigidBody> mRigidBodies;
   std::vector<Wall>      mWalls;

   CollisionState         mCollisionState;
   int                    mCollidingRigidBodyIndex;
   int                    mCollidingVertexIndex;
   glm::vec3              mCollisionNormal;
};

#endif
