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
   , mCamera3(4.5f, 0.0f, glm::vec3(0.0f), Q::quat(), glm::vec3(0.0f, 0.0f, 0.0f), 1.0f, 10.0f, -90.0f, 90.0f, 45.0f, 1280.0f / 720.0f, 0.1f, 130.0f, 0.25f)
   , mDecalRenderer(std::make_shared<DecalRenderer>(window->getWidthOfFramebufferInPix(), window->getHeightOfFramebufferInPix()))
{
   // Initialize the diffuse shader
   mDiffuseShader = ResourceManager<Shader>().loadUnmanagedResource<ShaderLoader>("resources/shaders/diffuse.vert",
                                                                                  "resources/shaders/diffuse.frag");

   loadModels();

   mWindow->setDecalRenderer(mDecalRenderer);
}

void PlayState::initializeState()
{

}

void PlayState::enter()
{
   initializeState();
   resetCamera();
   resetScene();
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
   // TODO: Handle simulation errors
   mWorld.simulate(deltaTime);
}

void PlayState::render()
{
   ImGui_ImplOpenGL3_NewFrame();
   ImGui_ImplGlfw_NewFrame();
   ImGui::NewFrame();

   userInterface();

   // Render the walls and rigid bodies into the depth texture
   mDecalRenderer->bindDepthFBO();
   glClear(GL_DEPTH_BUFFER_BIT);
   renderWalls();
   renderRigidBodies();
   mDecalRenderer->unbindDepthFBO();

#ifndef __EMSCRIPTEN__
   mWindow->bindMultisampleFramebuffer();
#endif
   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

   // Enable depth testing for 3D objects
   glEnable(GL_DEPTH_TEST);
   glClear(GL_DEPTH_BUFFER_BIT);

   //renderWalls();
   //renderRigidBodies();
   mDecalRenderer->renderDepthTextureToFullScreenQuad();

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
   // Load the texture of the cube
   mCubeTexture = ResourceManager<Texture>().loadUnmanagedResource<TextureLoader>("resources/models/cube/cube.png");

   // Load the cube
   cgltf_data* data = LoadGLTFFile("resources/models/cube/cube.glb");
   mCubeMeshes = LoadStaticMeshes(data);
   FreeGLTFFile(data);

   // Load the texture of the plane
   mPlaneTexture = ResourceManager<Texture>().loadUnmanagedResource<TextureLoader>("resources/models/plane/plane.png");

   // Load the plane
   data = LoadGLTFFile("resources/models/plane/plane.glb");
   mPlaneMeshes = LoadStaticMeshes(data);
   FreeGLTFFile(data);

   int positionsAttribLoc = mDiffuseShader->getAttributeLocation("position");
   int normalsAttribLoc   = mDiffuseShader->getAttributeLocation("normal");
   int texCoordsAttribLoc = mDiffuseShader->getAttributeLocation("texCoord");

   for (unsigned int i = 0,
        size = static_cast<unsigned int>(mCubeMeshes.size());
        i < size;
        ++i)
   {
      mCubeMeshes[i].ConfigureVAO(positionsAttribLoc,
                                  normalsAttribLoc,
                                  texCoordsAttribLoc);
   }

   for (unsigned int i = 0,
        size = static_cast<unsigned int>(mPlaneMeshes.size());
        i < size;
        ++i)
   {
      mPlaneMeshes[i].ConfigureVAO(positionsAttribLoc,
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

   }

   ImGui::End();
}

void PlayState::resetScene()
{

}

void PlayState::resetCamera()
{
   mCamera3.reposition(4.5f, 0.0f, glm::vec3(0.0f), Q::quat(), glm::vec3(0.0f, 0.0f, 0.0f), 1.0f, 10.0f, -90.0f, 90.0f);
   mCamera3.processMouseMovement(180.0f / 0.25f, 0.0f);
}

void PlayState::renderRigidBodies()
{
   mDiffuseShader->use(true);
   mDiffuseShader->setUniformMat4("view",       mCamera3.getViewMatrix());
   mDiffuseShader->setUniformMat4("projection", mCamera3.getPerspectiveProjectionMatrix());
   mCubeTexture->bind(0, mDiffuseShader->getUniformLocation("diffuseTex"));

   // Loop over the rigid bodies and render each one
   const std::vector<RigidBody>& rigidBodies = mWorld.getRigidBodies();
   for (const RigidBody& rigidBody : rigidBodies)
   {
      mDiffuseShader->setUniformMat4("model", rigidBody.getModelMatrix(current));

      // Loop over the cube meshes and render each one
      for (unsigned int i = 0,
           size = static_cast<unsigned int>(mCubeMeshes.size());
           i < size;
           ++i)
      {
         mCubeMeshes[i].Render();
      }
   }

   mCubeTexture->unbind(0);
   mDiffuseShader->use(false);
}

void PlayState::renderWalls()
{
   mDiffuseShader->use(true);
   mDiffuseShader->setUniformMat4("view",       mCamera3.getViewMatrix());
   mDiffuseShader->setUniformMat4("projection", mCamera3.getPerspectiveProjectionMatrix());
   mPlaneTexture->bind(0, mDiffuseShader->getUniformLocation("diffuseTex"));

   // Loop over the walls and render each one
   const std::vector<Wall>& walls = mWorld.getWalls();
   for (const Wall& wall : walls)
   {
      mDiffuseShader->setUniformMat4("model", wall.getModelMatrix());

      // Loop over the plane meshes and render each one
      for (unsigned int i = 0,
           size = static_cast<unsigned int>(mPlaneMeshes.size());
           i < size;
           ++i)
      {
         mPlaneMeshes[i].Render();
      }
   }

   mPlaneTexture->unbind(0);
   mDiffuseShader->use(false);
}
