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

PlayState::PlayState(const std::shared_ptr<FiniteStateMachine>& finiteStateMachine,
                     const std::shared_ptr<Window>&             window)
   : mFSM(finiteStateMachine)
   , mWindow(window)
   , mCamera3(4.5f, 35.0f, glm::vec3(0.0f), Q::quat(), glm::vec3(0.0f, 0.0f, 0.0f), 1.0f, 10.0f, -90.0f, 90.0f, 45.0f, 1280.0f / 720.0f, 0.1f, 130.0f, 0.25f)
   , mDecalRenderer(std::make_shared<DecalRenderer>(window->getWidthOfFramebufferInPix(), window->getHeightOfFramebufferInPix()))
   , mPlaneModelTransform(glm::vec3(0.0f), Q::quat(), glm::vec3(3.0f))
{
   // Initialize the diffuse shader
   mDiffuseShader = ResourceManager<Shader>().loadUnmanagedResource<ShaderLoader>("resources/shaders/diffuse.vert",
                                                                                  "resources/shaders/diffuse.frag");

   // Initialize the Gourad shader
   mGouradShader = ResourceManager<Shader>().loadUnmanagedResource<ShaderLoader>("resources/shaders/gourad.vert",
                                                                                 "resources/shaders/gourad.frag");

   // Initialize the normal and depth shader
   mNormalAndDepthShader = ResourceManager<Shader>().loadUnmanagedResource<ShaderLoader>("resources/shaders/normal_and_depth.vert",
                                                                                         "resources/shaders/normal_and_depth.frag");

   loadModels();

   mWindow->setDecalRenderer(mDecalRenderer);

   mDecalRenderer->addDecal(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
   mDecalRenderer->addDecal(glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
   mDecalRenderer->addDecal(glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
   mDecalRenderer->addDecal(glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, 1.0f, 0.0f));
   mDecalRenderer->addDecal(glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, 1.0f, 0.0f));
}

void PlayState::initializeState()
{

}

void PlayState::enter()
{
   initializeState();
   resetCamera();
}

void PlayState::processInput()
{
   // Close the game
   if (mWindow->keyIsPressed(GLFW_KEY_ESCAPE)) { mWindow->setShouldClose(true); }

#ifndef __EMSCRIPTEN__
   // Make the game full screen or windowed
   if (mWindow->keyIsPressed(GLFW_KEY_F) && !mWindow->keyHasBeenProcessed(GLFW_KEY_F))
   {
      mWindow->setKeyAsProcessed(GLFW_KEY_F);
      mWindow->setFullScreen(!mWindow->isFullScreen());
   }

   // Change the number of samples used for anti aliasing
   if (mWindow->keyIsPressed(GLFW_KEY_1) && !mWindow->keyHasBeenProcessed(GLFW_KEY_1))
   {
      mWindow->setKeyAsProcessed(GLFW_KEY_1);
      mWindow->setNumberOfSamples(1);
   }
   else if (mWindow->keyIsPressed(GLFW_KEY_2) && !mWindow->keyHasBeenProcessed(GLFW_KEY_2))
   {
      mWindow->setKeyAsProcessed(GLFW_KEY_2);
      mWindow->setNumberOfSamples(2);
   }
   else if (mWindow->keyIsPressed(GLFW_KEY_4) && !mWindow->keyHasBeenProcessed(GLFW_KEY_4))
   {
      mWindow->setKeyAsProcessed(GLFW_KEY_4);
      mWindow->setNumberOfSamples(4);
   }
   else if (mWindow->keyIsPressed(GLFW_KEY_8) && !mWindow->keyHasBeenProcessed(GLFW_KEY_8))
   {
      mWindow->setKeyAsProcessed(GLFW_KEY_8);
      mWindow->setNumberOfSamples(8);
   }
#endif

   // Reset the camera
   if (mWindow->keyIsPressed(GLFW_KEY_R)) { resetCamera(); }

   // Orient the camera
   if (mWindow->mouseMoved() && mWindow->isMouseButtonPressed(GLFW_MOUSE_BUTTON_LEFT))
   {
      mCamera3.processMouseMovement(mWindow->getCursorXOffset(), mWindow->getCursorYOffset());
      mWindow->resetMouseMoved();
   }

   // Adjust the distance between the player and the camera
   if (mWindow->scrollWheelMoved())
   {
      mCamera3.processScrollWheelMovement(mWindow->getScrollYOffset());
      mWindow->resetScrollWheelMoved();
   }
}

void PlayState::update(float deltaTime)
{
   mDecalRenderer->setMaxNumDecals(mMaxNumDecals);

   if (mSelectedDecalScale != mCurrentDecalScale)
   {
      mDecalRenderer->setDecalScale(mSelectedDecalScale);
      mCurrentDecalScale = mSelectedDecalScale;
   }

   if (mSelectedDelayBetweenCircles != mCurrentDelayBetweenCircles)
   {
      mDecalRenderer->setDelayBetweenCircles(mSelectedDelayBetweenCircles);
      mCurrentDelayBetweenCircles = mSelectedDelayBetweenCircles;
   }

   if (mSelectedDecalBounce != mCurrentDecalBounce)
   {
      mDecalRenderer->setDecalBounce(mSelectedDecalBounce);
      mCurrentDecalBounce = mSelectedDecalBounce;
   }

   mDecalRenderer->setNormalThreshold(mDecalNormalThreshold);

   mDecalRenderer->updateDecals(mPlaybackSpeed);
}

void PlayState::render()
{
   ImGui_ImplOpenGL3_NewFrame();
   ImGui_ImplGlfw_NewFrame();
   ImGui::NewFrame();

   userInterface();

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

   if (mDisplayMode == 0) // Final
   {
      renderWorld();
      glDisable(GL_DEPTH_TEST);
      glEnable(GL_BLEND);
      mDecalRenderer->renderDecals(mCamera3.getViewMatrix(), mCamera3.getPerspectiveProjectionMatrix(), mDisplayDecalOBBs, mDisplayDiscardedDecalParts);
      glDisable(GL_BLEND);
      glEnable(GL_DEPTH_TEST);
   }
   else if (mDisplayMode == 1) // Depth
   {
      mDecalRenderer->renderDepthTextureToFullScreenQuad();
   }
   else if (mDisplayMode == 2) // Normal
   {
      mDecalRenderer->renderNormalTextureToFullScreenQuad();
   }

   ImGui::Render();
   ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

#ifndef __EMSCRIPTEN__
   mWindow->generateAntiAliasedImage();
#endif

   mWindow->swapBuffers();
   mWindow->pollEvents();
}

void PlayState::exit()
{

}

void PlayState::loadModels()
{
   // Load the texture of the plane
   mPlaneTexture = ResourceManager<Texture>().loadUnmanagedResource<TextureLoader>("resources/models/plane/plane.png");

   // Load the plane
   cgltf_data* data = LoadGLTFFile("resources/models/plane/world_plane.glb");
   mPlaneMeshes = LoadStaticMeshes(data);
   FreeGLTFFile(data);

   // Load the normal plane
   data = LoadGLTFFile("resources/models/plane/world_plane.glb");
   mNormalPlaneMeshes = LoadStaticMeshes(data);
   FreeGLTFFile(data);

   int positionsAttribLoc = mDiffuseShader->getAttributeLocation("position");
   int normalsAttribLoc   = mDiffuseShader->getAttributeLocation("normal");
   int texCoordsAttribLoc = mDiffuseShader->getAttributeLocation("texCoord");

   for (unsigned int i = 0,
        size = static_cast<unsigned int>(mPlaneMeshes.size());
        i < size;
        ++i)
   {
      mPlaneMeshes[i].ConfigureVAO(positionsAttribLoc,
                                   normalsAttribLoc,
                                   texCoordsAttribLoc);
   }

   positionsAttribLoc = mGouradShader->getAttributeLocation("position");
   normalsAttribLoc   = mGouradShader->getAttributeLocation("normal");
   texCoordsAttribLoc = mGouradShader->getAttributeLocation("texCoord");

   mGouradShader->use(true);
   mGouradShader->setUniformVec3( "light.worldPos",  glm::vec3(0.0, 0.0f, 1.0f));
   mGouradShader->setUniformVec3( "light.color",     glm::vec3(1.0f));
   mGouradShader->setUniformFloat("light.linearAtt", 0.0f);
   mGouradShader->use(false);

   positionsAttribLoc = mNormalAndDepthShader->getAttributeLocation("position");
   normalsAttribLoc   = mNormalAndDepthShader->getAttributeLocation("normal");
   texCoordsAttribLoc = mNormalAndDepthShader->getAttributeLocation("texCoord");

   for (unsigned int i = 0,
        size = static_cast<unsigned int>(mNormalPlaneMeshes.size());
        i < size;
        ++i)
   {
      mNormalPlaneMeshes[i].ConfigureVAO(positionsAttribLoc,
                                         normalsAttribLoc,
                                         texCoordsAttribLoc);
   }
}

void PlayState::userInterface()
{
   ImGui::SetNextWindowPos(ImVec2(10.0f, 10.0f), ImGuiCond_Appearing);

   char title[64];
   snprintf(title, 32, "Simple Physics (%.1f FPS)###SimplePhysics", ImGui::GetIO().Framerate);
   ImGui::Begin(title, nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize);

   if (ImGui::CollapsingHeader("Controls", nullptr, ImGuiTreeNodeFlags_DefaultOpen))
   {
      ImGui::BulletText("Hold the left mouse button and move the mouse\n"
                        "to rotate the camera around the scene.");
      ImGui::BulletText("Use the scroll wheel to zoom in and out.");
      ImGui::BulletText("Press the R key to reset the camera.");
   }

   if (ImGui::CollapsingHeader("Settings", nullptr, ImGuiTreeNodeFlags_DefaultOpen))
   {
      ImGui::RadioButton("Display final scene", &mDisplayMode, 0);
      ImGui::RadioButton("Display depth texture", &mDisplayMode, 1);
      ImGui::RadioButton("Display normal texture", &mDisplayMode, 2);

      ImGui::SliderFloat("Playback speed", &mPlaybackSpeed, 0.0f, 1.0f, "%.3f");

      if (ImGui::InputInt("Max num decals", &mMaxNumDecals, 1, 1, ImGuiInputTextFlags_EnterReturnsTrue))
      {
         if (mMaxNumDecals < 1)
         {
            mMaxNumDecals = 1;
         }
         else if (mMaxNumDecals > 1000)
         {
            mMaxNumDecals = 1000;
         }
      }

      ImGui::SliderFloat("Decal scale", &mSelectedDecalScale, 0.1f, 3.0f, "%.3f");

      ImGui::SliderFloat("Animation delay", &mSelectedDelayBetweenCircles, 0.0f, 1.0f, "%.3f");

      ImGui::SliderFloat("Animation bounce", &mSelectedDecalBounce, 0.0f, 10.0f, "%.3f");

#ifndef __EMSCRIPTEN__
      ImGui::SliderFloat("Decal normal threshold", &mDecalNormalThreshold, 0.0f, 180.0f, "%.3f");
#endif

      ImGui::Checkbox("Display decal OBBs", &mDisplayDecalOBBs);

#ifndef __EMSCRIPTEN__
      ImGui::Checkbox("Display discarded decal parts", &mDisplayDiscardedDecalParts);
#endif
   }

   ImGui::End();
}

void PlayState::resetCamera()
{
   mCamera3.reposition(4.5f, 35.0f, glm::vec3(0.0f), Q::quat(), glm::vec3(0.0f, 0.0f, 0.0f), 1.0f, 10.0f, -90.0f, 90.0f);
   mCamera3.processMouseMovement(180.0f / 0.25f, 0.0f);
}

void PlayState::renderWorld()
{
   mDiffuseShader->use(true);
   mDiffuseShader->setUniformMat4("model",      transformToMat4(mPlaneModelTransform));
   mDiffuseShader->setUniformMat4("view",       mCamera3.getViewMatrix());
   mDiffuseShader->setUniformMat4("projection", mCamera3.getPerspectiveProjectionMatrix());
   mPlaneTexture->bind(0, mDiffuseShader->getUniformLocation("diffuseTex"));

   // Loop over the plane meshes and render each one
   for (unsigned int i = 0,
        size = static_cast<unsigned int>(mPlaneMeshes.size());
        i < size;
        ++i)
   {
      mPlaneMeshes[i].Render();
   }

   mPlaneTexture->unbind(0);
   mDiffuseShader->use(false);
}

void PlayState::renderNormalsAndDepth()
{
   mNormalAndDepthShader->use(true);
   mNormalAndDepthShader->setUniformMat4("model",      transformToMat4(mPlaneModelTransform));
   mNormalAndDepthShader->setUniformMat4("view",       mCamera3.getViewMatrix());
   mNormalAndDepthShader->setUniformMat4("projection", mCamera3.getPerspectiveProjectionMatrix());
   mNormalAndDepthShader->setUniformMat3("normalMat",  glm::mat3(1.0f));

   // Loop over the normal plane meshes and render each one
   for (unsigned int i = 0,
        size = static_cast<unsigned int>(mNormalPlaneMeshes.size());
        i < size;
        ++i)
   {
      mNormalPlaneMeshes[i].Render();
   }

   mNormalAndDepthShader->use(false);
}
