#ifndef GLTF_LOADER_H
#define GLTF_LOADER_H

#include "cgltf/cgltf.h"

#include "AnimatedMesh.h"

cgltf_data*               LoadGLTFFile(const char* path);
void                      FreeGLTFFile(cgltf_data* handle);

std::vector<AnimatedMesh> LoadStaticMeshes(cgltf_data* data);

#endif
