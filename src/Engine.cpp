#include "Engine.h"

AllManagers<
  Base,
WindowManager,
FrameRateManager,
InputManager,
CameraManager,
DeserializeManager,
ObjectManager,
PhysicsManager,
AnimationManager,
SplineManager,
InverseKinematicManager,
RenderManager,
ImGuiUIManager>Engine::managers_;

Engine::Engine()
{
  managers_.GetManager<WindowManager*>()->Setup();

  managers_.GetManager<FrameRateManager*>()->Setup();

  managers_.GetManager<InputManager*>()->Setup();

  managers_.GetManager<CameraManager*>()->Setup();

  managers_.GetManager<DeserializeManager*>()->Setup();

  managers_.GetManager<ObjectManager*>()->Setup();

  managers_.GetManager<PhysicsManager*>()->Setup();

  managers_.GetManager<AnimationManager*>()->Setup();

  managers_.GetManager<SplineManager*>()->Setup();

  managers_.GetManager<InverseKinematicManager*>()->Setup();

  managers_.GetManager<RenderManager*>()->Setup();

  managers_.GetManager<ImGuiUIManager*>()->Setup();
}

void Engine::Run()
{
  while(!managers_.GetManager<WindowManager*>()->ShouldClose())
  {
    Step();
  }
  glfwDestroyWindow(managers_.GetManager<WindowManager*>()->GetHandle());
}

void Engine::Step()
{
  managers_.GetManager<RenderManager*>()->BeginFrame();

  managers_.GetManager<WindowManager*>()->Update();

  managers_.GetManager<FrameRateManager*>()->Update();

  managers_.GetManager<InputManager*>()->Update();

  managers_.GetManager<CameraManager*>()->Update();

  managers_.GetManager<DeserializeManager*>()->Update();

  managers_.GetManager<ObjectManager*>()->Update();

  managers_.GetManager<PhysicsManager*>()->Update();

  managers_.GetManager<AnimationManager*>()->Update();

  managers_.GetManager<SplineManager*>()->Update();

  managers_.GetManager<InverseKinematicManager*>()->Update();

  managers_.GetManager<RenderManager*>()->Update();

  managers_.GetManager<ImGuiUIManager*>()->Update();

  managers_.GetManager<RenderManager*>()->EndFrame();
}
