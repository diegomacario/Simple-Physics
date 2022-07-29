#include <glm/gtc/matrix_transform.hpp>

#include "RigidBody.h"

RigidBody::RigidBody(float            mass,
                     float            width,
                     float            height,
                     float            depth,
                     float            coefficientOfRestitution,
                     const glm::vec3& positionOfCM,
                     const glm::mat3& orientation,
                     const glm::vec3& velocityOfCM,
                     const glm::vec3& angularMomentum)
   : mOneOverMass(1.0f / mass)
   , mWidth(width)
   , mHeight(height)
   , mDepth(depth)
   , mCoefficientOfRestitution(coefficientOfRestitution)
   , mStates({KinematicAndDynamicState(positionOfCM, orientation, velocityOfCM, angularMomentum), KinematicAndDynamicState()})
{
   float halfWidth  = mWidth * 0.5f;
   float halfHeight = mHeight * 0.5f;
   float halfDepth  = mDepth * 0.5f;

   float halfWidthSquared  = halfWidth * halfWidth;
   float halfHeightSquared = halfHeight * halfHeight;
   float halfDepthSquared  = halfDepth * halfDepth;

   mInverseInertiaTensorInLocalSpace[0][0] = (3.0f * mOneOverMass) / (halfHeightSquared + halfDepthSquared);
   mInverseInertiaTensorInLocalSpace[1][1] = (3.0f * mOneOverMass) / (halfWidthSquared + halfDepthSquared);
   mInverseInertiaTensorInLocalSpace[2][2] = (3.0f * mOneOverMass) / (halfWidthSquared + halfHeightSquared);

   mVerticesInLocalSpace[0] = glm::vec3( halfWidth,  halfHeight,  halfDepth);
   mVerticesInLocalSpace[1] = glm::vec3( halfWidth,  halfHeight, -halfDepth);
   mVerticesInLocalSpace[2] = glm::vec3( halfWidth, -halfHeight,  halfDepth);
   mVerticesInLocalSpace[3] = glm::vec3( halfWidth, -halfHeight, -halfDepth);
   mVerticesInLocalSpace[4] = glm::vec3(-halfWidth,  halfHeight,  halfDepth);
   mVerticesInLocalSpace[5] = glm::vec3(-halfWidth,  halfHeight, -halfDepth);
   mVerticesInLocalSpace[6] = glm::vec3(-halfWidth, -halfHeight,  halfDepth);
   mVerticesInLocalSpace[7] = glm::vec3(-halfWidth, -halfHeight, -halfDepth);

   calculateWorldSpaceVertices(current);
}

void RigidBody::calculateWorldSpaceVertices(RigidBodyState state)
{
   glm::vec3& translation = mStates[state].positionOfCM;
   glm::mat3& rotation = mStates[state].orientation;
   std::array<glm::vec3, 8> worldSpaceVertices = mStates[state].verticesInWorldSpace;

   for (int i = 0; i < 8; ++i)
   {
      worldSpaceVertices[i] = translation + (rotation * mVerticesInLocalSpace[i]);
   }
}

glm::mat4 RigidBody::getModelMatrix(RigidBodyState state) const
{
   // 3) Translate the cube
   glm::mat4 modelMatrix = glm::translate(glm::mat4(1.0f), mStates[state].positionOfCM);

   // 2) Rotate the cube
   modelMatrix = modelMatrix * glm::mat4(mStates[state].orientation);

   // 1) Scale the cube
   modelMatrix = glm::scale(modelMatrix, glm::vec3(mWidth, mHeight, mDepth));

   return modelMatrix;
}

RigidBody::KinematicAndDynamicState::KinematicAndDynamicState()
   : positionOfCM()
   , orientation()
   , velocityOfCM()
   , angularMomentum()
   , forceOfCM()
   , torque()
   , inverseInertiaTensorInWorldSpace()
   , angularVelocity()
   , verticesInWorldSpace()
{

}

RigidBody::KinematicAndDynamicState::KinematicAndDynamicState(const glm::vec3& positionOfCenterOfMass,
                                                              const glm::mat3& orientation,
                                                              const glm::vec3& velocityOfCenterOfMass,
                                                              const glm::vec3& angularMomentum)
   : positionOfCM(positionOfCenterOfMass)
   , orientation(orientation)
   , velocityOfCM(velocityOfCenterOfMass)
   , angularMomentum(angularMomentum)
   , forceOfCM()
   , torque()
   , inverseInertiaTensorInWorldSpace()
   , angularVelocity()
   , verticesInWorldSpace()
{

}
