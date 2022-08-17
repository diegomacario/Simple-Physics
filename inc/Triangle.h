#ifndef TRIANGLE_H
#define TRIANGLE_H

#include <glm/glm.hpp>

#include "Ray.h"

/*
         C
        / \
       /   \
      /     \
     /       \
    /         \
   A-----------B
*/

struct Triangle
{
   Triangle();
   Triangle(const glm::vec3& vA, const glm::vec3& vB, const glm::vec3& vC);
   Triangle(const glm::vec3& vA, const glm::vec3& vB, const glm::vec3& vC, int idx);

   bool  isFrontFacingTo(const glm::vec3& direction) const;
   float signedDistanceTo(const glm::vec3& point) const;

   // Assumes that the point is in the triangle plane
   bool  isPointInTriangle(const glm::vec3& point) const;

   bool  doesRayIntersectTriangle(const Ray& ray, glm::vec3& outHitPoint) const;

   // Vertices must be specified in a CCWISE order
   glm::vec3 vertexA;
   glm::vec3 vertexB;
   glm::vec3 vertexC;

   glm::vec3 normal;

   // This is only needed by the system that detects whether a player is looking at a painting,
   // and by the system that detects in which zone a player is
   // It feels wrong to increase the size of all triangles just for those systems,
   // but this approach helps us avoid a lot of code duplication
   // TODO: Create a derived class called IndexedTriangle?
   int       index;
};

#endif
