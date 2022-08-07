#ifndef DECAL_H
#define DECAL_H

#include <glm/glm.hpp>

class Decal
{
public:

   Decal(const glm::mat4& modelMatrix, const glm::vec3& normal);
   ~Decal() = default;

   const glm::mat4& getModelMatrix() const { return mModelMatrix; }
   const glm::mat4& getInverseModelMatrix() const { return mInverseModelMatrix; }
   const glm::vec3& getNormal() const { return mNormal; }

private:

   glm::mat4 mModelMatrix;
   glm::mat4 mInverseModelMatrix;
   glm::vec3 mNormal;
};

#endif
