#ifndef DECAL_RENDERER_H
#define DECAL_RENDERER_H

#include "Shader.h"
#include "AnimatedMesh.h"

class DecalRenderer
{
public:

   DecalRenderer();
   ~DecalRenderer();

   DecalRenderer(const DecalRenderer&) = delete;
   DecalRenderer& operator=(const DecalRenderer&) = delete;

   DecalRenderer(DecalRenderer&& rhs) = delete;
   DecalRenderer& operator=(DecalRenderer&& rhs) = delete;

   void         bindDepthFBO();
   void         unbindDepthFBO();
   void         renderDepthTextureToFullScreenQuad();

private:

   void         configureDepthFBO();

   unsigned int createDepthTextureAttachment(int width, int height);

   void         loadQuad();

   unsigned int              mDepthFBO;
   unsigned int              mDepthTexture;

   std::shared_ptr<Shader>   mFullScreenQuadWithDepthTextureShader;

   std::vector<AnimatedMesh> mQuadMeshes;
};

#endif
