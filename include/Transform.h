#pragma once
#include "LibHeader.h"
#include <fstream>

// Factory functions to create specific transformations, multiply two, and invert one.
glm::mat4 Rotate(const int i, const float theta);
glm::mat4 Scale(const float x, const float y, const float z);
glm::mat4 Translate(const float x, const float y, const float z);
glm::mat4 Perspective(const float rx, const float ry,
  const float front, const float back);
glm::mat4 LookAt(const glm::vec3 Eye, const glm::vec3 Center, const glm::vec3 Up);
glm::mat4 PerspectiveFov(const float fov, const float aspect, const float front, const float back);

float* Pntr(glm::mat4& m);