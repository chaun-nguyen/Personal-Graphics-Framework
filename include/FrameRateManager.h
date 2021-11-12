#pragma once
#include "ManagerBase.h"

class FrameRateManager : public ManagerBase<FrameRateManager>
{
public:
  FrameRateManager() = default;
  ~FrameRateManager() override = default;

  void Setup() override;
  void Update() override;

  void UpdateWindowTitle();

  

  double elapsed_time = 0.0;
  double curr_time = 0.0;
  double delta_time = 0.0;
  double fps = 0.0;
  double fps_calc_interval = 1.0;

private:
  void Timer();
  bool click = true;
};