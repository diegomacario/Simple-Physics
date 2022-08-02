#ifdef __EMSCRIPTEN__
#include <GLES3/gl3.h>
#else
#include <glad/glad.h>
#endif

#include <iostream>

#include "DecalRenderer.h"

DecalRenderer::DecalRenderer()
   : mDepthFBO(0)
   , mDepthTexture(0)
{
   configureDepthFBO();
}

DecalRenderer::~DecalRenderer()
{
   glDeleteFramebuffers(1, &mDepthFBO);
   glDeleteTextures(1, &mDepthTexture);
}

void DecalRenderer::bindDepthFBO()
{
   glBindFramebuffer(GL_FRAMEBUFFER, mDepthFBO);
}

void DecalRenderer::unbindDepthFBO()
{
   glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void DecalRenderer::render(const glm::mat4& projectionView, const glm::vec3& cameraPosition)
{
   glActiveTexture(GL_TEXTURE0);
   glBindTexture(GL_TEXTURE_2D, mDepthTexture);
   //mShader->setUniformInt("depthMap", 0);

   // Render
   // ...

   // Unbind textures
   glActiveTexture(GL_TEXTURE0);
   glBindTexture(GL_TEXTURE_2D, 0);
}

void DecalRenderer::configureDepthFBO()
{
   glGenFramebuffers(1, &mDepthFBO);
   glBindFramebuffer(GL_FRAMEBUFFER, mDepthFBO);

   mDepthTexture = createDepthTextureAttachment(1280 * 2, 720 * 2);

   if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
   {
      std::cout << "Error - DecalRenderer::ConfigureDepthFBO - Depth framebuffer is not complete" << "\n";
   }

   glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

unsigned int DecalRenderer::createDepthTextureAttachment(int width, int height)
{
   // Create a texture and use it as a depth attachment
   unsigned int depthTexture;
   glGenTextures(1, &depthTexture);
   glBindTexture(GL_TEXTURE_2D, depthTexture);
   glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32F, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
   glBindTexture(GL_TEXTURE_2D, 0);
   glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthTexture, 0);
   return depthTexture;
}
