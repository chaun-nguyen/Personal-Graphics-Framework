#pragma once
#include "LibHeader.h"
#include "ManagerBase.h"

class WindowManager : public ManagerBase<WindowManager>
{
public:
  WindowManager() = default;
  ~WindowManager() override;

  void SetSize(int width, int height);
  void SetTitle(const char* title);
  int GetWidth();
  int GetHeight();

  void Setup() override;
  void Update() override;

  int ShouldClose();

  GLFWwindow* GetHandle();

  glm::vec2 resolution_;
private:
  GLFWwindow* window_ = nullptr;

  int w_ = 0;
  int h_ = 0;

  char* title_ = nullptr;

  void SetUpGLEW();
  void HardwareInfo();
};
