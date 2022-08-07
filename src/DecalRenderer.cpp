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
   int width  = 0;
   int height = 0;
   mDecalTexture = ResourceManager<Texture>().loadUnmanagedResource<TextureLoader>("resources/models/decal/decal.png", &width, &height);
   mOneOverDecalAspectRatio = static_cast<float>(height) / static_cast<float>(width);

   loadQuad();
   loadCube();
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

   for (unsigned int decalIndex = 0,
        numDecals = static_cast<unsigned int>(mDecalModelMatrices.size());
        decalIndex < numDecals;
        ++decalIndex)
   {
      mDecalShader->setUniformMat4("model", mDecalModelMatrices[decalIndex]);
      mDecalShader->setUniformMat4("inverseModel", mDecalInverseModelMatrices[decalIndex]);
      mDecalShader->setUniformVec3("decalNormal", mDecalNormals[decalIndex]);

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
   Transform modelTransform(decalPosition, Q::lookRotation(decalNormal, glm::vec3(0.0f, 1.0f, 0.0f)), glm::vec3(0.5f, 0.5f * mOneOverDecalAspectRatio, 0.5f));
   glm::mat4 decalModelMatrix = transformToMat4(modelTransform);
   mDecalModelMatrices.push_back(decalModelMatrix);
   mDecalInverseModelMatrices.push_back(glm::inverse(decalModelMatrix));
   mDecalNormals.push_back(decalNormal);
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

