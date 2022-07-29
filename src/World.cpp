#include "World.h"

World::World()
{
   initialize();
}

void World::initialize()
{
   // Construct a rotation around the Z axis
   //float theta = M_PI / 2.0f;
   float theta = 0.0f;
   glm::mat3 orientation(0.0f);
   orientation[0][0] =  glm::cos(theta);
   orientation[0][1] =  glm::sin(theta);
   orientation[1][0] = -glm::sin(theta);
   orientation[1][1] =  glm::cos(theta);
   orientation[2][2] = 1;

   mRigidBodies.emplace_back(1.0f,             // Mass
                             2.0f,             // Width
                             1.0f,             // Height
                             0.5f,             // Depth
                             1.0f,             // Coefficient of restitution
                             glm::vec3(0.0f, 0.0f, 0.0f), // Position of CM
                             orientation,      // Orientation
                             glm::vec3(0.0f),  // Velocity of CM,
                             glm::vec3(0.0f)); // Angular momentum
}

void World::simulate(float deltaTime)
{

}
