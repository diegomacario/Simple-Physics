#include <glm/gtc/matrix_transform.hpp>

#include "Wall.h"

Wall::Wall(const glm::vec3& position,
           const glm::mat3& orientation,
           float width,
           float height)
   : mPosition(position)
   , mOrientation(orientation)
   , mWidth(width)
   , mHeight(height)
{
   mNormal = orientation * glm::vec3(0.0f, 0.0f, 1.0f);
   mD      = -glm::dot(mNormal, mPosition);
}

glm::mat4 Wall::getModelMatrix() const
{
   // 3) Translate the plane
   glm::mat4 modelMatrix = glm::translate(glm::mat4(1.0f), mPosition);

   // 2) Rotate the plane
   modelMatrix = modelMatrix * glm::mat4(mOrientation);

   // 1) Scale the plane
   modelMatrix = glm::scale(modelMatrix, glm::vec3(mWidth, mHeight, 1.0f));

   return modelMatrix;
}
