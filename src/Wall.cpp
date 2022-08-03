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

   // Compute the model matrix
   // 3) Translate the plane
   mModelMatrix = glm::translate(glm::mat4(1.0f), mPosition);
   // 2) Rotate the plane
   mModelMatrix = mModelMatrix * glm::mat4(mOrientation);
   // 1) Scale the plane
   mModelMatrix = glm::scale(mModelMatrix, glm::vec3(mWidth, mHeight, 1.0f));

   // Compute the normal matrix
   mNormalMatrix = glm::mat3(glm::transpose(glm::inverse(mModelMatrix)));
}
