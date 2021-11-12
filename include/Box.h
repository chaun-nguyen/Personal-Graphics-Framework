#pragma once
#include "LibHeader.h"
#include "Shape.h"

class Box : public Shape
{
  void face(const glm::mat4x4 tr);
public:
  Box();

  bool intersect(const Ray& ray, Intersection& intersection) override;
  BoundingVolume* bbox() override;
};