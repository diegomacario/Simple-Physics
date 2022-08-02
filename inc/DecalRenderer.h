#ifndef DECAL_RENDERER_H
#define DECAL_RENDERER_H

#include "Shader.h"
#include "AnimatedMesh.h"
#include "Texture.h"

class DecalRenderer
{
public:

   DecalRenderer(unsigned int widthOfFramebuffer, unsigned int heightOfFramebuffer);
   ~DecalRenderer();

   DecalRenderer(const DecalRenderer&) = delete;
   DecalRenderer& operator=(const DecalRenderer&) = delete;

   DecalRenderer(DecalRenderer&& rhs) = delete;
   DecalRenderer& operator=(DecalRenderer&& rhs) = delete;

   void         bindDepthFBO();
   void         unbindDepthFBO();
   void         renderDecals(const glm::mat4& viewMatrix, const glm::mat4& perspectiveProjectionMatrix);
   void         renderDepthTextureToFullScreenQuad();
   void         resizeDepthTexture(unsigned int widthOfFramebuffer, unsigned int heightOfFramebuffer);

private:

   void         configureDepthFBO();

   unsigned int createDepthTextureAttachment();

   void         loadQuad();
   void         loadCube();

   unsigned int              mWidthOfFramebuffer;
   unsigned int              mHeightOfFramebuffer;
   unsigned int              mDepthFBO;
   unsigned int              mDepthTexture;

   std::shared_ptr<Shader>   mFullScreenQuadWithDepthTextureShader;
   std::shared_ptr<Shader>   mDecalShader;

   std::vector<AnimatedMesh> mQuadMeshes;
   std::vector<AnimatedMesh> mCubeMeshes;

   std::shared_ptr<Texture>  mDecalTexture;
};

#endif
