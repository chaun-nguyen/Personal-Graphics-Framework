#pragma once

class FBO
{
public:
  unsigned int fboID;
  unsigned int textureID;
  unsigned int textureUnit;
  int width, height;  // Size of the texture.

  FBO() = default;
  ~FBO();

  void SetUp(int w, int h, unsigned int texUnit);
  void Bind();
  void Unbind();
  void generateMipMap();
private:
};