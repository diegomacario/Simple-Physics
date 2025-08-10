#ifndef PLAY_STATE_H
#define PLAY_STATE_H

#include "Window.h"
#include "Shader.h"
#include "Camera3.h"
#include "AnimatedMesh.h"
#include "DecalRenderer.h"

class PlayState
{
public:
   PlayState(const std::shared_ptr<Window> &window);
   ~PlayState() = default;

   PlayState(const PlayState &) = delete;
   PlayState &operator=(const PlayState &) = delete;

   PlayState(PlayState &&) = delete;
   PlayState &operator=(PlayState &&) = delete;

   void update(float deltaTime);
   void render();

private:
   void loadModels();

   void userInterface();

   void renderWorld();
   void renderNormalsAndDepth();

   std::shared_ptr<Window> mWindow;

   Camera3 mCamera3;

   std::shared_ptr<Shader> mGouradShader;
   std::shared_ptr<Shader> mNormalAndDepthShader;

   std::vector<AnimatedMesh> mInvertedIcosphereMeshes;
   std::vector<AnimatedMesh> mNormalInvertedIcosphereMeshes;

   std::shared_ptr<DecalRenderer> mDecalRenderer;

   float mPlaybackSpeed = 1.0f;
   int mDisplayMode = 0;
   int mMaxNumDecals = 100;
   float mCurrentDecalScale = 1.0f;
   float mSelectedDecalScale = 1.0f;
   float mDecalNormalThreshold = 89.0f;
   float mCurrentDelayBetweenCircles = 0.1f;
   float mSelectedDelayBetweenCircles = 0.1f;
   float mCurrentDecalBounce = 4.5f;
   float mSelectedDecalBounce = 4.5f;
   bool mDisplayDecalOBBs = false;
   bool mDisplayDiscardedDecalParts = false;
};

#endif
