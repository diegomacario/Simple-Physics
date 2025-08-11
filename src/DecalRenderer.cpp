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
    : mWidthOfFramebuffer(widthOfFramebuffer), mHeightOfFramebuffer(heightOfFramebuffer), mDecalFBO(0), mNormalTexture(0), mDepthTexture(0), mNormalThreshold(glm::cos(glm::radians(89.0f))), mDecalIndex(0), mMaxNumDecals(100), mDelayBetweenCircles(0.1f)
{
}

void DecalRenderer::renderDecals(const glm::mat4 &viewMatrix, const glm::mat4 &perspectiveProjectionMatrix, bool displayDecalOBBs, bool displayDiscardedDecalParts)
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

void DecalRenderer::addDecal(const glm::vec3 &decalPosition, const glm::vec3 &decalNormal)
{
   Transform modelTransform(decalPosition, Q::lookRotation(decalNormal, glm::vec3(0.0f, 1.0f, 0.0f)), glm::vec3(1.0f, 1.0f, 1.0f));
   mDecals.emplace_back(modelTransform, decalNormal, mDecalIndex, mDelayBetweenCircles);
   mGrowingDecals.push_back(std::prev(mDecals.end()));
   mDecalIndex = (mDecalIndex + 1) % 20;
}

void DecalRenderer::setDecalScale(float scale)
{
   ScalarFrame &frame1 = mGrowAnimation.GetFrame(1);
   frame1.mValue[0] = scale;

   ScalarFrame &frame0 = mShrinkAnimation.GetFrame(0);
   frame0.mValue[0] = scale;

   for (const std::list<Decal>::iterator &decalIter : mStableDecals)
   {
      decalIter->updateScale(scale);
   }
}

void DecalRenderer::setDecalBounce(float bounce)
{
   ScalarFrame &frame0 = mGrowAnimation.GetFrame(0);
   frame0.mOutSlope[0] = bounce;
}

void DecalRenderer::loadCube()
{
   cgltf_data *data = LoadGLTFFile("resources/models/cube/cube.glb");
   mCubeMeshes = LoadStaticMeshes(data);
   FreeGLTFFile(data);

   int positionsAttribLoc = mDecalShader->getAttributeLocation("position");
   int normalsAttribLoc = mDecalShader->getAttributeLocation("normal");
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

void DecalRenderer::renderAnimatedDecals(const std::deque<std::list<Decal>::iterator> &decals)
{
   mDecalShader->setUniformBool("animated", true);
   for (const std::list<Decal>::iterator &decalIter : decals)
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
   for (const std::list<Decal>::iterator &decalIter : mStableDecals)
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
