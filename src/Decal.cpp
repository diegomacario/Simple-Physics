#include "Decal.h"

Decal::Decal(const Transform& modelTransform, const glm::vec3& normal)
   : mModelTransform()
   , mModelMatrix()
   , mInverseModelMatrix()
   , mNormal(normal)
   , mPlaybackTime(0.0f)
   , mGrowingModelTransforms({modelTransform, modelTransform, modelTransform, modelTransform})
   , mGrowingModelMatrices()
   , mGrowingInverseModelMatrices()
   , mGrowingPlaybackTimes({0.0f, -0.1f, -0.2f, -0.3f})
{

}

bool Decal::grow(const ScalarTrack& growAnimation)
{
   for (int i = 0; i < 4; ++i)
   {
      mGrowingPlaybackTimes[i] += 0.025f;
   }

   if (mGrowingPlaybackTimes[3] >= 1.0f)
   {
      for (int i = 0; i < 4; ++i)
      {
         float scalingFactor              = growAnimation.Sample(mGrowingPlaybackTimes[i], false);
         mGrowingModelTransforms[i].scale = glm::vec3(scalingFactor, scalingFactor, 1.0f);
         mGrowingModelMatrices[i]         = transformToMat4(mGrowingModelTransforms[i]);
         mGrowingInverseModelMatrices[i]  = glm::inverse(mGrowingModelMatrices[i]);
      }

      mModelTransform     = mGrowingModelTransforms[0];
      mModelMatrix        = transformToMat4(mModelTransform);
      mInverseModelMatrix = glm::inverse(mModelMatrix);

      return true;
   }

   for (int i = 0; i < 4; ++i)
   {
      float scalingFactor              = growAnimation.Sample(mGrowingPlaybackTimes[i], false);
      mGrowingModelTransforms[i].scale = glm::vec3(scalingFactor, scalingFactor, 1.0f);
      mGrowingModelMatrices[i]         = transformToMat4(mGrowingModelTransforms[i]);
      mGrowingInverseModelMatrices[i]  = glm::inverse(mGrowingModelMatrices[i]);
   }

   return false;
}

bool Decal::shrink(const ScalarTrack& shrinkAnimation)
{
   mPlaybackTime += 0.025f;

   if (mPlaybackTime >= 1.0f)
   {
      float scalingFactor   = shrinkAnimation.Sample(mPlaybackTime, false);
      mModelTransform.scale = glm::vec3(scalingFactor, scalingFactor, 1.0f);
      mModelMatrix          = transformToMat4(mModelTransform);
      mInverseModelMatrix   = glm::inverse(mModelMatrix);

      return true;
   }

   float scalingFactor   = shrinkAnimation.Sample(mPlaybackTime, false);
   mModelTransform.scale = glm::vec3(scalingFactor, scalingFactor, 1.0f);
   mModelMatrix          = transformToMat4(mModelTransform);
   mInverseModelMatrix   = glm::inverse(mModelMatrix);

   return false;
}
