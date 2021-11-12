#include "Texture.h"
#define STB_IMAGE_IMPLEMENTATION
#define STBI_FAILURE_USERMSG
#include "stb_image.h"

TextureLoader::TextureLoader(const std::string& path) : textureId(0)
{
  stbi_set_flip_vertically_on_load(true);
  image = stbi_load(path.c_str(), &width, &height, &depth, 4);
  depth = 4;
  printf("%d %d %d %s\n", depth, width, height, path.c_str());
  if (!image) {
    printf("\nRead error on file %s:\n  %s\n\n", path.c_str(), stbi_failure_reason());
    exit(-1);
  }

  // Here we create MIPMAP and set some useful modes for the texture
  glGenTextures(1, &textureId);   // Get an integer id for this texture from OpenGL
  glBindTexture(GL_TEXTURE_2D, textureId);
  glTexImage2D(GL_TEXTURE_2D, 0, (GLint)GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 10);
  glGenerateMipmap(GL_TEXTURE_2D);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, (int)GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, (int)GL_LINEAR_MIPMAP_LINEAR);
  glBindTexture(GL_TEXTURE_2D, 0);
  stbi_image_free(image);

}

// Make a texture availabe to a shader program.  The unit parameter is
// a small integer specifying which texture unit should load the
// texture.  The name parameter is the sampler2d in the shader program
// which will provide access to the texture.
void TextureLoader::Bind(const int unit, const int programId, const std::string& name)
{
  glBindTextureUnit(unit, textureId);
  int loc = glGetUniformLocation(programId, name.c_str());
  glUniform1i(loc, unit);
}

// Unbind a texture from a texture unit whne no longer needed.
void TextureLoader::Unbind()
{
  glBindTexture(GL_TEXTURE_2D, 0);
}

glm::vec3 TextureLoader::GetTexel(float u, float v)
{
  int i = int(v * height) * width * depth + int(u * width) * depth;
  return glm::vec3(image[i] / 127.0, image[i + 1] / 127.0, image[i + 2] / 127.0);
}
