#include "InverseKinematicManager.h"
#include "Object.h"
#include "Box.h"
#include "Texture.h"
#include "Engine.h"
#include <iostream>

void InverseKinematicManager::Setup()
{
  speed = 3000.0f;
}

void InverseKinematicManager::Update()
{
  if (key == GLFW_KEY_UP || key == GLFW_KEY_DOWN || key == GLFW_KEY_RIGHT || key == GLFW_KEY_LEFT)
  {
    Movement();
  }
}

void InverseKinematicManager::Movement()
{
  auto* fr = Engine::managers_.GetManager<FrameRateManager*>();
  float dist = speed * static_cast<float>(fr->delta_time);
  glm::vec3 currPosition = EndEffector->GetPosition();

  switch (key)
  {
  case GLFW_KEY_UP:
    currPosition.z -= dist;
    EndEffector->SetPosition(currPosition);
    break;
  case GLFW_KEY_DOWN:
    currPosition.z += dist;
    EndEffector->SetPosition(currPosition);
    break;
  case GLFW_KEY_RIGHT:
    currPosition.x += dist;
    EndEffector->SetPosition(currPosition);
    break;
  case GLFW_KEY_LEFT:
    currPosition.x -= dist;
    EndEffector->SetPosition(currPosition);
    break;
  }
}
