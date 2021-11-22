#include "InverseKinematicManager.h"
#include "Object.h"
#include "Box.h"
#include "Texture.h"
#include "Engine.h"
#include <iostream>
#include <numeric>

#include "Transform.h"

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
  {
    StartIK();
    UpdateVBO();
  }
}

void InverseKinematicManager::DrawIKChain(ShaderProgram* shaderProgram)
{
  CHECKERROR;
  int loc = glGetUniformLocation(shaderProgram->programID, "color");
  glUniform3fv(loc, 1, glm::value_ptr(glm::vec3(1.f, 0.f, 1.f)));

  // world space
  glm::mat4 modelTr =
    Translate(IKChainWorldLocation.x, IKChainWorldLocation.y, IKChainWorldLocation.z) * Scale(5.f, 5.f, 5.f);

  modelTr *= IKChainOrientationMatrix;

  loc = glGetUniformLocation(shaderProgram->programID, "ModelTr");
  glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(modelTr));
  CHECKERROR;

  glBindVertexArray(IKChainVAO);
  CHECKERROR;
  glDrawElements(GL_LINE_STRIP, static_cast<GLsizei>(IKChainIndices.size()), GL_UNSIGNED_INT, 0);
  CHECKERROR;
  glBindVertexArray(0);
  CHECKERROR;
}

void InverseKinematicManager::SetBoneWorldPosition(glm::vec3 pos)
{
  IKChainWorldLocation = pos;
}

void InverseKinematicManager::SetBoneOrientation(glm::mat4& mat)
{
  IKChainOrientationMatrix = mat;
}

void InverseKinematicManager::UpdateVBO()
{
  // update position of bone when model is animating
  CHECKERROR;
  auto* am = Engine::managers_.GetManager<AnimationManager*>();
  auto& boneInfoMap = am->animation->GetBoneIDMap();
  auto& ikChain = m_CCDSolver.getChain();
  
  // get each bone local position
  // and layout continuously
  for (int i = 0; i < IKChainPosition.size(); ++i)
  {
    // local-space position
    IKChainPosition[i] =
      glm::vec3(am->animator->m_PreOffSetMatrices[boneInfoMap[ikChain[i].name].id] *
        glm::vec4(ikChain[i].localPosition, 1.f));
  }
  CHECKERROR;
  glInvalidateBufferData(IKChainVBO);
  CHECKERROR;
  glNamedBufferSubData(IKChainVBO, 0, IKChainPosition.size() * sizeof(glm::vec3),
    IKChainPosition.data());
  CHECKERROR;
}

void InverseKinematicManager::SetUpVAO()
{
  // populate indices
  IKChainIndices.resize(IKChainPosition.size());
  std::iota(IKChainIndices.begin(), IKChainIndices.begin() + IKChainIndices.size(), 0);

  CHECKERROR;
  glCreateVertexArrays(1, &IKChainVAO);
  CHECKERROR;
  glCreateBuffers(1, &IKChainVBO);
  CHECKERROR;
  glCreateBuffers(1, &IKChainEBO);
  CHECKERROR;
  glNamedBufferStorage(IKChainVBO,
    IKChainPosition.size() * sizeof(glm::vec3),
    nullptr, GL_DYNAMIC_STORAGE_BIT);
  CHECKERROR;

  glNamedBufferSubData(IKChainVBO,
    0,
    IKChainPosition.size() * sizeof(glm::vec3),
    IKChainPosition.data());
  CHECKERROR;

  // bone position
  glEnableVertexArrayAttrib(IKChainVAO, 0);
  glVertexArrayVertexBuffer(IKChainVAO, 0, IKChainVBO, 0, sizeof(glm::vec3));
  glVertexArrayAttribFormat(IKChainVAO, 0, 3, GL_FLOAT, GL_FALSE, 0);
  glVertexArrayAttribBinding(IKChainVAO, 0, 0);

  CHECKERROR;
  // setting up indices
  glNamedBufferStorage(IKChainEBO, IKChainIndices.size() * sizeof(unsigned int), IKChainIndices.data(), GL_DYNAMIC_STORAGE_BIT);
  glVertexArrayElementBuffer(IKChainVAO, IKChainEBO);
  CHECKERROR;
}

void InverseKinematicManager::Movement()
{
  auto* fr = Engine::managers_.GetManager<FrameRateManager*>();
  float dist = speed * static_cast<float>(fr->delta_time);
  glm::vec3 currPosition = Goal->GetPosition();

  switch (key)
  {
  case GLFW_KEY_UP:
    currPosition.z -= dist;
    Goal->SetPosition(currPosition);
    break;
  case GLFW_KEY_DOWN:
    currPosition.z += dist;
    Goal->SetPosition(currPosition);
    break;
  case GLFW_KEY_RIGHT:
    currPosition.x += dist;
    Goal->SetPosition(currPosition);
    break;
  case GLFW_KEY_LEFT:
    currPosition.x -= dist;
    Goal->SetPosition(currPosition);
    break;
  }
}

void InverseKinematicManager::UpdatePath()
{
  auto& path = Engine::managers_.GetManager<SplineManager*>()->GetCurve(SpaceCurveIndex);
  auto& controlPts = path.GetControlPoints();
  glm::vec3 pos = Goal->GetPosition() / 500.f;
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
    glm::vec3 lastControlPts = Goal->GetPosition() / 500.f;
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
