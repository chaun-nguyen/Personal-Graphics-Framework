#pragma once
#include "Shape.h"

class Sphere : public Shape
{
public:
  Sphere(const int n);

  bool intersect(const Ray& ray, Intersection& intersection) override;
  BoundingVolume* bbox() override;
};