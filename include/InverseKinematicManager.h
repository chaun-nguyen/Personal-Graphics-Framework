#pragma once
#include "ManagerBase.h"

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
private:
  int speed;
  void Movement();
};