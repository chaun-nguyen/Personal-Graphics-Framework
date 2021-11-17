#include "InverseKinematicManager.h"
#include "Object.h"
#include "Box.h"
#include "Texture.h"
#include "Engine.h"
#include <iostream>

void InverseKinematicManager::Setup()
{
  speed = 3000.0f;
  runFlag = false;
  t = 0.f;
}

void InverseKinematicManager::Update()
{
  if (key == GLFW_KEY_UP || key == GLFW_KEY_DOWN || key == GLFW_KEY_RIGHT || key == GLFW_KEY_LEFT)
  {
    Movement();
    UpdatePath();
  }

  if (runFlag)
    StartIK();
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

void InverseKinematicManager::UpdatePath()
{
  auto& path = Engine::managers_.GetManager<SplineManager*>()->GetCurve(SpaceCurveIndex);
  auto& controlPts = path.GetControlPoints();
  glm::vec3 pos = EndEffector->GetPosition() / 500.f;
  controlPts.back().x = pos.x; //> 0.f ? pos.x - 0.5f : pos.x + 0.5f;
  controlPts.back().z = pos.z; //> 0.f ? pos.z - 0.5f : pos.z + 0.5f;
  
  path.dirtyFlag = true;
}

void InverseKinematicManager::StartIK()
{
  auto* sm = Engine::managers_.GetManager<SplineManager*>();
  auto& curve = sm->GetCurve(SpaceCurveIndex);
  auto& models = Engine::managers_.GetManager<ObjectManager*>()->GetModels();
  Object* player = models[0];
  
  sm->MoveAlongSpaceCurve(player, curve, t);
  // step size
  t += Engine::managers_.GetManager<FrameRateManager*>()->delta_time / 10.f;
  
  
  if (t > 0.9f)
  {
    runFlag = false;
    auto& controlPts = curve.GetControlPoints();
    controlPts.clear();

    glm::vec3 firstcontrolPts = player->GetPosition() / 500.f;
    firstcontrolPts.y = 0.f;
    glm::vec3 lastControlPts = EndEffector->GetPosition() / 500.f;
    lastControlPts.y = 0.f;
    glm::vec3 middlePts = (firstcontrolPts + lastControlPts) / 2.f;
    middlePts.x += 1.f;

    controlPts.push_back(firstcontrolPts);
    controlPts.push_back(middlePts);
    controlPts.push_back(lastControlPts);

    t = 0.f;
  }
}

void InverseKinematicManager::CCDSolver()
{
}
