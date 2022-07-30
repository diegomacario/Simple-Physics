#ifndef RIGID_BODY_H
#define RIGID_BODY_H

#include <glm/glm.hpp>

#include <array>

enum RigidBodyState
{
   current = 0,
   future  = 1,
};

class RigidBody
{
public:

   RigidBody(float            mass,
             float            width,
             float            height,
             float            depth,
             float            coefficientOfRestitution,
             const glm::vec3& positionOfCM,
             const glm::mat3& orientation,
             const glm::vec3& velocityOfCM,
             const glm::vec3& angularMomentum);
   ~RigidBody() = default;

   struct KinematicAndDynamicState
   {
      KinematicAndDynamicState();
      KinematicAndDynamicState(const glm::vec3& positionOfCM,
                               const glm::mat3& orientation,
                               const glm::vec3& velocityOfCM,
                               const glm::vec3& angularMomentum);

      glm::vec3                positionOfCM;
      glm::mat3                orientation;

      glm::vec3                velocityOfCM;
      glm::vec3                angularMomentum;

      glm::vec3                forceOfCM;
      glm::vec3                torque;

      glm::mat3                inverseInertiaTensorInWorldSpace;
      glm::vec3                angularVelocity;

      std::array<glm::vec3, 8> verticesInWorldSpace;
   };

   void                      calculateWorldSpaceVertices(RigidBodyState state);
   glm::mat4                 getModelMatrix(RigidBodyState state) const;
   KinematicAndDynamicState& getState(RigidBodyState state) { return mStates[state]; }
   float                     getOneOverMass() const { return mOneOverMass; }
   float                     getCoefficientOfRestitution() const { return mCoefficientOfRestitution; }
   glm::mat3                 getInverseInertiaTensorInLocalSpace() const { return mInverseInertiaTensorInLocalSpace; }
   void                      swapStates();

private:

   float                                   mOneOverMass;
   float                                   mWidth;
   float                                   mHeight;
   float                                   mDepth;
   float                                   mCoefficientOfRestitution;
   glm::mat3                               mInverseInertiaTensorInLocalSpace;
   std::array<glm::vec3, 8>                mVerticesInLocalSpace;

   std::array<KinematicAndDynamicState, 2> mStates;
};

#endif
