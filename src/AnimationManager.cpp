#include <iostream>

#include "Engine.h"
#include "AnimationManager.h"

#include "Transform.h"

void AnimationManager::Setup()
{
  //std::cout << "Animation Manager Setup" << std::endl;
}

void AnimationManager::Update()
{
  if (PlayAnimation)
  {
    if (animation && animator)
    {
      float dt = Engine::managers_.GetManager<FrameRateManager*>()->delta_time;
      animator->UpdateAnimation(dt);
      animator->UpdateVBO();
    }
  }
}

void AnimationManager::DrawBone(ShaderProgram* shaderProgram)
{
  animation->DrawBone(shaderProgram);
}

