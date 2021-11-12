#include "GBuffer.h"
#include <numeric>
#include <cassert>

void GBuffer::SetUp(int w, int h)
{
  width = w;
  height = h;
  glCreateFramebuffers(1, &ID);
  glBindFramebuffer(GL_FRAMEBUFFER, ID);

  SetUpLayout(width, height, Layout::Position, GL_RGB16F, GL_RGB, GL_FLOAT, 1 + to_integral(Layout::Position));
  SetUpLayout(width, height, Layout::Normal, GL_RGB16F, GL_RGB, GL_FLOAT, 1 + to_integral(Layout::Normal));
  SetUpLayout(width, height, Layout::Albedo, GL_RGBA16F, GL_RGBA, GL_FLOAT, 1 + to_integral(Layout::Albedo));
  SetUpLayout(width, height, Layout::Depth, GL_RGB, GL_RGB, GL_UNSIGNED_BYTE, 1 + to_integral(Layout::Depth));
  SetUpLayout(width, height, Layout::Specular, GL_RGB16F, GL_RGB, GL_FLOAT, 1 + to_integral(Layout::Specular));

  std::array<unsigned int, to_integral(Layout::Total)> attachments;
  std::iota(attachments.begin(), attachments.end(), GL_COLOR_ATTACHMENT0);

  glNamedFramebufferDrawBuffers(ID, to_integral(Layout::Total), attachments.data());

  glCreateRenderbuffers(1, &depthBuffer);
  glNamedRenderbufferStorage(depthBuffer, GL_DEPTH24_STENCIL8, width, height);
  glNamedFramebufferRenderbuffer(ID, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, depthBuffer);

  assert(glCheckNamedFramebufferStatus(ID, GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

GBuffer::~GBuffer()
{
  for (auto& g : GBufferInfo)
  {
    glDeleteTextures(1, &g.textureID);
  }
  glDeleteRenderbuffers(1, &depthBuffer);
  glDeleteFramebuffers(1, &ID);
}

GLuint GBuffer::GetGBufferID()
{
  return ID;
}

GLuint GBuffer::GetTextureUnit(Layout type)
{
  return GBufferInfo[to_integral(type)].textureUnit;
}

GLuint GBuffer::GetTextureID(Layout type)
{
  return GBufferInfo[to_integral(type)].textureID;
}

void GBuffer::GenerateMipmap()
{
  for (auto& info : GBufferInfo)
  {
    glGenerateTextureMipmap(info.textureID);
  }
}

void GBuffer::Bind()
{
  glBindFramebuffer(GL_DRAW_FRAMEBUFFER, ID);
}

void GBuffer::UnBind()
{
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void GBuffer::SetUpLayout(GLsizei width, GLsizei height, Layout layout, GLint internal_format, GLint format, GLenum type, GLuint textureUnit)
{

  glCreateTextures(GL_TEXTURE_2D, 1, &GBufferInfo[to_integral(layout)].textureID);

  glBindTextureUnit(textureUnit, GBufferInfo[to_integral(layout)].textureID);
  // saved texture unit
  GBufferInfo[to_integral(layout)].textureUnit = textureUnit;

  glBindTexture(GL_TEXTURE_2D, GBufferInfo[to_integral(layout)].textureID);

  glTexImage2D(GL_TEXTURE_2D, 0, internal_format, width, height, 0, format, type, nullptr);

  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + to_integral(layout), GL_TEXTURE_2D, GBufferInfo[to_integral(layout)].textureID, 0);

  glTextureParameteri(GBufferInfo[to_integral(layout)].textureID, GL_TEXTURE_MAX_LEVEL, 10);

  glTextureParameteri(GBufferInfo[to_integral(layout)].textureID, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
  glTextureParameteri(GBufferInfo[to_integral(layout)].textureID, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
  glTextureParameteri(GBufferInfo[to_integral(layout)].textureID, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
  glTextureParameteri(GBufferInfo[to_integral(layout)].textureID, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  glBindTexture(GL_TEXTURE_2D, 0);
}
