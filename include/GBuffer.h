#pragma once
#include "LibHeader.h"
#include <array>

enum class Layout
{
  Albedo,
  Normal,
  Position,
  Depth,
  Specular,

  Total
};

class GBuffer
{
public:
  GBuffer() = default;
  ~GBuffer();

  void SetUp(int w, int h);

  GLuint GetGBufferID();

  GLuint GetTextureUnit(Layout type);
  GLuint GetTextureID(Layout type);

  void GenerateMipmap();

  void Bind();
  void UnBind();

  GLsizei width;
  GLsizei height;
private:
  GLuint ID;
  GLuint depthBuffer;
  struct Info
  {
    GLuint textureID;
    GLuint textureUnit;
  };

  std::array<Info, to_integral(Layout::Total)> GBufferInfo;
  void SetUpLayout(GLsizei width, GLsizei height, Layout layout, GLint internal_format, GLint format,
    GLenum type, GLuint textureUnit);
};