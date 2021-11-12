#pragma once
#include "Shape.h"

class Plane : public Shape
{
public:
  Plane(const float range, const int n);

  bool intersect(const Ray& ray, Intersection& intersection) override;
  BoundingVolume* bbox() override;
};