#include <glm/gtx/matrix_cross_product.hpp>

#include "Transform.h"
#include "GLTFLoader.h"
#include "World.h"

World::World(const std::shared_ptr<DecalRenderer>& decalRenderer)
   : mRigidBodies()
   , mWorldTriangles()
   , mCollisionState(CollisionState::clear)
   , mCollidingRigidBodyIndex(-1)
   , mCollidingVertexIndex(-1)
   , mCollisionNormal()
   , mDecalRenderer(decalRenderer)
   , mCurrentScene(0)
{
   initializeRigidBodies();
   initializeWorldTriangles();
}

void World::initializeRigidBodies()
{
   Transform orientation(glm::vec3(0.0f), Q::angleAxis(glm::radians(45.0f), glm::vec3(0.0f, 0.0f, 1.0f)), glm::vec3(1.0f));
   //orientation = combine(orientation, Transform(glm::vec3(0.0f), Q::angleAxis(glm::radians(65.0f), glm::vec3(0.0f, 1.0f, 0.0f)), glm::vec3(1.0f)));

   mRigidBodies.emplace_back(1.0f, // Mass
                             0.5f, // Width
                             0.25f, // Height
                             0.25f, // Depth
                             1.0f, // Coefficient of restitution
                             glm::vec3(0.0f, 0.0f, 0.0f), // Position of CM
                             glm::mat3(transformToMat4(orientation)), // Orientation
                             glm::vec3(-1.0f, -1.0f, -1.0f), // Velocity of CM,
                             glm::vec3(0.0f)); // Angular momentum
}

void World::initializeWorldTriangles()
{
   cgltf_data* data = LoadGLTFFile("resources/models/inverted_icosphere/inverted_icosphere.glb");
   std::vector<SimpleMesh> invertedIcosphereMeshes = LoadSimpleMeshes(data);
   FreeGLTFFile(data);

   data = LoadGLTFFile("resources/models/inverted_cube/inverted_cube.glb");
   std::vector<SimpleMesh> invertedCubeMeshes = LoadSimpleMeshes(data);
   FreeGLTFFile(data);

   mWorldTriangles.push_back(getTrianglesFromMeshes(invertedIcosphereMeshes));
   mWorldTriangles.push_back(getTrianglesFromMeshes(invertedCubeMeshes));
}

bool World::simulate(float deltaTime)
{
   float currentTime = 0.0f;
   float targetTime  = deltaTime;

   while (currentTime < deltaTime)
   {
      if ((targetTime - currentTime) < 1e-6) // TODO: Make threshold a constant
      {
         changeScene(mCurrentScene);
         return false; // Unresolvable penetration error
      }

      computeForces();

      integrateUsingRK4(targetTime - currentTime);

      // Calculate the world-space vertices of each rigid body at the target time
      for (std::vector<RigidBody>::iterator iter = mRigidBodies.begin(); iter != mRigidBodies.end(); ++iter)
      {
         iter->calculateWorldSpaceVertices(future);
      }

      mFirstCheckForCollisions = true;
      checkForCollisions();
      mFirstCheckForCollisions = false;

      if (mCollisionState == CollisionState::penetrating)
      {
         // We simulated too far, so subdivide time and try again
         targetTime = (currentTime + targetTime) / 2.0f;
         continue;
      }

      if (mCollisionState == CollisionState::colliding)
      {
         int numIterations = 0;
         do
         {
            resolveCollisions();
            numIterations++;
         } while ((checkForCollisions() == CollisionState::colliding) && (numIterations < 100));

         if ((numIterations == 100) && (checkForCollisions() == CollisionState::colliding))
         {
            changeScene(mCurrentScene);
            return false; // Unresolvable collision error
         }
      }

      if (mCollidingRigidBodyIndex != -1 && mCollidingVertexIndex != -1)
      {
         mDecalRenderer->addDecal(mRigidBodies[mCollidingRigidBodyIndex].getState(future).verticesInWorldSpace[mCollidingVertexIndex], mDecalNormal);
      }

      // We made a successful step, so swap configurations to save the data for the next step
      currentTime = targetTime;
      targetTime = deltaTime;

      for (std::vector<RigidBody>::iterator iter = mRigidBodies.begin(); iter != mRigidBodies.end(); ++iter)
      {
         iter->swapStates();
      }

      mCollidingRigidBodyIndex = -1;
      mCollidingVertexIndex = -1;
   }

   return true; // No error
}

void World::changeScene(int sceneIndex)
{
   mRigidBodies.clear();
   initializeRigidBodies();
   mCurrentScene = sceneIndex;
}

void World::computeForces()
{
   for (std::vector<RigidBody>::iterator iter = mRigidBodies.begin(); iter != mRigidBodies.end(); ++iter)
   {
      RigidBody::KinematicAndDynamicState& currentState = iter->getState(current);

      // Clear the torque
      currentState.torque = glm::vec3(0.0f);

      // Force = Mass * Acceleration
      currentState.forceOfCM = glm::vec3(0.0f, 0.0f, 0.0f) / iter->getOneOverMass();
   }
}

void World::integrateUsingEulers(float deltaTime)
{
   for (std::vector<RigidBody>::iterator iter = mRigidBodies.begin(); iter != mRigidBodies.end(); ++iter)
   {
      RigidBody::KinematicAndDynamicState& currentState = iter->getState(current);
      RigidBody::KinematicAndDynamicState& futureState  = iter->getState(future);

      // Integrate primary quantities

      futureState.positionOfCM = currentState.positionOfCM + (currentState.velocityOfCM * deltaTime);

      futureState.orientation = currentState.orientation + (glm::matrixCross3(currentState.angularVelocity) * currentState.orientation * deltaTime);

      futureState.velocityOfCM = currentState.velocityOfCM + (currentState.forceOfCM * iter->getOneOverMass() * deltaTime);

      futureState.angularMomentum = currentState.angularMomentum + (currentState.torque * deltaTime);

      orthonormalizeOrientation(futureState.orientation);

      // Compute auxiliary quantities

      futureState.inverseInertiaTensorInWorldSpace = futureState.orientation * iter->getInverseInertiaTensorInLocalSpace() * glm::transpose(futureState.orientation);

      futureState.angularVelocity = futureState.inverseInertiaTensorInWorldSpace * futureState.angularMomentum;
   }
}

void World::integrateUsingRK4(float deltaTime)
{
   for (std::vector<RigidBody>::iterator iter = mRigidBodies.begin(); iter != mRigidBodies.end(); ++iter)
   {
      RigidBody::KinematicAndDynamicState& currentState = iter->getState(current);
      RigidBody::KinematicAndDynamicState& futureState  = iter->getState(future);

      float midPointOfDeltaTime = deltaTime / 2.0f;

      // Calculate new position and velocity using the classical 4th order Runge-Kutta method

      // We want to solve this 2nd order ODE:
      // F = M * X^dotdot
      // We can solve it by writing it as a system of two 1rst order ODEs:
      // [X^dot] = [  V  ]
      // [V^dot] = [F / M]
      // Where we want to find the position (X) and the velocity (V)

      glm::vec3 k1Pos                     = currentState.velocityOfCM;
      glm::vec3 k1Vel                     = (iter->getOneOverMass() * currentState.forceOfCM);

      //glm::vec3 posAtMidPointOfDeltaTime1 = currentState.positionOfCM + (k1Pos * midPointOfDeltaTime);
      glm::vec3 velAtMidPointOfDeltaTime1 = currentState.velocityOfCM + (k1Vel * midPointOfDeltaTime);

      glm::vec3 k2Pos                     = velAtMidPointOfDeltaTime1;
      glm::vec3 k2Vel                     = (iter->getOneOverMass() * currentState.forceOfCM);

      //glm::vec3 posAtMidPointOfDeltaTime2 = currentState.positionOfCM + (k2Pos * midPointOfDeltaTime);
      glm::vec3 velAtMidPointOfDeltaTime2 = currentState.velocityOfCM + (k2Vel * midPointOfDeltaTime);

      glm::vec3 k3Pos                     = velAtMidPointOfDeltaTime2;
      glm::vec3 k3Vel                     = (iter->getOneOverMass() * currentState.forceOfCM);

      //glm::vec3 posAtDeltaTime            = currentState.positionOfCM + (k3Pos * deltaTime);
      glm::vec3 velAtDeltaTime            = currentState.velocityOfCM + (k3Vel * deltaTime);

      glm::vec3 k4Pos                     = velAtDeltaTime;
      glm::vec3 k4Vel                     = (iter->getOneOverMass() * currentState.forceOfCM);

      glm::vec3 weightedAverageOfPositionSlopes = ((k1Pos + (2.0f * k2Pos) + (2.0f * k3Pos) + k4Pos) / 6.0f);
      glm::vec3 weightedAverageOfVelocitySlopes = ((k1Vel + (2.0f * k2Vel) + (2.0f * k3Vel) + k4Vel) / 6.0f);

      // P_n+1 = P_n + (weightedAverageOfPositionSlopes * h)
      futureState.positionOfCM = currentState.positionOfCM + (weightedAverageOfPositionSlopes * deltaTime);

      // V_n+1 = V_n + (weightedAverageOfVelocitySlopes * h)
      futureState.velocityOfCM = currentState.velocityOfCM + (weightedAverageOfVelocitySlopes * deltaTime);

      // Calculate new orientation and angular velocity using the classical 4th order Runge-Kutta method

      // We want to solve this 2nd order ODE:
      // T = L^dot = (I * W)^dot = I * W^dotdot
      // We can solve it by writing it as a system of two 1rst order ODEs:
      // [O^dot] = [ W ]
      // [L^dot] = [ T ]
      // Where we want to find the orientation (O) and the angular velocity (W)

      glm::vec3 k1Orientation     = currentState.angularVelocity;
      glm::vec3 k1AngularMomentum = currentState.torque;

      glm::mat3 orientationAtMidPointOfDeltaTime1 = currentState.orientation + (glm::matrixCross3(k1Orientation) * currentState.orientation * midPointOfDeltaTime);
      orthonormalizeOrientation(orientationAtMidPointOfDeltaTime1);
      glm::vec3 angularMomentumAtMidPointOfDeltaTime1 = currentState.angularMomentum + (k1AngularMomentum * midPointOfDeltaTime);
      glm::mat3 inverseInertiaTensorInWorldSpaceAtMidPointOfDeltaTime1 = orientationAtMidPointOfDeltaTime1 * iter->getInverseInertiaTensorInLocalSpace() * glm::transpose(orientationAtMidPointOfDeltaTime1);
      glm::vec3 angularVelocityAtMidPointOfDeltaTime1 = inverseInertiaTensorInWorldSpaceAtMidPointOfDeltaTime1 * angularMomentumAtMidPointOfDeltaTime1;

      glm::vec3 k2Orientation     = angularVelocityAtMidPointOfDeltaTime1;
      glm::vec3 k2AngularMomentum = currentState.torque;

      glm::mat3 orientationAtMidPointOfDeltaTime2 = currentState.orientation + (glm::matrixCross3(k2Orientation) * currentState.orientation * midPointOfDeltaTime);
      orthonormalizeOrientation(orientationAtMidPointOfDeltaTime2);
      glm::vec3 angularMomentumAtMidPointOfDeltaTime2 = currentState.angularMomentum + (k2AngularMomentum * midPointOfDeltaTime);
      glm::mat3 inverseInertiaTensorInWorldSpaceAtMidPointOfDeltaTime2 = orientationAtMidPointOfDeltaTime2 * iter->getInverseInertiaTensorInLocalSpace() * glm::transpose(orientationAtMidPointOfDeltaTime2);
      glm::vec3 angularVelocityAtMidPointOfDeltaTime2 = inverseInertiaTensorInWorldSpaceAtMidPointOfDeltaTime2 * angularMomentumAtMidPointOfDeltaTime2;

      glm::vec3 k3Orientation     = angularVelocityAtMidPointOfDeltaTime2;
      glm::vec3 k3AngularMomentum = currentState.torque;

      glm::mat3 orientationAtDeltaTime = currentState.orientation + (glm::matrixCross3(k3Orientation) * currentState.orientation * deltaTime);
      orthonormalizeOrientation(orientationAtDeltaTime);
      glm::vec3 angularMomentumAtDeltaTime = currentState.angularMomentum + (k3AngularMomentum * deltaTime);
      glm::mat3 inverseInertiaTensorInWorldSpaceAtDeltaTime = orientationAtDeltaTime * iter->getInverseInertiaTensorInLocalSpace() * glm::transpose(orientationAtDeltaTime);
      glm::vec3 angularVelocityAtDeltaTime = inverseInertiaTensorInWorldSpaceAtDeltaTime * angularMomentumAtDeltaTime;

      glm::vec3 k4Orientation     = angularVelocityAtDeltaTime;
      glm::vec3 k4AngularMomentum = currentState.torque;

      glm::vec3 weightedAverageOfOrientationSlopes = ((k1Orientation + (2.0f * k2Orientation) + (2.0f * k3Orientation) + k4Orientation) / 6.0f);
      glm::vec3 weightedAverageOfAngularMomentumSlopes = ((k1AngularMomentum + (2.0f * k2AngularMomentum) + (2.0f * k3AngularMomentum) + k4AngularMomentum) / 6.0f);

      // O_n+1 = O_n + (W^~_n * O_n * h)
      futureState.orientation = currentState.orientation + (glm::matrixCross3(weightedAverageOfOrientationSlopes) * currentState.orientation * deltaTime);
      orthonormalizeOrientation(futureState.orientation);

      // L_n+1 = L_n + (T_n * h)
      futureState.angularMomentum = currentState.angularMomentum + (weightedAverageOfAngularMomentumSlopes * deltaTime);

      futureState.inverseInertiaTensorInWorldSpace = futureState.orientation * iter->getInverseInertiaTensorInLocalSpace() * glm::transpose(futureState.orientation);

      futureState.angularVelocity = futureState.inverseInertiaTensorInWorldSpace * futureState.angularMomentum;
   }
}

World::CollisionState World::checkForCollisions()
{
   mCollisionState = CollisionState::clear;
   const float depthEpsilon = 0.001f;

   std::vector<Triangle>& sceneTriangles = mWorldTriangles[mCurrentScene];
   for (int rigidBodyIndex = 0; (rigidBodyIndex < mRigidBodies.size()) && (mCollisionState != CollisionState::penetrating); rigidBodyIndex++)
   {
      RigidBody& rigidBody = mRigidBodies[rigidBodyIndex];
      RigidBody::KinematicAndDynamicState& dynamicAndKinematicState = rigidBody.getState(future);

      for (int vertexIndex = 0; (vertexIndex < 8) && (mCollisionState != CollisionState::penetrating); vertexIndex++)
      {
         glm::vec3 vertexPos  = dynamicAndKinematicState.verticesInWorldSpace[vertexIndex];
         glm::vec3 cmToVertex = vertexPos - dynamicAndKinematicState.positionOfCM;
         glm::vec3 velocity   = dynamicAndKinematicState.velocityOfCM + glm::cross(dynamicAndKinematicState.angularVelocity, cmToVertex);

         for (int triangleIndex = 0; (triangleIndex < sceneTriangles.size()) && (mCollisionState != CollisionState::penetrating); triangleIndex++)
         {
            Triangle& triangle = sceneTriangles[triangleIndex];

            // For a point in space P0, a triangle with normal N and a point on the triangle P1,
            // the distance between P0 and the triangle is given by the projection of (P0 - P1) onto N
            float distBetweenVertexAndTriangle = glm::dot(vertexPos, triangle.normal) - glm::dot(triangle.normal, triangle.vertexA);
            if (distBetweenVertexAndTriangle < -depthEpsilon)
            {
               mCollisionState = CollisionState::penetrating;
            }
            else if (distBetweenVertexAndTriangle < depthEpsilon)
            {
               float relativeVelocity = glm::dot(triangle.normal, velocity);
               if (relativeVelocity < 0.0f)
               {
                  mCollisionState          = CollisionState::colliding;
                  mCollidingRigidBodyIndex = rigidBodyIndex;
                  mCollidingVertexIndex    = vertexIndex;
                  mCollisionNormal         = triangle.normal;

                  if (mFirstCheckForCollisions)
                  {
                     // Uncomment this if you want the decal bounding boxes to look in the direction of the normal of the wall that the body collided with
                     mDecalNormal = mCollisionNormal;

                     // Uncomment this if you want the decal bounding boxes to look in the direction of the colliding body's velocity
                     //mDecalNormal = glm::normalize(dynamicAndKinematicState.velocityOfCM * -1.0f);

                     // Uncomment this if you want the decal bounding boxes to look in the direction of the colliding vertex's velocity
                     //mDecalNormal = glm::normalize(velocity * -1.0f);
                  }
               }
            }
         }
      }
   }

   return mCollisionState;
}

void World::resolveCollisions()
{
   RigidBody& rigidBody = mRigidBodies[mCollidingRigidBodyIndex];
   RigidBody::KinematicAndDynamicState& dynamicAndKinematicState = rigidBody.getState(future);

   glm::vec3 vertexPos = dynamicAndKinematicState.verticesInWorldSpace[mCollidingVertexIndex];

   glm::vec3 cmToVertex = vertexPos - dynamicAndKinematicState.positionOfCM;

   glm::vec3 velocity = dynamicAndKinematicState.velocityOfCM + glm::cross(dynamicAndKinematicState.angularVelocity, cmToVertex);

   float impulseNumerator = -(1.0f + rigidBody.getCoefficientOfRestitution()) * glm::dot(velocity, mCollisionNormal);

   float impulseDenominator = rigidBody.getOneOverMass() + glm::dot(glm::cross(dynamicAndKinematicState.inverseInertiaTensorInWorldSpace * glm::cross(cmToVertex, mCollisionNormal), cmToVertex), mCollisionNormal);

   glm::vec3 impulse = (impulseNumerator / impulseDenominator) * mCollisionNormal;

   // Apply impulse to primary quantities
   dynamicAndKinematicState.velocityOfCM += rigidBody.getOneOverMass() * impulse;
   dynamicAndKinematicState.angularMomentum += glm::cross(cmToVertex, impulse);

   // Compute affected auxiliary quantities
   dynamicAndKinematicState.angularVelocity = dynamicAndKinematicState.inverseInertiaTensorInWorldSpace * dynamicAndKinematicState.angularMomentum;
}

void World::orthonormalizeOrientation(glm::mat3& orientation)
{
   glm::vec3 xAxis(orientation[0][0], orientation[0][1], orientation[0][2]);
   glm::vec3 yAxis(orientation[1][0], orientation[1][1], orientation[1][2]);
   glm::vec3 zAxis;

   xAxis = glm::normalize(xAxis);
   zAxis = glm::normalize(glm::cross(xAxis, yAxis));
   yAxis = glm::normalize(glm::cross(zAxis, xAxis));

   orientation[0][0] = xAxis[0];
   orientation[1][0] = yAxis[0];
   orientation[2][0] = zAxis[0];
   orientation[0][1] = xAxis[1];
   orientation[1][1] = yAxis[1];
   orientation[2][1] = zAxis[1];
   orientation[0][2] = xAxis[2];
   orientation[1][2] = yAxis[2];
   orientation[2][2] = zAxis[2];
}

std::vector<Triangle> World::getTrianglesFromMeshes(std::vector<SimpleMesh>& meshes)
{
   std::vector<Triangle> triangles;

   // Loop over the meshes
   unsigned int numMeshes = static_cast<unsigned int>(meshes.size());
   for (unsigned int meshIndex = 0; meshIndex < numMeshes; ++meshIndex)
   {
      SimpleMesh&               mesh      = meshes[meshIndex];
      std::vector<glm::vec3>    positions = mesh.GetPositions();
      std::vector<unsigned int> indices   = mesh.GetIndices();

      if (indices.size() == 0)
      {
         // If the current mesh doesn't have indices, each sequential set of 3 positions makes up a triangle
         unsigned int numPositions = static_cast<unsigned int>(positions.size());
         for (unsigned int i = 0; i < numPositions; i += 3)
         {
            triangles.push_back(Triangle(positions[i + 0],
                                         positions[i + 1],
                                         positions[i + 2]));
         }
      }
      else
      {
         // If the current mesh has indices, each sequential set of 3 indices makes up a triangle
         unsigned int numIndices = static_cast<unsigned int>(indices.size());
         for (unsigned int i = 0; i < numIndices; i += 3)
         {
            triangles.push_back(Triangle(positions[indices[i + 0]],
                                         positions[indices[i + 1]],
                                         positions[indices[i + 2]]));
         }
      }
   }

   return triangles;
}
