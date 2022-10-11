#include "Decal.h"

Decal::Decal(const Transform& modelTransform, const glm::vec3& normal, unsigned int decalIndex, float delayBetweenCircles)
   : mModelTransform()
   , mModelMatrix()
   , mInverseModelMatrix()
   , mNormal(normal)
   , mDecalDepthScalingFactor(2.0f)
   , mDelayBetweenCircles(delayBetweenCircles)
   , mCircleModelTransforms({modelTransform, modelTransform, modelTransform, modelTransform})
   , mCircleModelMatrices()
   , mCircleInverseModelMatrices()
   , mCirclePlaybackTimes({0.0f, -delayBetweenCircles, -delayBetweenCircles * 2.0f, -delayBetweenCircles * 3.0f})
   , mDecalIndex(decalIndex)
{

}

bool Decal::grow(const ScalarTrack& growAnimation, float playbackSpeed)
{
   // Increase the playback times
   for (int i = 0; i < 4; ++i)
   {
      mCirclePlaybackTimes[i] += 0.025f * playbackSpeed;
   }

   // Once the smallest circle completes its animation we switch to the stable state
   if (mCirclePlaybackTimes[3] >= 1.0f)
   {
      for (int i = 0; i < 4; ++i)
      {
         float scalingFactor             = growAnimation.Sample(mCirclePlaybackTimes[i], false);
         mCircleModelTransforms[i].scale = glm::vec3(scalingFactor, scalingFactor, mDecalDepthScalingFactor);
         mCircleModelMatrices[i]         = transformToMat4(mCircleModelTransforms[i]);
         mCircleInverseModelMatrices[i]  = glm::inverse(mCircleModelMatrices[i]);
      }

      // Set the values that will be used in the stable state
      mModelTransform     = mCircleModelTransforms[0];
      mModelMatrix        = transformToMat4(mModelTransform);
      mInverseModelMatrix = glm::inverse(mModelMatrix);

      // Set the values that will be used in the shrinking state
      mCirclePlaybackTimes = {-mDelayBetweenCircles * 3.0f, -mDelayBetweenCircles * 2.0f, -mDelayBetweenCircles, 0.0f};

      return true;
   }

   for (int i = 0; i < 4; ++i)
   {
      float scalingFactor             = growAnimation.Sample(mCirclePlaybackTimes[i], false);
      mCircleModelTransforms[i].scale = glm::vec3(scalingFactor, scalingFactor, mDecalDepthScalingFactor);
      mCircleModelMatrices[i]         = transformToMat4(mCircleModelTransforms[i]);
      mCircleInverseModelMatrices[i]  = glm::inverse(mCircleModelMatrices[i]);
   }

   return false;
}

bool Decal::shrink(const ScalarTrack& shrinkAnimation, float playbackSpeed)
{
   // Increase the playback times
   for (int i = 0; i < 4; ++i)
   {
      mCirclePlaybackTimes[i] += 0.025f * playbackSpeed;
   }

   // Once the biggest circle completes its animation we switch to the stable state
   if (mCirclePlaybackTimes[0] >= 1.0f)
   {
      for (int i = 0; i < 4; ++i)
      {
         float scalingFactor             = shrinkAnimation.Sample(mCirclePlaybackTimes[i], false);
         mCircleModelTransforms[i].scale = glm::vec3(scalingFactor, scalingFactor, mDecalDepthScalingFactor);
         mCircleModelMatrices[i]         = transformToMat4(mCircleModelTransforms[i]);
         mCircleInverseModelMatrices[i]  = glm::inverse(mCircleModelMatrices[i]);
      }

      return true;
   }

   for (int i = 0; i < 4; ++i)
   {
      float scalingFactor             = shrinkAnimation.Sample(mCirclePlaybackTimes[i], false);
      mCircleModelTransforms[i].scale = glm::vec3(scalingFactor, scalingFactor, mDecalDepthScalingFactor);
      mCircleModelMatrices[i]         = transformToMat4(mCircleModelTransforms[i]);
      mCircleInverseModelMatrices[i]  = glm::inverse(mCircleModelMatrices[i]);
   }

   return false;
}

void Decal::updateScale(float scale)
{
   mModelTransform.scale = glm::vec3(scale, scale, mDecalDepthScalingFactor);
   mModelMatrix          = transformToMat4(mModelTransform);
   mInverseModelMatrix   = glm::inverse(mModelMatrix);
}

void Decal::move(bool wKeyIsPressed, bool aKeyIsPressed, bool sKeyIsPressed, bool dKeyIsPressed)
{
   float horizontalInput = 0.0f;
   float verticalInput   = 0.0f;

   if (wKeyIsPressed)
   {
      verticalInput -= 0.05f;
   }
   else if (sKeyIsPressed)
   {
      verticalInput += 0.05f;
   }

   if (aKeyIsPressed)
   {
      horizontalInput -= 0.05f;
   }
   else if (dKeyIsPressed)
   {
      horizontalInput += 0.05f;
   }

   mModelTransform.position += glm::vec3(horizontalInput, 0.0f, verticalInput);
   mModelMatrix              = transformToMat4(mModelTransform);
   mInverseModelMatrix       = glm::inverse(mModelMatrix);

   for (int i = 0; i < 4; ++i)
   {
      mCircleModelTransforms[i].position += glm::vec3(horizontalInput, 0.0f, verticalInput);
      mCircleModelMatrices[i]             = transformToMat4(mCircleModelTransforms[i]);
      mCircleInverseModelMatrices[i]      = glm::inverse(mCircleModelMatrices[i]);
   }
}
