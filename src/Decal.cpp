#include "Decal.h"

Decal::Decal(const Transform& modelTransform, const glm::vec3& normal, unsigned int decalIndex)
   : mModelTransform()
   , mModelMatrix()
   , mInverseModelMatrix()
   , mNormal(normal)
   , mPlaybackTime(0.0f)
   , mCircleModelTransforms({modelTransform, modelTransform, modelTransform, modelTransform})
   , mCircleModelMatrices()
   , mCircleInverseModelMatrices()
   , mCirclePlaybackTimes({0.0f, -0.1f, -0.2f, -0.3f})
   , mDecalIndex(decalIndex)
{

}

bool Decal::grow(const ScalarTrack& growAnimation)
{
   // Increase the playback times
   for (int i = 0; i < 4; ++i)
   {
      mCirclePlaybackTimes[i] += 0.025f;
   }

   // Once the smallest circle completes its animation we switch to the stable state
   if (mCirclePlaybackTimes[3] >= 1.0f)
   {
      for (int i = 0; i < 4; ++i)
      {
         float scalingFactor             = growAnimation.Sample(mCirclePlaybackTimes[i], false);
         mCircleModelTransforms[i].scale = glm::vec3(scalingFactor, scalingFactor, 1.0f);
         mCircleModelMatrices[i]         = transformToMat4(mCircleModelTransforms[i]);
         mCircleInverseModelMatrices[i]  = glm::inverse(mCircleModelMatrices[i]);
      }

      // Set the values that will be used in the stable state
      mModelTransform     = mCircleModelTransforms[0];
      mModelMatrix        = transformToMat4(mModelTransform);
      mInverseModelMatrix = glm::inverse(mModelMatrix);

      // Set the values that will be used in the shrinking state
      mCirclePlaybackTimes = {-0.3f, -0.2f, -0.1f, 0.0f};

      return true;
   }

   for (int i = 0; i < 4; ++i)
   {
      float scalingFactor             = growAnimation.Sample(mCirclePlaybackTimes[i], false);
      mCircleModelTransforms[i].scale = glm::vec3(scalingFactor, scalingFactor, 1.0f);
      mCircleModelMatrices[i]         = transformToMat4(mCircleModelTransforms[i]);
      mCircleInverseModelMatrices[i]  = glm::inverse(mCircleModelMatrices[i]);
   }

   return false;
}

bool Decal::shrink(const ScalarTrack& shrinkAnimation)
{
   // Increase the playback times
   for (int i = 0; i < 4; ++i)
   {
      mCirclePlaybackTimes[i] += 0.025f;
   }

   // Once the biggest circle completes its animation we switch to the stable state
   if (mCirclePlaybackTimes[0] >= 1.0f)
   {
      for (int i = 0; i < 4; ++i)
      {
         float scalingFactor             = shrinkAnimation.Sample(mCirclePlaybackTimes[i], false);
         mCircleModelTransforms[i].scale = glm::vec3(scalingFactor, scalingFactor, 1.0f);
         mCircleModelMatrices[i]         = transformToMat4(mCircleModelTransforms[i]);
         mCircleInverseModelMatrices[i]  = glm::inverse(mCircleModelMatrices[i]);
      }

      return true;
   }

   for (int i = 0; i < 4; ++i)
   {
      float scalingFactor             = shrinkAnimation.Sample(mCirclePlaybackTimes[i], false);
      mCircleModelTransforms[i].scale = glm::vec3(scalingFactor, scalingFactor, 1.0f);
      mCircleModelMatrices[i]         = transformToMat4(mCircleModelTransforms[i]);
      mCircleInverseModelMatrices[i]  = glm::inverse(mCircleModelMatrices[i]);
   }

   return false;
}
