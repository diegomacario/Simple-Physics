#ifndef WORLD_H
#define WORLD_H

#include <vector>

#include "RigidBody.h"

class World
{
public:

   World();
   ~World() = default;

   void initialize();

   void simulate(float deltaTime);

   const std::vector<RigidBody>& getRigidBodies() const { return mRigidBodies; };

private:

   std::vector<RigidBody> mRigidBodies;
};

#endif
