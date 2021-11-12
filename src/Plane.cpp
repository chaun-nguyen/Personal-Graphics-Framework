#include "Plane.h"

Plane::Plane(const float r, const int n)
{
  diffuseColor = glm::vec3(0.756, 0.756, 0.756);
  specularColor = glm::vec3(1.0, 1.0, 1.0);
  shininess = 10.0;

  for (int i = 0; i <= n; i++) {
    float s = i / float(n);
    for (int j = 0; j <= n; j++) {
      float t = j / float(n);
      Pnt.push_back(glm::vec4(s * 2.0 * r - r, 0.0, t * 2.0 * r - r, 1.0));
      Nrm.push_back(glm::vec3(0.0, 1.0, 0.0));
      Tex.push_back(glm::vec2(s, t));
      Tan.push_back(glm::vec3(1.0, 0.0, 0.0));
      if (i > 0 && j > 0) {
        pushquad(Tri, (i - 1) * (n + 1) + (j - 1),
          (i - 1) * (n + 1) + (j),
          (i) * (n + 1) + (j),
          (i) * (n + 1) + (j - 1));
      }
    }
  }
  ComputeSize();
  MakeVAO();
}

bool Plane::intersect(const Ray& ray, Intersection& intersection)
{
  return false;
}

BoundingVolume* Plane::bbox()
{
  BoundingVolume* bv = new BV_AABB(minP, maxP, parent, { 1.0f,0.f,0.f });
  return bv;
}
