#pragma once
#include "ManagerBase.h"

class PhysicsManager : public ManagerBase<PhysicsManager>
{
public:
  PhysicsManager() = default;
  ~PhysicsManager() override = default;

  void Setup() override;
  void Update() override;
  
private:
};