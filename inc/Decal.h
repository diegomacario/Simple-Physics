#ifndef DECAL_H
#define DECAL_H

#include <array>

#include "Transform.h"
#include "Track.h"

class Decal
{
public:

   Decal(const Transform& modelTransform, const glm::vec3& normal, unsigned int decalIndex, float delayBetweenCircles);
   ~Decal() = default;

   bool                            grow(const ScalarTrack& growAnimation, float playbackSpeed);
   bool                            shrink(const ScalarTrack& shrinkAnimation, float playbackSpeed);

   const glm::mat4&                getModelMatrix() const { return mModelMatrix; }
   const glm::mat4&                getInverseModelMatrix() const { return mInverseModelMatrix; }
   const glm::vec3&                getNormal() const { return mNormal; }

   const std::array<glm::mat4, 4>& getCircleModelMatrices() const { return mCircleModelMatrices; }
   const std::array<glm::mat4, 4>& getCircleInverseModelMatrices() const { return mCircleInverseModelMatrices; }

   unsigned int                    getDecalIndex() const { return mDecalIndex; }

   void                            updateScale(float scale);

   void                            move(bool wKeyIsPressed, bool aKeyIsPressed, bool sKeyIsPressed, bool dKeyIsPressed);

private:

   Transform                mModelTransform;
   glm::mat4                mModelMatrix;
   glm::mat4                mInverseModelMatrix;
   glm::vec3                mNormal;
   float                    mDecalDepthScalingFactor;
   float                    mDelayBetweenCircles;

   std::array<Transform, 4> mCircleModelTransforms;
   std::array<glm::mat4, 4> mCircleModelMatrices;
   std::array<glm::mat4, 4> mCircleInverseModelMatrices;
   std::array<float, 4>     mCirclePlaybackTimes;

   unsigned int             mDecalIndex;
};

#endif
