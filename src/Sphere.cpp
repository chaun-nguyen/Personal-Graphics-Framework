#include "Sphere.h"
#include <algorithm>

Sphere::Sphere(const int n)
{
  diffuseColor = glm::vec3(0.9, 0.4, 0.8);
  specularColor = glm::vec3(1.0, 1.0, 1.0);
  shininess = 10.0;

  float d = 2.0f * PI / float(n * 2);
  for (int i = 0; i <= n * 2; i++)
  {
    float s = i * 2.0f * PI / float(n * 2);
    for (int j = 0; j <= n; j++)
    {
      float t = j * PI / float(n);
      float x = cos(s) * sin(t);
      float z = sin(s) * sin(t);
      float y = cos(t);
      Pnt.push_back(glm::vec4(x, y, z, 1.0f));
      Nrm.push_back(glm::vec3(x, y, z));
      Tex.push_back(glm::vec2(s / (2 * PI), t / PI));
      Tan.push_back(glm::vec3(-sin(s), cos(s), 0.0));
      if (i > 0 && j > 0)
      {
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

bool Sphere::intersect(const Ray& ray, Intersection& intersection)
{
  glm::vec3 Q_bar = ray.Q - center;
  float Q_bar_D = glm::dot(Q_bar, ray.D);
  float Q_bar_Q_bar = glm::dot(Q_bar, Q_bar);
  float discriminant = Q_bar_D * Q_bar_D - Q_bar_Q_bar + size * size;

  // no intersection
  if (discriminant < EPSILON)
  {
    return false;
  }

  discriminant = sqrt(discriminant);
  float t0 = -Q_bar_D + discriminant;
  float t1 = -Q_bar_D - discriminant;

  if (t0 < EPSILON && t1 < EPSILON)
  {
    return false;
  }

  intersection.t = std::min(t0, t1);
  intersection.P = ray.eval(intersection.t);
  intersection.N = glm::normalize(intersection.P - center);
  intersection.object = this;

  return true;
}

BoundingVolume* Sphere::bbox()
{
  return new BV_AABB(minP, maxP, parent, { 1.f,0.f,0.f });
}
