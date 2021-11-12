#pragma once
#include "ManagerBase.h"
#include "LibHeader.h"

class InputManager : public ManagerBase<InputManager>
{
public:
  InputManager() = default;
  ~InputManager() override = default;

  void Setup() override;
  void Update() override;

  double mouseX = 0.0;
  double mouseY = 0.0;
  bool leftDown = false;
  bool rightDown = false;
  bool middleDown = false;

  bool glfw_used_flag = false;

private:
  void MouseMotion(GLFWwindow* window, double x, double y);
  void MouseButton(GLFWwindow* window, int button, int action, int mods);
  void Keyboard(GLFWwindow* window, int key, int scancode, int action, int mods);  

  using MouseButtonCallBackFn = void (InputManager::*)(GLFWwindow*, int, int, int);
  using MouseButtonCallBackHelper = CCallbackHelper<InputManager, MouseButtonCallBackFn, &InputManager::MouseButton, void, GLFWwindow*, int, int, int>;

  using MouseMotionCallBackFn = void (InputManager::*)(GLFWwindow*, double, double);
  using MouseMotionCallBackHelper = CCallbackHelper<InputManager, MouseMotionCallBackFn, &InputManager::MouseMotion, void, GLFWwindow*, double, double>;

  using KeyboardCallBackFn = void (InputManager::*)(GLFWwindow*, int, int, int, int);
  using KeyboardCallBackHelper = CCallbackHelper<InputManager, KeyboardCallBackFn, &InputManager::Keyboard, void, GLFWwindow*, int, int, int, int>;
};