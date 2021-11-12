#include "Box.h"

void Box::face(const glm::mat4x4 tr)
{
  int n = static_cast<int>(Pnt.size());

  float verts[8] = { 1.0f,1.0f, -1.0f,1.0f, -1.0f,-1.0f, 1.0f,-1.0f };
  float texcd[8] = { 1.0f,1.0f,  0.0f,1.0f,  0.0f, 0.0f, 1.0f, 0.0f };

  // Four vertices to make a single face, with its own normal and
  // texture coordinates.
  for (int i = 0; i < 8; i += 2) {
    Pnt.push_back(tr * glm::vec4(verts[i], verts[i + 1], 1.0f, 1.0f));
    Nrm.push_back(glm::vec3(tr * glm::vec4(0.0f, 0.0f, 1.0f, 0.0f)));
    Tex.push_back(glm::vec2(texcd[i], texcd[i + 1]));
    Tan.push_back(glm::vec3(tr * glm::vec4(1.0f, 0.0f, 0.0f, 0.0f)));
  }

  pushquad(Tri, n, n + 1, n + 2, n + 3);
}

Box::Box()
{
  diffuseColor = glm::vec3(0.5, 0.5, 1.0);
  specularColor = glm::vec3(1.0, 1.0, 1.0);
  shininess = 10.0f;

  glm::mat4 I(1.0f);

  // Six faces, each a rotation of a rectangle placed on the z axis.
  face(I);
  float r90 = PI / 2;
  face(glm::rotate(I, r90, glm::vec3(1.0f, 0.0f, 0.0f)));
  face(glm::rotate(I, -r90, glm::vec3(1.0f, 0.0f, 0.0f)));
  face(glm::rotate(I, r90, glm::vec3(0.0f, 1.0f, 0.0f)));
  face(glm::rotate(I, -r90, glm::vec3(0.0f, 1.0f, 0.0f)));
  face(glm::rotate(I, PI, glm::vec3(1.0f, 0.0f, 0.0f)));

  ComputeSize();
  MakeVAO();
}

bool Box::intersect(const Ray& ray, Intersection& intersection)
{
  return false;
}

BoundingVolume* Box::bbox()
{
  return nullptr;
}
