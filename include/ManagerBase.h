#pragma once

class WindowManager;

class Base
{
public:
  Base() = default;
  virtual ~Base() = default;
  virtual void Setup() = 0;
  virtual void Update() = 0;
};

template<class T>
class ManagerBase : public Base
{
public:
  ManagerBase() = default;
  ~ManagerBase() override = default;
  void Setup() override {}
  void Update() override {}
};

template<>
class ManagerBase<WindowManager> : public Base
{
public:
  ManagerBase() = default;
  ~ManagerBase() override = default;
  void Setup() override {}
  void Update() override {}
};