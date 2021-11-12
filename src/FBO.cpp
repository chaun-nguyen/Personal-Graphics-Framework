#include "FBO.h"
#include "LibHeader.h"

FBO::~FBO()
{
  glDeleteFramebuffers(1, &fboID);
}

void FBO::SetUp(int w, int h, unsigned texUnit)
{
  width = w;
  height = h;
  textureUnit = texUnit;
  CHECKERROR;

  glCreateFramebuffers(1, &fboID);
  glBindFramebuffer(GL_FRAMEBUFFER, fboID);
  CHECKERROR;
  glCreateTextures(GL_TEXTURE_2D, 1, &textureID);
  glBindTextureUnit(textureUnit, textureID);
  glBindTexture(GL_TEXTURE_2D, textureID);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width, height, 0, GL_RGBA, GL_FLOAT, nullptr);
  CHECKERROR;
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureID, 0);
  
  glTextureParameteri(textureID, GL_TEXTURE_MAX_LEVEL, 10);

  glTextureParameteri(textureID, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
  glTextureParameteri(textureID, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
  glTextureParameteri(textureID, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
  glTextureParameteri(textureID, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glBindTexture(GL_TEXTURE_2D, 0);
  CHECKERROR;
  
  unsigned int attachment[1] = { GL_COLOR_ATTACHMENT0 };
  glNamedFramebufferDrawBuffers(fboID, 1, attachment);
  CHECKERROR;

  unsigned depthBuffer;
  glCreateRenderbuffers(1, &depthBuffer);
  glNamedRenderbufferStorage(depthBuffer, GL_DEPTH24_STENCIL8, width, height);
  glNamedFramebufferRenderbuffer(fboID, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, depthBuffer);
  CHECKERROR;

  assert(glCheckNamedFramebufferStatus(fboID, GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  CHECKERROR;
}

void FBO::Bind()
{
  glBindFramebuffer(GL_FRAMEBUFFER, fboID);
}

void FBO::Unbind()
{
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void FBO::generateMipMap()
{
  glGenerateTextureMipmap(textureID);
}
