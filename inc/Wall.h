#ifndef WALL_H
#define WALL_H

#include <glm/glm.hpp>

// A wall is defined using the 3D plane equation:
// ax + by + cz + d = 0
// Where [a, b, c] is the normal of the plane
// And d is:
// -dot([a, b, c], (x0, y0, z0))
// Where (x0, y0, z0) is any point on the plane

class Wall
{
public:

   Wall(const glm::vec3& position,
        const glm::mat3& orientation,
        float width,
        float height);
   ~Wall() = default;

   glm::mat4 getModelMatrix() const { return mModelMatrix; }
   glm::mat3 getNormalMatrix() const { return mNormalMatrix; }
   glm::vec3 getNormal() const { return mNormal; }
   float     getD() const { return mD; }

private:

   glm::vec3 mPosition;
   glm::mat3 mOrientation;
   float     mWidth;
   float     mHeight;
   glm::vec3 mNormal;
   float     mD;
   glm::mat4 mModelMatrix;
   glm::mat3 mNormalMatrix;
};

#endif
