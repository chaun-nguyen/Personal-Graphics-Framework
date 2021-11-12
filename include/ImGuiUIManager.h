#pragma once
#include "ManagerBase.h"

#include "../imgui/imgui.h"
#include "../imgui/imgui_impl_glfw.h"
#include "../imgui/imgui_impl_opengl3.h"

#include "../imgui/implot.h"

#include <string>
#include <vector>

class ImGuiUIManager : public ManagerBase<ImGuiUIManager>
{
public:
  ImGuiUIManager() = default;
  ~ImGuiUIManager() override;

  void Setup() override;
  void Update() override;

  std::vector<std::string> loadedItems;
private:
  void NewFrame();
  void SetStyle();
  void ShowDockSpace(bool* p_open);
  const char* items[2];
};