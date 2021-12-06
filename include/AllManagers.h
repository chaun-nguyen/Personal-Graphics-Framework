#pragma once
#include <vector>
#include <typeinfo>
#include <stdexcept>
#include "magic_enum.hpp"

#include "WindowManager.h"
#include "FrameRateManager.h"
#include "InputManager.h"
#include "CameraManager.h"
#include "DeserializeManager.h"
#include "ObjectManager.h"
#include "PhysicsManager.h"
#include "AnimationManager.h"
#include "SplineManager.h"
#include "InverseKinematicManager.h"

#include "RenderManager.h"
#include "ImGuiUIManager.h"


enum class ManagerOrder
{
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
  ImGuiUIManager,

  Total
};

template<class Base, class ... Features>
class AllManagers
{
public:
  AllManagers();
  ~AllManagers();

  template<class T>
  auto GetManager() -> std::enable_if_t<(std::is_base_of_v<Base, Features>&& ...), T>
  {
    std::string name = std::string(typeid(T).name());
    auto& OrderNames = magic_enum::enum_names<ManagerOrder>();
    for (unsigned i = 0; i < magic_enum::enum_count<ManagerOrder>(); ++i)
    {
      if (name.find(OrderNames[i]) != std::string::npos)
      {
        return dynamic_cast<T>(container_.at(static_cast<unsigned>(magic_enum::enum_cast<ManagerOrder>(OrderNames[i]).value())));
      }
    }

    throw std::runtime_error("Cant find correct manager");
  }

  size_t GetContainerSize();

private:
  std::vector<Base*> container_;
  static_assert((std::is_base_of_v<Base, Features>&& ...), "must be same base");
};

template <class Base, class ... Features>
AllManagers<Base, Features...>::AllManagers()
{
  container_.reserve(static_cast<unsigned>(ManagerOrder::Total));
  (container_.push_back(static_cast<Base*>(new Features)), ...);
}

template <class Base, class ... Features>
AllManagers<Base, Features...>::~AllManagers()
{
  for (auto& c : container_)
  {
    delete c;
  }
}

template <class Base, class ... Features>
size_t AllManagers<Base, Features...>::GetContainerSize()
{
  return container_.size();
}
