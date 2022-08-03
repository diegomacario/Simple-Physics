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

   void         bindDecalFBO();
   void         unbindDecalFBO();
   void         renderDecals(const glm::mat4& viewMatrix, const glm::mat4& perspectiveProjectionMatrix);
   void         renderNormalTextureToFullScreenQuad();
   void         renderDepthTextureToFullScreenQuad();
   void         resizeTextures(unsigned int widthOfFramebuffer, unsigned int heightOfFramebuffer);

private:

   void         configureDecalFBO();

   unsigned int createColorTextureAttachment();
   unsigned int createDepthTextureAttachment();

   void         loadQuad();
   void         loadCube();

   unsigned int              mWidthOfFramebuffer;
   unsigned int              mHeightOfFramebuffer;
   unsigned int              mDecalFBO;
   unsigned int              mNormalTexture;
   unsigned int              mDepthTexture;

   std::shared_ptr<Shader>   mFullScreenQuadWithNormalTextureShader;
   std::shared_ptr<Shader>   mFullScreenQuadWithDepthTextureShader;
   std::shared_ptr<Shader>   mDecalShader;

   std::vector<AnimatedMesh> mQuadMeshes;
   std::vector<AnimatedMesh> mCubeMeshes;

   std::shared_ptr<Texture>  mDecalTexture;
};

#endif
