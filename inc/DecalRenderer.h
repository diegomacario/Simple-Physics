#ifndef DECAL_RENDERER_H
#define DECAL_RENDERER_H

#include <list>
#include <deque>

#include "Shader.h"
#include "AnimatedMesh.h"
#include "Texture.h"
#include "Decal.h"

class DecalRenderer
{
public:

   DecalRenderer(unsigned int widthOfFramebuffer, unsigned int heightOfFramebuffer);
   ~DecalRenderer();

   DecalRenderer(const DecalRenderer&) = delete;
   DecalRenderer& operator=(const DecalRenderer&) = delete;

   DecalRenderer(DecalRenderer&& rhs) = delete;
   DecalRenderer& operator=(DecalRenderer&& rhs) = delete;

   void         bindDecalFBO();
   void         unbindDecalFBO();
   void         renderDecals(const glm::mat4& viewMatrix, const glm::mat4& perspectiveProjectionMatrix, bool displayDecalOBBs, bool displayDiscardedDecalParts);
   void         renderNormalTextureToFullScreenQuad();
   void         renderDepthTextureToFullScreenQuad();
   void         resizeTextures(unsigned int widthOfFramebuffer, unsigned int heightOfFramebuffer);
   void         addDecal(const glm::vec3& decalPosition, const glm::vec3& decalNormal);
   void         updateDecals(float playbackSpeed);
   void         reset();

   void         setMaxNumDecals(int maxNumDecals) { mMaxNumDecals = maxNumDecals; }
   void         setDecalScale(float scale);
   void         setNormalThreshold(float normalThreshold) { mNormalThreshold = glm::cos(glm::radians(normalThreshold)); }
   void         setDelayBetweenCircles(float delay) { mDelayBetweenCircles = delay; }
   void         setDecalBounce(float bounce);

   void         moveDecal(unsigned int decalIndex, bool wKeyIsPressed, bool aKeyIsPressed, bool sKeyIsPressed, bool dKeyIsPressed);

private:

   void         configureDecalFBO();

   unsigned int createColorTextureAttachment();
   unsigned int createDepthTextureAttachment();

   void         loadQuad();
   void         loadCube();
   void         composeGrowAnimation();
   void         composeShrinkAnimation();

   void         updateGrowingDecals(float playbackSpeed);
   void         updateStableDecals();
   void         updateShrinkingDecals(float playbackSpeed);

   void         renderAnimatedDecals(const std::deque<std::list<Decal>::iterator>& decals);
   void         renderStableDecals();

   glm::vec3    hexToColor(int hex);

   unsigned int                             mWidthOfFramebuffer;
   unsigned int                             mHeightOfFramebuffer;
   unsigned int                             mDecalFBO;
   unsigned int                             mNormalTexture;
   unsigned int                             mDepthTexture;

   float                                    mNormalThreshold;

   std::shared_ptr<Shader>                  mFullScreenQuadWithNormalTextureShader;
   std::shared_ptr<Shader>                  mFullScreenQuadWithDepthTextureShader;
   std::shared_ptr<Shader>                  mDecalShader;

   std::vector<AnimatedMesh>                mQuadMeshes;
   std::vector<AnimatedMesh>                mCubeMeshes;

   std::array<std::shared_ptr<Texture>, 20> mDecalTextures;
   std::array<std::shared_ptr<Texture>, 4>  mCircleTextures;
   std::array<std::array<glm::vec3, 4>, 20> mCircleColors;

   std::list<Decal>                         mDecals;
   std::deque<std::list<Decal>::iterator>   mGrowingDecals;
   std::deque<std::list<Decal>::iterator>   mStableDecals;
   std::deque<std::list<Decal>::iterator>   mShrinkingDecals;

   ScalarTrack                              mGrowAnimation;
   ScalarTrack                              mShrinkAnimation;

   unsigned int                             mDecalIndex;

   int                                      mMaxNumDecals;
   float                                    mDelayBetweenCircles;
};

#endif
