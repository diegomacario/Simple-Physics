#include "Quat.h"
#include "World.h"

World::World()
{
   initializeRigidBodies();
   initializeWalls();
}

void World::initializeRigidBodies()
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
                             0.25f,            // Width
                             0.25f,            // Height
                             0.25f,            // Depth
                             1.0f,             // Coefficient of restitution
                             glm::vec3(0.0f, 0.0f, 0.0f), // Position of CM
                             orientation,      // Orientation
                             glm::vec3(0.0f),  // Velocity of CM,
                             glm::vec3(0.0f)); // Angular momentum
}

void World::initializeWalls()
{
   // Right
   glm::mat3 orientation = glm::mat3(Q::quatToMat4(Q::lookRotation(glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f))));
   mWalls.emplace_back(glm::vec3(2.5f, 0.0f, 0.0f), orientation, 2.5f, 2.5f);

   // Left
   orientation = glm::mat3(Q::quatToMat4(Q::lookRotation(glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f))));
   mWalls.emplace_back(glm::vec3(-2.5f, 0.0f, 0.0f), orientation, 2.5f, 2.5f);

   // Front
   orientation = glm::mat3(Q::quatToMat4(Q::lookRotation(glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, 1.0f, 0.0f))));
   mWalls.emplace_back(glm::vec3(0.0f, 0.0f, 2.5f), orientation, 2.5f, 2.5f);

   // Back
   orientation = glm::mat3(Q::quatToMat4(Q::lookRotation(glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, 1.0f, 0.0f))));
   mWalls.emplace_back(glm::vec3(0.0f, 0.0f, -2.5f), orientation, 2.5f, 2.5f);

   // Top
   orientation = glm::mat3(Q::quatToMat4(Q::lookRotation(glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f))));
   mWalls.emplace_back(glm::vec3(0.0f, 2.5f, 0.0f), orientation, 2.5f, 2.5f);

   // Bottom
   orientation = glm::mat3(Q::quatToMat4(Q::lookRotation(glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f))));
   mWalls.emplace_back(glm::vec3(0.0f, -2.5f, 0.0f), orientation, 2.5f, 2.5f);
}

void World::simulate(float deltaTime)
{

}
