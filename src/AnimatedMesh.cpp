#ifdef __EMSCRIPTEN__
#include <GLES3/gl3.h>
#else
#include <glad/glad.h>
#endif

#include "AnimatedMesh.h"
#include "Transform.h"

AnimatedMesh::AnimatedMesh()
{
   glGenVertexArrays(1, &mVAO);
   glGenBuffers(3, &mVBOs[0]);
   glGenBuffers(1, &mEBO);
}

AnimatedMesh::~AnimatedMesh()
{
   glDeleteVertexArrays(1, &mVAO);
   glDeleteBuffers(3, &mVBOs[0]);
   glDeleteBuffers(1, &mEBO);
}

AnimatedMesh::AnimatedMesh(AnimatedMesh&& rhs) noexcept
   : mPositions(std::move(rhs.mPositions))
   , mNormals(std::move(rhs.mNormals))
   , mTexCoords(std::move(rhs.mTexCoords))
   , mIndices(std::move(rhs.mIndices))
   , mNumVertices(std::exchange(rhs.mNumVertices, 0))
   , mNumIndices(std::exchange(rhs.mNumIndices, 0))
   , mVAO(std::exchange(rhs.mVAO, 0))
   , mVBOs(std::exchange(rhs.mVBOs, std::array<unsigned int, 3>()))
   , mEBO(std::exchange(rhs.mEBO, 0))
{

}

AnimatedMesh& AnimatedMesh::operator=(AnimatedMesh&& rhs) noexcept
{
   mPositions   = std::move(rhs.mPositions);
   mNormals     = std::move(rhs.mNormals);
   mTexCoords   = std::move(rhs.mTexCoords);
   mIndices     = std::move(rhs.mIndices);
   mNumVertices = std::exchange(rhs.mNumVertices, 0);
   mNumIndices  = std::exchange(rhs.mNumIndices, 0);
   mVAO         = std::exchange(rhs.mVAO, 0);
   mVBOs        = std::exchange(rhs.mVBOs, std::array<unsigned int, 3>());
   mEBO         = std::exchange(rhs.mEBO, 0);
   return *this;
}

// TODO: Experiment with GL_STATIC_DRAW, GL_STREAM_DRAW and GL_DYNAMIC_DRAW to see which is faster
void AnimatedMesh::LoadBuffers()
{
   glBindVertexArray(mVAO);

   // Load the mesh's data into the buffers

   // Positions
   glBindBuffer(GL_ARRAY_BUFFER, mVBOs[VBOTypes::positions]);
   glBufferData(GL_ARRAY_BUFFER, mPositions.size() * sizeof(glm::vec3), &mPositions[0], GL_STATIC_DRAW);
   // Normals
   glBindBuffer(GL_ARRAY_BUFFER, mVBOs[VBOTypes::normals]);
   glBufferData(GL_ARRAY_BUFFER, mNormals.size() * sizeof(glm::vec3), &mNormals[0], GL_STATIC_DRAW);
   // Texture coordinates
   glBindBuffer(GL_ARRAY_BUFFER, mVBOs[VBOTypes::texCoords]);
   glBufferData(GL_ARRAY_BUFFER, mTexCoords.size() * sizeof(glm::vec2), &mTexCoords[0], GL_STATIC_DRAW);

   glBindBuffer(GL_ARRAY_BUFFER, 0);

   // Indices
   glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mEBO);
   glBufferData(GL_ELEMENT_ARRAY_BUFFER, mIndices.size() * sizeof(unsigned int), &mIndices[0], GL_STATIC_DRAW);

   // Unbind the VAO first, then the EBO
   glBindVertexArray(0);
   glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

   mNumVertices = static_cast<unsigned int>(mPositions.size());
   mNumIndices = static_cast<unsigned int>(mIndices.size());
}

void AnimatedMesh::ClearMeshData()
{
   // This data has already been passed to the GPU, so it's not necessary to store it anymore
   mPositions.clear();
   mNormals.clear();
   mTexCoords.clear();
   mIndices.clear();
}

void AnimatedMesh::ConfigureVAO(int posAttribLocation,
                                int normalAttribLocation,
                                int texCoordsAttribLocation)
{
   glBindVertexArray(mVAO);

   // Set the vertex attribute pointers
   BindFloatAttribute(posAttribLocation,       mVBOs[VBOTypes::positions], 3);
   BindFloatAttribute(normalAttribLocation,    mVBOs[VBOTypes::normals], 3);
   BindFloatAttribute(texCoordsAttribLocation, mVBOs[VBOTypes::texCoords], 2);

   glBindVertexArray(0);
}

void AnimatedMesh::UnconfigureVAO(int posAttribLocation,
                                  int normalAttribLocation,
                                  int texCoordsAttribLocation)
{
   glBindVertexArray(mVAO);

   // Unset the vertex attribute pointers
   UnbindAttribute(posAttribLocation,        mVBOs[VBOTypes::positions]);
   UnbindAttribute(normalAttribLocation,     mVBOs[VBOTypes::normals]);
   UnbindAttribute(texCoordsAttribLocation,  mVBOs[VBOTypes::texCoords]);

   glBindVertexArray(0);
}

void AnimatedMesh::BindFloatAttribute(int attribLocation, unsigned int VBO, int numComponents)
{
   if (attribLocation >= 0)
   {
      glBindBuffer(GL_ARRAY_BUFFER, VBO);
      glEnableVertexAttribArray(attribLocation);
      glVertexAttribPointer(attribLocation, numComponents, GL_FLOAT, GL_FALSE, 0, (void*)0);
      glBindBuffer(GL_ARRAY_BUFFER, 0);
   }
}

void AnimatedMesh::BindIntAttribute(int attribLocation, unsigned int VBO, int numComponents)
{
   if (attribLocation >= 0)
   {
      glBindBuffer(GL_ARRAY_BUFFER, VBO);
      glEnableVertexAttribArray(attribLocation);
      glVertexAttribIPointer(attribLocation, numComponents, GL_INT, 0, (void*)0);
      glBindBuffer(GL_ARRAY_BUFFER, 0);
   }
}

void AnimatedMesh::UnbindAttribute(int attribLocation, unsigned int VBO)
{
   if (attribLocation >= 0)
   {
      glBindBuffer(GL_ARRAY_BUFFER, VBO);
      glDisableVertexAttribArray(attribLocation);
      glBindBuffer(GL_ARRAY_BUFFER, 0);
   }
}

// TODO: GL_TRIANGLES shouldn't be hardcoded here
//       Can we load that from the GLTF file?
void AnimatedMesh::Render()
{
   glBindVertexArray(mVAO);

   if (mNumIndices > 0)
   {
      glDrawElements(GL_TRIANGLES, mNumIndices, GL_UNSIGNED_INT, 0);
   }
   else
   {
      glDrawArrays(GL_TRIANGLES, 0, mNumVertices);
   }

   glBindVertexArray(0);
}

// TODO: GL_TRIANGLES shouldn't be hardcoded here
//       Can we load that from the GLTF file?
void AnimatedMesh::RenderInstanced(unsigned int numInstances)
{
   glBindVertexArray(mVAO);

   if (mNumIndices > 0)
   {
      glDrawElementsInstanced(GL_TRIANGLES, mNumIndices, GL_UNSIGNED_INT, 0, numInstances);
   }
   else
   {
      glDrawArraysInstanced(GL_TRIANGLES, 0, mNumVertices, numInstances);
   }

   glBindVertexArray(0);
}
