#ifdef __EMSCRIPTEN__
#include <GLES3/gl3.h>
#else
#include <glad/glad.h>
#endif

#include <iostream>

#include "ResourceManager.h"
#include "ShaderLoader.h"
#include "GLTFLoader.h"
#include "TextureLoader.h"
#include "Transform.h"
#include "DecalRenderer.h"

DecalRenderer::DecalRenderer(unsigned int widthOfFramebuffer, unsigned int heightOfFramebuffer)
   : mWidthOfFramebuffer(widthOfFramebuffer)
   , mHeightOfFramebuffer(heightOfFramebuffer)
   , mDecalFBO(0)
   , mNormalTexture(0)
   , mDepthTexture(0)
   , mNormalThreshold(glm::cos(glm::radians(89.0f)))
{
   configureDecalFBO();

   // Initialize the full screen quad with depth texture shader
   mFullScreenQuadWithDepthTextureShader = ResourceManager<Shader>().loadUnmanagedResource<ShaderLoader>("resources/shaders/full_screen_quad_with_depth_texture_shader.vert", "resources/shaders/full_screen_quad_with_depth_texture_shader.frag");

   // Initialize the full screen quad with normal texture shader
   mFullScreenQuadWithNormalTextureShader = ResourceManager<Shader>().loadUnmanagedResource<ShaderLoader>("resources/shaders/full_screen_quad_with_normal_texture_shader.vert", "resources/shaders/full_screen_quad_with_normal_texture_shader.frag");

   // Initialize the decal shader
   mDecalShader = ResourceManager<Shader>().loadUnmanagedResource<ShaderLoader>("resources/shaders/decal.vert", "resources/shaders/decal.frag");

   // Load the texture of the decal
   mDecalTexture = ResourceManager<Texture>().loadUnmanagedResource<TextureLoader>("resources/models/decals/1024/circles_1_1024.png",
                                                                                   nullptr,
                                                                                   nullptr,
                                                                                   GL_CLAMP_TO_EDGE,
                                                                                   GL_CLAMP_TO_EDGE,
                                                                                   GL_LINEAR,
                                                                                   GL_LINEAR,
                                                                                   false);

   loadQuad();
   loadCube();
   composeGrowAnimation();
}

DecalRenderer::~DecalRenderer()
{
   glDeleteFramebuffers(1, &mDecalFBO);
   glDeleteTextures(1, &mNormalTexture);
   glDeleteTextures(1, &mDepthTexture);
}

void DecalRenderer::bindDecalFBO()
{
   glBindFramebuffer(GL_FRAMEBUFFER, mDecalFBO);
}

void DecalRenderer::unbindDecalFBO()
{
   glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void DecalRenderer::renderDecals(const glm::mat4& viewMatrix, const glm::mat4& perspectiveProjectionMatrix, bool displayDecalOBBs, bool displayDiscardedDecalParts)
{
   mDecalShader->use(true);

   mDecalShader->setUniformMat4("view", viewMatrix);
   mDecalShader->setUniformMat4("projection", perspectiveProjectionMatrix);
   mDecalShader->setUniformMat4("inverseView", glm::inverse(viewMatrix));
   mDecalShader->setUniformMat4("inverseProjection", glm::inverse(perspectiveProjectionMatrix));
   mDecalShader->setUniformFloat("width", mWidthOfFramebuffer);
   mDecalShader->setUniformFloat("height", mHeightOfFramebuffer);

   glActiveTexture(GL_TEXTURE0);
   glBindTexture(GL_TEXTURE_2D, mNormalTexture);
   mDecalShader->setUniformInt("normalTex", 0);

   glActiveTexture(GL_TEXTURE1);
   glBindTexture(GL_TEXTURE_2D, mDepthTexture);
   mDecalShader->setUniformInt("depthTex", 1);

   mDecalTexture->bind(2, mDecalShader->getUniformLocation("decalTex"));

   mDecalShader->setUniformFloat("normalThreshold", mNormalThreshold);
   mDecalShader->setUniformBool("displayDecalOBBs", displayDecalOBBs);
   mDecalShader->setUniformBool("displayDiscardedDecalParts", displayDiscardedDecalParts);

   // Render stable decals
   for (const std::list<Decal>::iterator& stableDecalIter : mStableDecals)
   {
      mDecalShader->setUniformMat4("model", stableDecalIter->getModelMatrix());
      mDecalShader->setUniformMat4("inverseModel", stableDecalIter->getInverseModelMatrix());
      mDecalShader->setUniformVec3("decalNormal", stableDecalIter->getNormal());

      // Loop over the cube meshes and render each one
      for (unsigned int meshIndex = 0,
           numMeshes = static_cast<unsigned int>(mCubeMeshes.size());
           meshIndex < numMeshes;
           ++meshIndex)
      {
         mCubeMeshes[meshIndex].Render();
      }
   }

   // Render growing decals
   for (const std::list<Decal>::iterator& growingDecalIter : mGrowingDecals)
   {
      mDecalShader->setUniformMat4("model", growingDecalIter->getModelMatrix());
      mDecalShader->setUniformMat4("inverseModel", growingDecalIter->getInverseModelMatrix());
      mDecalShader->setUniformVec3("decalNormal", growingDecalIter->getNormal());

      // Loop over the cube meshes and render each one
      for (unsigned int meshIndex = 0,
           numMeshes = static_cast<unsigned int>(mCubeMeshes.size());
           meshIndex < numMeshes;
           ++meshIndex)
      {
         mCubeMeshes[meshIndex].Render();
      }
   }

   glActiveTexture(GL_TEXTURE1);
   glBindTexture(GL_TEXTURE_2D, 0);

   glActiveTexture(GL_TEXTURE0);
   glBindTexture(GL_TEXTURE_2D, 0);

   mDecalTexture->unbind(2);

   mDecalShader->use(false);
}

void DecalRenderer::renderNormalTextureToFullScreenQuad()
{
   mFullScreenQuadWithNormalTextureShader->use(true);
   // We need to scale up the quad by 2 because it spans from -0.5 to 0.5, and we need it to span from -1.0 to 1.0 (NDC)
   Transform modelTransform(glm::vec3(0.0f, 0.0f, 0.0f), Q::quat(), glm::vec3(2.0f, 2.0f, 1.0f));
   mFullScreenQuadWithNormalTextureShader->setUniformMat4("model", transformToMat4(modelTransform));
   glActiveTexture(GL_TEXTURE0);
   glBindTexture(GL_TEXTURE_2D, mNormalTexture);
   mFullScreenQuadWithNormalTextureShader->setUniformInt("normalTex", 0);
   mFullScreenQuadWithNormalTextureShader->setUniformFloat("width", mWidthOfFramebuffer);
   mFullScreenQuadWithNormalTextureShader->setUniformFloat("height", mHeightOfFramebuffer);

   // Loop over the quad meshes and render each one
   for (unsigned int i = 0,
        size = static_cast<unsigned int>(mQuadMeshes.size());
        i < size;
        ++i)
   {
      mQuadMeshes[i].Render();
   }

   glActiveTexture(GL_TEXTURE0);
   glBindTexture(GL_TEXTURE_2D, 0);
   mFullScreenQuadWithNormalTextureShader->use(false);
}

void DecalRenderer::renderDepthTextureToFullScreenQuad()
{
   mFullScreenQuadWithDepthTextureShader->use(true);
   // We need to scale up the quad by 2 because it spans from -0.5 to 0.5, and we need it to span from -1.0 to 1.0 (NDC)
   Transform modelTransform(glm::vec3(0.0f, 0.0f, 0.0f), Q::quat(), glm::vec3(2.0f, 2.0f, 1.0f));
   mFullScreenQuadWithDepthTextureShader->setUniformMat4("model", transformToMat4(modelTransform));
   glActiveTexture(GL_TEXTURE0);
   glBindTexture(GL_TEXTURE_2D, mDepthTexture);
   mFullScreenQuadWithDepthTextureShader->setUniformInt("depthTex", 0);
   mFullScreenQuadWithDepthTextureShader->setUniformFloat("width", mWidthOfFramebuffer);
   mFullScreenQuadWithDepthTextureShader->setUniformFloat("height", mHeightOfFramebuffer);

   // Loop over the quad meshes and render each one
   for (unsigned int i = 0,
        size = static_cast<unsigned int>(mQuadMeshes.size());
        i < size;
        ++i)
   {
      mQuadMeshes[i].Render();
   }

   glActiveTexture(GL_TEXTURE0);
   glBindTexture(GL_TEXTURE_2D, 0);
   mFullScreenQuadWithDepthTextureShader->use(false);
}

void DecalRenderer::resizeTextures(unsigned int widthOfFramebuffer, unsigned int heightOfFramebuffer)
{
   glBindTexture(GL_TEXTURE_2D, mNormalTexture);
   glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, widthOfFramebuffer, heightOfFramebuffer, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
   glBindTexture(GL_TEXTURE_2D, 0);

   glBindTexture(GL_TEXTURE_2D, mDepthTexture);
   glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32F, widthOfFramebuffer, heightOfFramebuffer, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
   glBindTexture(GL_TEXTURE_2D, 0);

   mWidthOfFramebuffer  = widthOfFramebuffer;
   mHeightOfFramebuffer = heightOfFramebuffer;
}

void DecalRenderer::addDecal(const glm::vec3& decalPosition, const glm::vec3& decalNormal)
{
   Transform modelTransform(decalPosition, Q::lookRotation(decalNormal, glm::vec3(0.0f, 1.0f, 0.0f)), glm::vec3(1.0f, 1.0f, 1.0f));
   mDecals.emplace_back(modelTransform, decalNormal);
   mGrowingDecals.push_back(std::prev(mDecals.end()));
}

void DecalRenderer::updateDecals()
{
   unsigned int numDecalsDoneGrowing = 0;
   for (const std::list<Decal>::iterator& growingDecalIter : mGrowingDecals)
   {
      if (growingDecalIter->grow(mGrowAnimation))
      {
         mStableDecals.push_back(growingDecalIter);
         ++numDecalsDoneGrowing;
      }
   }

   if (numDecalsDoneGrowing > 0)
   {
      mGrowingDecals.erase(mGrowingDecals.begin(), std::next(mGrowingDecals.begin(), numDecalsDoneGrowing));
   }

   unsigned int numDecalsDoneLiving = 0;
   for (const std::list<Decal>::iterator& stableDecalIter : mStableDecals)
   {
      if (stableDecalIter->updateLifetime())
      {
         ++numDecalsDoneLiving;
      }
   }

   if (numDecalsDoneLiving > 0)
   {
      mStableDecals.erase(mStableDecals.begin(), std::next(mStableDecals.begin(), numDecalsDoneLiving));
      mDecals.erase(mDecals.begin(), std::next(mDecals.begin(), numDecalsDoneLiving));
   }
}

void DecalRenderer::configureDecalFBO()
{
   glGenFramebuffers(1, &mDecalFBO);
   glBindFramebuffer(GL_FRAMEBUFFER, mDecalFBO);

   mNormalTexture = createColorTextureAttachment();
   mDepthTexture = createDepthTextureAttachment();

   if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
   {
      std::cout << "Error - DecalRenderer::configureDecalFBO - Decal framebuffer is not complete" << "\n";
   }

   glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

unsigned int DecalRenderer::createColorTextureAttachment()
{
   // Create a texture and use it as a color attachment
   unsigned int colorTexture;
   glGenTextures(1, &colorTexture);
   glBindTexture(GL_TEXTURE_2D, colorTexture);
   glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, mWidthOfFramebuffer, mHeightOfFramebuffer, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
   glBindTexture(GL_TEXTURE_2D, 0);
   glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorTexture, 0);
   return colorTexture;
}

unsigned int DecalRenderer::createDepthTextureAttachment()
{
   // Create a texture and use it as a depth attachment
   unsigned int depthTexture;
   glGenTextures(1, &depthTexture);
   glBindTexture(GL_TEXTURE_2D, depthTexture);
   glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32F, mWidthOfFramebuffer, mHeightOfFramebuffer, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
   glBindTexture(GL_TEXTURE_2D, 0);
   glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthTexture, 0);
   return depthTexture;
}

void DecalRenderer::loadQuad()
{
   cgltf_data* data = LoadGLTFFile("resources/models/plane/plane.glb");
   mQuadMeshes = LoadStaticMeshes(data);
   FreeGLTFFile(data);

   int positionsAttribLoc = mFullScreenQuadWithDepthTextureShader->getAttributeLocation("position");
   int normalsAttribLoc   = mFullScreenQuadWithDepthTextureShader->getAttributeLocation("normal");
   int texCoordsAttribLoc = mFullScreenQuadWithDepthTextureShader->getAttributeLocation("texCoord");

   for (unsigned int i = 0,
        size = static_cast<unsigned int>(mQuadMeshes.size());
        i < size;
        ++i)
   {
      mQuadMeshes[i].ConfigureVAO(positionsAttribLoc,
                                  normalsAttribLoc,
                                  texCoordsAttribLoc);
   }
}

void DecalRenderer::loadCube()
{
   cgltf_data* data = LoadGLTFFile("resources/models/cube/cube.glb");
   mCubeMeshes = LoadStaticMeshes(data);
   FreeGLTFFile(data);

   int positionsAttribLoc = mDecalShader->getAttributeLocation("position");
   int normalsAttribLoc   = mDecalShader->getAttributeLocation("normal");
   int texCoordsAttribLoc = mDecalShader->getAttributeLocation("texCoord");

   for (unsigned int i = 0,
        size = static_cast<unsigned int>(mCubeMeshes.size());
        i < size;
        ++i)
   {
      mCubeMeshes[i].ConfigureVAO(positionsAttribLoc,
                                  normalsAttribLoc,
                                  texCoordsAttribLoc);
   }
}

void DecalRenderer::composeGrowAnimation()
{
   // Compose the grow animation
   mGrowAnimation.SetInterpolation(Interpolation::Cubic);
   mGrowAnimation.SetNumberOfFrames(2);

   // Frame 0
   ScalarFrame& frame0 = mGrowAnimation.GetFrame(0);
   frame0.mTime        = 0.0f;
   frame0.mInSlope[0]  = 0.0f;
   frame0.mValue[0]    = 0.0f;
   frame0.mOutSlope[0] = 4.5f;

   // Frame 1
   ScalarFrame& frame1 = mGrowAnimation.GetFrame(1);
   frame1.mTime        = 1.0f;
   frame1.mInSlope[0]  = 0.0f;
   frame1.mValue[0]    = 1.0f;
   frame1.mOutSlope[0] = 0.0f;
}
