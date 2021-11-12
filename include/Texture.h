#pragma once
#include "LibHeader.h"
#include <string>

class TextureLoader
{
public:
  unsigned int textureId;
  int width, height, depth;
  unsigned char* image;
  TextureLoader(const std::string& filename);

  void Bind(const int unit, const int programId, const std::string& name);
  void Unbind();
  glm::vec3 GetTexel(float u, float v);
};