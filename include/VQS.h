#pragma once
#include "Quaternion.h"

class VQS
{
public:
  VQS() = default;
  ~VQS() = default;

  VQS(glm::vec3 translation, Quaternion rotation, glm::vec3 scale);

  VQS& operator=(const VQS& rhs);

  VQS operator+(VQS rhs);
  VQS operator*(VQS rhs);
  glm::vec3 operator*(glm::vec3 r);
  VQS operator*(float scalar);

  VQS inverse();
  VQS identity();

private:
  glm::vec3 v;
  Quaternion q;
  glm::vec3 s;
};