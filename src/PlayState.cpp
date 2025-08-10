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
   // Initialize the Gourad shader
   mGouradShader = ResourceManager<Shader>().loadUnmanagedResource<ShaderLoader>("resources/shaders/gourad.vert",
                                                                                 "resources/shaders/gourad.frag");

   // Initialize the normal and depth shader
   mNormalAndDepthShader = ResourceManager<Shader>().loadUnmanagedResource<ShaderLoader>("resources/shaders/normal_and_depth.vert",
                                                                                         "resources/shaders/normal_and_depth.frag");

   loadModels();

   mWindow->setDecalRenderer(mDecalRenderer);
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

void PlayState::loadModels()
{
   cgltf_data *data;

   // Load the inverted icosphere
   data = LoadGLTFFile("resources/models/inverted_icosphere/inverted_icosphere.glb");
   mInvertedIcosphereMeshes = LoadStaticMeshes(data);
   FreeGLTFFile(data);

   // Load the normal inverted icosphere
   data = LoadGLTFFile("resources/models/inverted_icosphere/inverted_icosphere.glb");
   mNormalInvertedIcosphereMeshes = LoadStaticMeshes(data);
   FreeGLTFFile(data);

   int positionsAttribLoc = mGouradShader->getAttributeLocation("position");
   int normalsAttribLoc = mGouradShader->getAttributeLocation("normal");
   int texCoordsAttribLoc = mGouradShader->getAttributeLocation("texCoord");

   for (unsigned int i = 0,
                     size = static_cast<unsigned int>(mInvertedIcosphereMeshes.size());
        i < size;
        ++i)
   {
      mInvertedIcosphereMeshes[i].ConfigureVAO(positionsAttribLoc,
                                               normalsAttribLoc,
                                               texCoordsAttribLoc);
   }

   positionsAttribLoc = mNormalAndDepthShader->getAttributeLocation("position");
   normalsAttribLoc = mNormalAndDepthShader->getAttributeLocation("normal");
   texCoordsAttribLoc = mNormalAndDepthShader->getAttributeLocation("texCoord");

   for (unsigned int i = 0,
                     size = static_cast<unsigned int>(mNormalInvertedIcosphereMeshes.size());
        i < size;
        ++i)
   {
      mNormalInvertedIcosphereMeshes[i].ConfigureVAO(positionsAttribLoc,
                                                     normalsAttribLoc,
                                                     texCoordsAttribLoc);
   }
}

void PlayState::userInterface()
{
   ImGui::SetNextWindowPos(ImVec2(10.0f, 10.0f), ImGuiCond_Appearing);

   char title[64];
   snprintf(title, 32, "Decal Renderer (%.1f FPS)###DecalRenderer", ImGui::GetIO().Framerate);
   ImGui::Begin(title, nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize);

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

void PlayState::renderWorld()
{
   mGouradShader->use(true);
   mGouradShader->setUniformMat4("model", glm::mat4(1.0f));
   mGouradShader->setUniformMat4("view", mCamera3.getViewMatrix());
   mGouradShader->setUniformMat4("projection", mCamera3.getPerspectiveProjectionMatrix());
   mGouradShader->setUniformVec3("diffuseColor", glm::vec3(1.0f));

   // Loop over the inverted icosphere meshes and render each one
   for (unsigned int i = 0,
                     size = static_cast<unsigned int>(mInvertedIcosphereMeshes.size());
        i < size;
        ++i)
   {
      mInvertedIcosphereMeshes[i].Render();
   }

   mGouradShader->use(false);
}

void PlayState::renderNormalsAndDepth()
{
   mNormalAndDepthShader->use(true);
   mNormalAndDepthShader->setUniformMat4("model", glm::mat4(1.0f));
   mNormalAndDepthShader->setUniformMat4("view", mCamera3.getViewMatrix());
   mNormalAndDepthShader->setUniformMat4("projection", mCamera3.getPerspectiveProjectionMatrix());
   mNormalAndDepthShader->setUniformMat3("normalMat", glm::mat3(1.0f));

   // Loop over the normal inverted meshes and render each one
   for (unsigned int i = 0,
                     size = static_cast<unsigned int>(mNormalInvertedIcosphereMeshes.size());
        i < size;
        ++i)
   {
      mNormalInvertedIcosphereMeshes[i].Render();
   }

   mNormalAndDepthShader->use(false);
}
