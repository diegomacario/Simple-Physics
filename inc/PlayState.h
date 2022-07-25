#ifndef PLAY_STATE_H
#define PLAY_STATE_H

#include "State.h"
#include "FiniteStateMachine.h"
#include "Window.h"
#include "Shader.h"
#include "Camera3.h"
#include "AnimatedMesh.h"
#include "Texture.h"

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

   void loadGround();

   void configureLights(const std::shared_ptr<Shader>& shader);

   void userInterface();

   void resetScene();

   void resetCamera();

   std::shared_ptr<FiniteStateMachine>    mFSM;

   std::shared_ptr<Window>                mWindow;

   Camera3                                mCamera3;

   std::vector<AnimatedMesh>              mGroundMeshes;
   std::shared_ptr<Texture>               mGroundTexture;
   std::shared_ptr<Shader>                mGroundShader;
};

#endif
