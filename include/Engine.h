#pragma once
#include "AllManagers.h"

class Engine
{
public:
  Engine();
  ~Engine() = default;

  void Run();
  void Step();

  static AllManagers<
    Base,
  WindowManager,
  FrameRateManager,
  InputManager,
  CameraManager,
  DeserializeManager,
  ObjectManager,
  AnimationManager,
  SplineManager,
  InverseKinematicManager,
  RenderManager,
  ImGuiUIManager> managers_;
};