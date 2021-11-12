#pragma once
#include "LibHeader.h"
#include "ManagerBase.h"

class CameraManager : public ManagerBase<CameraManager>
{
public:
  CameraManager() = default;
  ~CameraManager() override = default;

  void Setup() override;
  void Update() override;
  
  glm::mat4 WorldProj, WorldView, WorldInverse;
  glm::vec3 lookAtVector;
  glm::vec3 backVector;
  glm::vec3 relativeUpVector;
  glm::vec3 upVector;
  glm::vec3 rightVector;
  float ry, front, back, speed, aspectRatio, fov;
  float yaw, pitch; // yaw(around y), pitch(around x)
  bool nav;
  glm::vec3 eye, tr;
  int width, height;
  int key;

private:
  void BuildMatrix();
  void Movement();
};