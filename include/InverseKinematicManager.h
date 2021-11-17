#pragma once
#include "ManagerBase.h"

class InverseKinematicManager : public ManagerBase<InverseKinematicManager>
{
public:
  InverseKinematicManager() = default;
  ~InverseKinematicManager() override = default;

  void Setup() override;
  void Update() override;
  
private:
};