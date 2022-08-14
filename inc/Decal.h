#ifndef DECAL_H
#define DECAL_H

#include "Transform.h"
#include "Track.h"

class Decal
{
public:

   Decal(const Transform& modelTransform, const glm::vec3& normal);
   ~Decal() = default;

   bool             grow(const ScalarTrack& growAnimation);
   bool             shrink(const ScalarTrack& shrinkAnimation);

   const glm::mat4& getModelMatrix() const { return mModelMatrix; }
   const glm::mat4& getInverseModelMatrix() const { return mInverseModelMatrix; }
   const glm::vec3& getNormal() const { return mNormal; }

private:

   Transform mModelTransform;
   glm::mat4 mModelMatrix;
   glm::mat4 mInverseModelMatrix;
   glm::vec3 mNormal;
   float     mPlaybackTime;
};

#endif
