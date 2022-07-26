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
   , mDecalIndex(0)
   , mMaxNumDecals(100)
   , mDelayBetweenCircles(0.1f)
{
   configureDecalFBO();

   // Initialize the full screen quad with depth texture shader
   mFullScreenQuadWithDepthTextureShader = ResourceManager<Shader>().loadUnmanagedResource<ShaderLoader>("resources/shaders/full_screen_quad_with_depth_texture_shader.vert", "resources/shaders/full_screen_quad_with_depth_texture_shader.frag");

   // Initialize the full screen quad with normal texture shader
   mFullScreenQuadWithNormalTextureShader = ResourceManager<Shader>().loadUnmanagedResource<ShaderLoader>("resources/shaders/full_screen_quad_with_normal_texture_shader.vert", "resources/shaders/full_screen_quad_with_normal_texture_shader.frag");

   // Initialize the decal shader
   mDecalShader = ResourceManager<Shader>().loadUnmanagedResource<ShaderLoader>("resources/shaders/decal.vert", "resources/shaders/decal.frag");

   // Load the textures of the stable decals
   for (int i = 0; i < 20; ++i)
   {
      std::string decalPath = "resources/models/decals/circles_" + std::to_string(i) + ".png";
      mDecalTextures[i] = ResourceManager<Texture>().loadUnmanagedResource<TextureLoader>(decalPath, nullptr, nullptr, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_LINEAR, GL_LINEAR, false);
   }

   // Load the textures of the animated decals
   mCircleTextures = {
      ResourceManager<Texture>().loadUnmanagedResource<TextureLoader>("resources/models/decals/circle_0.png", nullptr, nullptr, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_LINEAR, GL_LINEAR, false),
      ResourceManager<Texture>().loadUnmanagedResource<TextureLoader>("resources/models/decals/circle_1.png", nullptr, nullptr, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_LINEAR, GL_LINEAR, false),
      ResourceManager<Texture>().loadUnmanagedResource<TextureLoader>("resources/models/decals/circle_2.png", nullptr, nullptr, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_LINEAR, GL_LINEAR, false),
      ResourceManager<Texture>().loadUnmanagedResource<TextureLoader>("resources/models/decals/circle_3.png", nullptr, nullptr, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_LINEAR, GL_LINEAR, false)
   };

   // Load the colors of the animated decals
   mCircleColors = {
      std::array<glm::vec3, 4>{ hexToColor(0x6A2C70), hexToColor(0xB83B5E), hexToColor(0xF08A5D), hexToColor(0xF9ED69) },
      std::array<glm::vec3, 4>{ hexToColor(0xF9ED69), hexToColor(0xF08A5D), hexToColor(0xB83B5E), hexToColor(0x6A2C70) },
      std::array<glm::vec3, 4>{ hexToColor(0x2D4059), hexToColor(0xEA5455), hexToColor(0xF07B3F), hexToColor(0xFFD460) },
      std::array<glm::vec3, 4>{ hexToColor(0xFFD460), hexToColor(0xF07B3F), hexToColor(0xEA5455), hexToColor(0x2D4059) },
      std::array<glm::vec3, 4>{ hexToColor(0xD92027), hexToColor(0xFF9234), hexToColor(0xFFCD3C), hexToColor(0x35D0BA) },
      std::array<glm::vec3, 4>{ hexToColor(0x35D0BA), hexToColor(0xFFCD3C), hexToColor(0xFF9234), hexToColor(0xD92027) },
      std::array<glm::vec3, 4>{ hexToColor(0x071A52), hexToColor(0x086972), hexToColor(0x17B978), hexToColor(0xA7FF83) },
      std::array<glm::vec3, 4>{ hexToColor(0xA7FF83), hexToColor(0x17B978), hexToColor(0x086972), hexToColor(0x071A52) },
      std::array<glm::vec3, 4>{ hexToColor(0xF7FD04), hexToColor(0xF9B208), hexToColor(0xF98404), hexToColor(0xFC5404) },
      std::array<glm::vec3, 4>{ hexToColor(0xFC5404), hexToColor(0xF98404), hexToColor(0xF9B208), hexToColor(0xF7FD04) },
      std::array<glm::vec3, 4>{ hexToColor(0x00AD7C), hexToColor(0x52D681), hexToColor(0xB5FF7D), hexToColor(0xFFF8B5) },
      std::array<glm::vec3, 4>{ hexToColor(0xFFF8B5), hexToColor(0xB5FF7D), hexToColor(0x52D681), hexToColor(0x00AD7C) },
      std::array<glm::vec3, 4>{ hexToColor(0xF06868), hexToColor(0xFAB57A), hexToColor(0xEDF798), hexToColor(0x80D6FF) },
      std::array<glm::vec3, 4>{ hexToColor(0x80D6FF), hexToColor(0xEDF798), hexToColor(0xFAB57A), hexToColor(0xF06868) },
      std::array<glm::vec3, 4>{ hexToColor(0x0CECDD), hexToColor(0xFFF338), hexToColor(0xFF67E7), hexToColor(0xC400FF) },
      std::array<glm::vec3, 4>{ hexToColor(0xC400FF), hexToColor(0xFF67E7), hexToColor(0xFFF338), hexToColor(0x0CECDD) },
      std::array<glm::vec3, 4>{ hexToColor(0x0F0766), hexToColor(0x59057B), hexToColor(0xAB0E86), hexToColor(0xE01171) },
      std::array<glm::vec3, 4>{ hexToColor(0xE01171), hexToColor(0xAB0E86), hexToColor(0x59057B), hexToColor(0x0F0766) },
      std::array<glm::vec3, 4>{ hexToColor(0x00E0FF), hexToColor(0x74F9FF), hexToColor(0xA6FFF2), hexToColor(0xE8FFE8) },
      std::array<glm::vec3, 4>{ hexToColor(0xE8FFE8), hexToColor(0xA6FFF2), hexToColor(0x74F9FF), hexToColor(0x00E0FF) }
   };

   loadQuad();
   loadCube();
   composeGrowAnimation();
   composeShrinkAnimation();
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

   mDecalShader->setUniformFloat("normalThreshold", mNormalThreshold);
   mDecalShader->setUniformBool("displayDecalOBBs", displayDecalOBBs);
   mDecalShader->setUniformBool("displayDiscardedDecalParts", displayDiscardedDecalParts);

   renderAnimatedDecals(mShrinkingDecals);
   renderStableDecals();
   renderAnimatedDecals(mGrowingDecals);

   glActiveTexture(GL_TEXTURE1);
   glBindTexture(GL_TEXTURE_2D, 0);

   glActiveTexture(GL_TEXTURE0);
   glBindTexture(GL_TEXTURE_2D, 0);

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
   mDecals.emplace_back(modelTransform, decalNormal, mDecalIndex, mDelayBetweenCircles);
   mGrowingDecals.push_back(std::prev(mDecals.end()));
   mDecalIndex = (mDecalIndex + 1) % 20;
}

void DecalRenderer::updateDecals(float playbackSpeed)
{
   updateGrowingDecals(playbackSpeed);
   updateStableDecals();
   updateShrinkingDecals(playbackSpeed);
}

void DecalRenderer::reset()
{
   mGrowingDecals.clear();
   mStableDecals.clear();
   mShrinkingDecals.clear();
   mDecals.clear();
   mDecalIndex = 0;
}

void DecalRenderer::setDecalScale(float scale)
{
   ScalarFrame& frame1 = mGrowAnimation.GetFrame(1);
   frame1.mValue[0]    = scale;

   ScalarFrame& frame0 = mShrinkAnimation.GetFrame(0);
   frame0.mValue[0]    = scale;

   for (const std::list<Decal>::iterator& decalIter : mStableDecals)
   {
      decalIter->updateScale(scale);
   }
}

void DecalRenderer::setDecalBounce(float bounce)
{
   ScalarFrame& frame0 = mGrowAnimation.GetFrame(0);
   frame0.mOutSlope[0] = bounce;
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

void DecalRenderer::composeShrinkAnimation()
{
   // Compose the shrink animation
   mShrinkAnimation.SetInterpolation(Interpolation::Cubic);
   mShrinkAnimation.SetNumberOfFrames(2);

   // Frame 0
   ScalarFrame& frame0 = mShrinkAnimation.GetFrame(0);
   frame0.mTime        = 0.0f;
   frame0.mInSlope[0]  = 0.0f;
   frame0.mValue[0]    = 1.0f;
   frame0.mOutSlope[0] = 0.0f;

   // Frame 1
   ScalarFrame& frame1 = mShrinkAnimation.GetFrame(1);
   frame1.mTime        = 1.0f;
   frame1.mInSlope[0]  = 0.0f;
   frame1.mValue[0]    = 0.0f;
   frame1.mOutSlope[0] = 0.0f;
}

void DecalRenderer::updateGrowingDecals(float playbackSpeed)
{
   unsigned int numDecalsDoneGrowing = 0;
   for (const std::list<Decal>::iterator& growingDecalIter : mGrowingDecals)
   {
      if (growingDecalIter->grow(mGrowAnimation, playbackSpeed))
      {
         mStableDecals.push_back(growingDecalIter);
         ++numDecalsDoneGrowing;
      }
   }

   if (numDecalsDoneGrowing > 0)
   {
      mGrowingDecals.erase(mGrowingDecals.begin(), std::next(mGrowingDecals.begin(), numDecalsDoneGrowing));
   }
}

void DecalRenderer::updateStableDecals()
{
   int numDecalsToStartShrinking = static_cast<int>(mStableDecals.size()) - mMaxNumDecals;

   /*
   // Start erasing all dead decals at once
   if (numDecalsToStartShrinking > 0)
   {
      for (int i = 0; i < numDecalsToStartShrinking; ++i)
      {
         mShrinkingDecals.push_back(mStableDecals[i]);
      }

      mStableDecals.erase(mStableDecals.begin(), std::next(mStableDecals.begin(), numDecalsToStartShrinking));
   }
   */

   // Start erasing one dead decal per frame
   if (numDecalsToStartShrinking > 0)
   {
      mShrinkingDecals.push_back(mStableDecals[0]);
      mStableDecals.erase(mStableDecals.begin(), std::next(mStableDecals.begin(), 1));
   }
}

void DecalRenderer::updateShrinkingDecals(float playbackSpeed)
{
   unsigned int numDecalsDoneShrinking = 0;
   for (const std::list<Decal>::iterator& shrinkingDecalIter : mShrinkingDecals)
   {
      if (shrinkingDecalIter->shrink(mShrinkAnimation, playbackSpeed))
      {
         ++numDecalsDoneShrinking;
      }
   }

   if (numDecalsDoneShrinking > 0)
   {
      mShrinkingDecals.erase(mShrinkingDecals.begin(), std::next(mShrinkingDecals.begin(), numDecalsDoneShrinking));
      mDecals.erase(mDecals.begin(), std::next(mDecals.begin(), numDecalsDoneShrinking));
   }
}

void DecalRenderer::renderAnimatedDecals(const std::deque<std::list<Decal>::iterator>& decals)
{
   mDecalShader->setUniformBool("animated", true);
   for (const std::list<Decal>::iterator& decalIter : decals)
   {
      mDecalShader->setUniformVec3("decalNormal", decalIter->getNormal());

      for (unsigned int i = 0; i < 4; ++i)
      {
         mDecalShader->setUniformMat4("model", decalIter->getCircleModelMatrices()[i]);
         mDecalShader->setUniformMat4("inverseModel", decalIter->getCircleInverseModelMatrices()[i]);
         mCircleTextures[i]->bind(2, mDecalShader->getUniformLocation("decalTex"));
         mDecalShader->setUniformVec3("decalColor", mCircleColors[decalIter->getDecalIndex()][i]);

         // Loop over the cube meshes and render each one
         for (unsigned int meshIndex = 0,
              numMeshes = static_cast<unsigned int>(mCubeMeshes.size());
              meshIndex < numMeshes;
              ++meshIndex)
         {
            mCubeMeshes[meshIndex].Render();
         }

         mCircleTextures[i]->unbind(2);
      }
   }
}

void DecalRenderer::renderStableDecals()
{
   mDecalShader->setUniformBool("animated", false);
   for (const std::list<Decal>::iterator& decalIter : mStableDecals)
   {
      mDecalShader->setUniformMat4("model", decalIter->getModelMatrix());
      mDecalShader->setUniformMat4("inverseModel", decalIter->getInverseModelMatrix());
      mDecalTextures[decalIter->getDecalIndex()]->bind(2, mDecalShader->getUniformLocation("decalTex"));
      mDecalShader->setUniformVec3("decalNormal", decalIter->getNormal());

      // Loop over the cube meshes and render each one
      for (unsigned int meshIndex = 0,
           numMeshes = static_cast<unsigned int>(mCubeMeshes.size());
           meshIndex < numMeshes;
           ++meshIndex)
      {
         mCubeMeshes[meshIndex].Render();
      }

      mDecalTextures[decalIter->getDecalIndex()]->unbind(2);
   }
}

glm::vec3 DecalRenderer::hexToColor(int hex)
{
   float r = static_cast<float>(((hex >> 16) & 0xff)) / 255.0f;
   float g = static_cast<float>(((hex >> 8)  & 0xff)) / 255.0f;
   float b = static_cast<float>(( hex        & 0xff)) / 255.0f;

   return glm::vec3(r, g, b);
}
