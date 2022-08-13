#include "Decal.h"

Decal::Decal(const Transform& modelTransform, const glm::vec3& normal)
   : mModelTransform(modelTransform)
   , mModelMatrix()
   , mInverseModelMatrix()
   , mNormal(normal)
   , mPlaybackTime(0.0f)
{

}

bool Decal::grow(const ScalarTrack& growAnimation)
{
   bool finished = false;

   mPlaybackTime += 0.025f;
   if (mPlaybackTime >= 1.0f)
   {
      mPlaybackTime = 1.0f;
      finished = true;
   }

   float scalingFactor = growAnimation.Sample(mPlaybackTime, false);

   mModelTransform.scale = glm::vec3(scalingFactor, scalingFactor, 1.0f);
   mModelMatrix = transformToMat4(mModelTransform);
   mInverseModelMatrix = glm::inverse(mModelMatrix);

   return finished;
}

bool Decal::updateLifetime()
{
   mPlaybackTime += 0.025f;
   if (mPlaybackTime >= 5.0f)
   {
      return true;
   }

   return false;
}
