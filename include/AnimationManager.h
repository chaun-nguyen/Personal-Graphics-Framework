#pragma once
#include "ManagerBase.h"
#include "SkeletalAnimation.h"
#include "Animator.h"

#include <memory>

class AnimationManager : public ManagerBase<AnimationManager>
{
public:
  AnimationManager() = default;
  ~AnimationManager() override = default;

  void Setup() override;
  void Update() override;
  void DrawBone(ShaderProgram* shaderProgram);

  std::unique_ptr<SkeletalAnimation> animation;
  std::unique_ptr<Animator> animator;

  bool PlayAnimation = false;
private:
};
