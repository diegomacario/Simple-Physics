#include "Decal.h"

Decal::Decal(const glm::mat4& modelMatrix, const glm::vec3& normal)
   : mModelMatrix(modelMatrix)
   , mInverseModelMatrix(glm::inverse(modelMatrix))
   , mNormal(normal)
{

}
