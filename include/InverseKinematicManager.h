#pragma once
#include "ManagerBase.h"
#include "Spline.h"

class Object;

class InverseKinematicManager : public ManagerBase<InverseKinematicManager>
{
public:
  InverseKinematicManager() = default;
  ~InverseKinematicManager() override = default;

  void Setup() override;
  void Update() override;

  Object* EndEffector = nullptr;
  int key;
  int SpaceCurveIndex;
  bool runFlag;
private:
  int speed;
  float t;
  void Movement();
  void UpdatePath();
  void StartIK();
  void CCDSolver();
};