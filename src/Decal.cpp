#include "Decal.h"

Decal::Decal(const Transform& modelTransform, const glm::vec3& normal)
   : mModelTransform(modelTransform)
   , mModelMatrix()
   , mInverseModelMatrix()
   , mNormal(normal)
   , mPlaybackTime(0.0f)
{

}

void Decal::update(const ScalarTrack& growAnimation)
{
   mPlaybackTime += 0.025f;
   if (mPlaybackTime >= 1.0f)
   {
      mPlaybackTime = 1.0f;
   }

   float scalingFactor = growAnimation.Sample(mPlaybackTime, false);

   mModelTransform.scale = glm::vec3(scalingFactor, scalingFactor, 1.0f);
   mModelMatrix = transformToMat4(mModelTransform);
   mInverseModelMatrix = glm::inverse(mModelMatrix);
}
