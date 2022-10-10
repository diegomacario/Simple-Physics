#ifndef PLAY_STATE_H
#define PLAY_STATE_H

#include "State.h"
#include "FiniteStateMachine.h"
#include "Window.h"
#include "Shader.h"
#include "Camera3.h"
#include "AnimatedMesh.h"
#include "Texture.h"
#include "DecalRenderer.h"
#include "World.h"

class PlayState : public State
{
public:

   PlayState(const std::shared_ptr<FiniteStateMachine>& finiteStateMachine,
             const std::shared_ptr<Window>&             window);
   ~PlayState() = default;

   PlayState(const PlayState&) = delete;
   PlayState& operator=(const PlayState&) = delete;

   PlayState(PlayState&&) = delete;
   PlayState& operator=(PlayState&&) = delete;

   void initializeState();

   void enter() override;
   void processInput() override;
   void update(float deltaTime) override;
   void render() override;
   void exit() override;

private:

   void loadModels();

   void userInterface();

   void resetCamera();

   void renderRigidBodies();
   void renderWorld();
   void renderNormalsAndDepth();

   std::shared_ptr<FiniteStateMachine>    mFSM;

   std::shared_ptr<Window>                mWindow;

   Camera3                                mCamera3;

   std::shared_ptr<Shader>                mDiffuseShader;
   std::shared_ptr<Shader>                mGouradShader;
   std::shared_ptr<Shader>                mNormalAndDepthShader;

   std::vector<AnimatedMesh>              mCubeMeshes;
   std::shared_ptr<Texture>               mCubeTexture;
   std::vector<AnimatedMesh>              mPlaneMeshes;
   std::shared_ptr<Texture>               mPlaneTexture;
   std::vector<AnimatedMesh>              mInvertedCubeMeshes;
   std::shared_ptr<Texture>               mInvertedCubeTexture;
   std::vector<AnimatedMesh>              mNormalCubeMeshes;
   std::vector<AnimatedMesh>              mNormalPlaneMeshes;
   std::vector<AnimatedMesh>              mNormalInvertedCubeMeshes;

   std::shared_ptr<DecalRenderer>         mDecalRenderer;

   World                                  mWorld;

   float                                  mPlaybackSpeed = 1.0f;
   int                                    mDisplayMode = 0;
   int                                    mMaxNumDecals = 100;
   float                                  mCurrentDecalScale = 1.0f;
   float                                  mSelectedDecalScale = 1.0f;
   float                                  mDecalNormalThreshold = 89.0f;
   float                                  mCurrentDelayBetweenCircles = 0.1f;
   float                                  mSelectedDelayBetweenCircles = 0.1f;
   float                                  mCurrentDecalBounce = 4.5f;
   float                                  mSelectedDecalBounce = 4.5f;
   bool                                   mDisplayDecalOBBs = false;
   bool                                   mDisplayDiscardedDecalParts = false;
   bool                                   mGravity = false;
   int                                    mVelocityChange = 0; // 0 = No change, 1 = Increase, 2 = Decrease
};

#endif
