#ifdef __EMSCRIPTEN__
#include <emscripten/html5.h>
#endif

#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"

#include "ResourceManager.h"
#include "ShaderLoader.h"
#include "TextureLoader.h"
#include "GLTFLoader.h"
#include "PlayState.h"

PlayState::PlayState(const std::shared_ptr<Window> &window)
    : mWindow(window), mCamera3(4.5f, 0.0f, glm::vec3(0.0f), Q::quat(), glm::vec3(0.0f, 0.0f, 0.0f), 1.0f, 10.0f, -90.0f, 90.0f, 45.0f, 1280.0f / 720.0f, 0.1f, 130.0f, 0.25f), mDecalRenderer(std::make_shared<DecalRenderer>(window->getWidthOfFramebufferInPix(), window->getHeightOfFramebufferInPix()))
{
}

void PlayState::update(float deltaTime)
{
   mDecalRenderer->updateDecals(mPlaybackSpeed);
}

void PlayState::render()
{
   // Render the depth and normal textures
   mDecalRenderer->bindDecalFBO();
   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
   renderNormalsAndDepth();
   mDecalRenderer->unbindDecalFBO();

#ifndef __EMSCRIPTEN__
   mWindow->bindMultisampleFramebuffer();
#endif
   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

   // Enable depth testing for 3D objects
   glEnable(GL_DEPTH_TEST);
   glClear(GL_DEPTH_BUFFER_BIT);

   renderWorld();
   glDisable(GL_DEPTH_TEST);
   glEnable(GL_BLEND);
   mDecalRenderer->renderDecals(mCamera3.getViewMatrix(), mCamera3.getPerspectiveProjectionMatrix(), mDisplayDecalOBBs, mDisplayDiscardedDecalParts);
   glDisable(GL_BLEND);
   glEnable(GL_DEPTH_TEST);
}
