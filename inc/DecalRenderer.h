#ifndef DECAL_RENDERER_H
#define DECAL_RENDERER_H

#include <glm/glm.hpp>

class DecalRenderer
{
public:

   DecalRenderer();
   ~DecalRenderer();

   DecalRenderer(const DecalRenderer&) = delete;
   DecalRenderer& operator=(const DecalRenderer&) = delete;

   DecalRenderer(DecalRenderer&& rhs) = delete;
   DecalRenderer& operator=(DecalRenderer&& rhs) = delete;

   void bindDepthFBO();
   void unbindDepthFBO();
   void render(const glm::mat4& projectionView, const glm::vec3& cameraPosition);

//private:

   void configureDepthFBO();

   unsigned int createDepthTextureAttachment(int width, int height);

   unsigned int mDepthFBO;
   unsigned int mDepthTexture;
};

#endif
