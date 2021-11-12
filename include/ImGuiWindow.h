#pragma once
#include <string>
#include "../imgui/imgui_impl_glfw.h"
#include "../imgui/imgui_impl_opengl3.h"

class ImGuiWindow
{
public:
  ImGuiWindow(std::string name, bool* show = 0);
  ~ImGuiWindow();

  void Update();
  void Show();

  void End();

  bool open_flag;
};

inline ImGuiWindow::ImGuiWindow(std::string name, bool* show)
{
  open_flag = ImGui::Begin(name.c_str(), show);
}

inline void ImGuiWindow::End()
{
  ImGui::End();
}
