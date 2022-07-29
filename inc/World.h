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

   void initializeRigidBodies();
   void initializeWalls();

   void simulate(float deltaTime);

   const std::vector<RigidBody>& getRigidBodies() const { return mRigidBodies; };
   const std::vector<Wall>& getWalls() const { return mWalls; };

private:

   std::vector<RigidBody> mRigidBodies;
   std::vector<Wall>      mWalls;
};

#endif
