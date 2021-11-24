#include "InverseKinematicManager.h"
#include "Object.h"
#include "Box.h"
#include "Texture.h"
#include "Engine.h"
#include <iostream>
#include <numeric>

#include "Transform.h"
#include <glm/glm/gtx/matrix_decompose.hpp>

void InverseKinematicManager::Setup()
{
  IKChainModelMatrix = Translate(IKChainWorldLocation.x, IKChainWorldLocation.y, IKChainWorldLocation.z) * Scale(5.f, 5.f, 5.f);
  speed = 3000.0f;
  runFlag = false;
  updateFlag = false;
  t = 0.f;

  // animation data
  step = 0.f;
  keyFrame = 0;
  jointIndex = 0;
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
    UpdateVBO();
    StartIK();
  }

  if (updateFlag)
  {
    AnimateIK();
  }

  //std::cout << "Finished" << std::endl;
}

void InverseKinematicManager::DrawIKChain(ShaderProgram* shaderProgram)
{
  // draw Links
  CHECKERROR;
  int loc = glGetUniformLocation(shaderProgram->programID, "color");
  glUniform3fv(loc, 1, glm::value_ptr(glm::vec3(1.f, 0.f, 0.f)));

  loc = glGetUniformLocation(shaderProgram->programID, "ModelTr");
  glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(IKChainModelMatrix));
  CHECKERROR;

  glBindVertexArray(IKChainVAO);
  CHECKERROR;
  glDrawElements(GL_LINE_STRIP, static_cast<GLsizei>(IKChainIndices.size()), GL_UNSIGNED_INT, 0);
  CHECKERROR;
  glBindVertexArray(0);
  CHECKERROR;

  // draw Joints
  loc = glGetUniformLocation(shaderProgram->programID, "color");
  glUniform3fv(loc, 1, glm::value_ptr(glm::vec3(0.f, 0.f, 1.f)));
  // draw control points
  glBindVertexArray(IKChainVAO);
  CHECKERROR;
  glPointSize(3.f);
  glDrawElements(GL_POINTS, static_cast<GLsizei>(IKChainIndices.size()), GL_UNSIGNED_INT, 0);
  CHECKERROR;

  glBindVertexArray(0);
  CHECKERROR;
}

void InverseKinematicManager::SetBoneWorldPosition(glm::vec3 pos)
{
  IKChainWorldLocation = pos;
  IKChainModelMatrix = Translate(IKChainWorldLocation.x, IKChainWorldLocation.y, IKChainWorldLocation.z) * Scale(5.f, 5.f, 5.f);
}

void InverseKinematicManager::SetBoneOrientation(glm::mat4& mat)
{
  IKChainModelMatrix *= mat;
}

glm::mat4& InverseKinematicManager::getWorldMatrix()
{
  return IKChainModelMatrix;
}

void InverseKinematicManager::UpdateVBO()
{
  // update position of bone when model is animating
  CHECKERROR;
  auto* am = Engine::managers_.GetManager<AnimationManager*>();
  auto& boneInfoMap = am->animation->GetBoneIDMap();
  auto& ikChain = m_CCDSolver.getChain();
  auto offsetMatrices = am->animator->GetPreOffSetMatrices();

  // get each bone local position
  // and layout continuously
  for (int i = 0; i < ikChain.size(); ++i)
  {
    // m_PreOffSetMatrices get update every frame as player is animated
    // hierarchically calculate the world position of each joints inside IK chain
    glm::mat4 transformation = offsetMatrices[boneInfoMap[ikChain[i].name].id];

    // save world matrices
    //ikChain[i].Transformation = transformation;

    // update bone position inside IK chain for drawing purpose
    IKChainPosition[i] = glm::vec3(transformation * glm::vec4(ikChain[i].localPosition, 1.f));
    
    // saved world position for CCD algorithm solving IK
    ikChain[i].worldPosition = IKChainPosition[i];
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
  
  
  if (t > 0.825f)
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

    CCDSolver();
  }
}

void InverseKinematicManager::CCDSolver()
{
  // convert goal into correct space before using ccd solver
  glm::vec3 goal = Goal->GetPosition();
  glm::mat4 toLocalSpace = glm::inverse(IKChainModelMatrix);
  glm::vec3 goalLocalSpace = glm::vec3(toLocalSpace * glm::vec4(goal, 1.f));

  // clear buffer before going to the next solving iteration
  m_CCDSolver.getIntermediatePosition().clear();

  auto& ikChain = m_CCDSolver.getChain();
  for (int i = 0; i < ikChain.size(); ++i)
  {
    ikChain[i].Transformation = glm::mat4(1.0f); // reset to identity matrix for next ccd iteration
  }

  // ccd solver
  m_CCDSolver.Solve(goalLocalSpace);

  // update flag is used for animate the IK chain
  //updateFlag = true;

  auto* am = Engine::managers_.GetManager<AnimationManager*>();
  auto& boneInfoMap = am->animation->GetBoneIDMap();
  auto offsetMatrices = am->animator->GetPreOffSetMatrices();

  for (int i = 0; i < ikChain.size(); ++i)
  {
    // update transformation matrix for bones that are inside IK chain
    // calculate the final position of bones inside IK chain after running ccd
    IKChainPosition[i] = glm::vec3(ikChain[i].Transformation * 
      offsetMatrices[boneInfoMap[ikChain[i].name].id] *
      glm::vec4(ikChain[i].localPosition, 1.f));
  }
  
  CHECKERROR;
  glInvalidateBufferData(IKChainVBO);
  CHECKERROR;
  glNamedBufferSubData(IKChainVBO, 0, IKChainPosition.size() * sizeof(glm::vec3),
    IKChainPosition.data());
  CHECKERROR;

  // apply new transformation matrix hierarchically to the entire bone tree
  //ApplyTransformationHierarchy(&am->animation->GetRootNode(), glm::mat4(1.0f));

  //unsigned size = am->animation->bonePosition.size();
  //for (int i = 0; i < size; ++i)
  //{
  //  // local-space position
  //  am->animation->bonePosition[i] =
  //    glm::vec3(am->animator->m_PreOffSetMatrices[boneInfoMap[am->animation->boneName[i]].id] *
  //      glm::vec4(am->animation->boneLocalPosition[i], 1.f));
  //}
  //
  //CHECKERROR;
  //glInvalidateBufferData(am->animation->boneVBO);
  //CHECKERROR;
  //glNamedBufferSubData(am->animation->boneVBO, 0, am->animation->bonePosition.size() * sizeof(glm::vec3),
  //  am->animation->bonePosition.data());
  //CHECKERROR;
}

void InverseKinematicManager::AnimateIK()
{
  auto& intermediatePosition = m_CCDSolver.getIntermediatePosition();

  for (int i = 1; i < m_CCDSolver.getChain().size(); ++i)
  {
    IKChainPosition[i] = glm::mix(IKChainPosition[i], intermediatePosition[keyFrame][i].worldPosition, step);
  }

  // update data for drawing
  CHECKERROR;
  glInvalidateBufferData(IKChainVBO);
  CHECKERROR;
  glNamedBufferSubData(IKChainVBO, 0, IKChainPosition.size() * sizeof(glm::vec3),
    IKChainPosition.data());
  CHECKERROR;

  if (step >= 1.f)
  {
    if (keyFrame >= intermediatePosition.size() - 1)
    {
      // stop
      updateFlag = false;
      keyFrame = 0;
    }
    ++keyFrame;
    step = 0.f;
  }
  
  step += (1.f / 60.f);//Engine::managers_.GetManager<FrameRateManager*>()->delta_time;
}

void InverseKinematicManager::ApplyTransformationHierarchy(const NodeData* node, glm::mat4 parentTransform)
{
  std::string nodeName = node->name;

  glm::mat4 globalTransformation;

  if (isInsideIKChain(nodeName))
  {
    auto* am = Engine::managers_.GetManager<AnimationManager*>();
    auto& boneInfoMap = am->animation->GetBoneIDMap();

    int index = boneInfoMap[nodeName].id;

    globalTransformation = parentTransform * am->animator->GetPreOffSetMatrices()[index];
  }

  

  for (int i = 0; i < node->childrenCount; ++i)
  {
    ApplyTransformationHierarchy(&node->children[i], globalTransformation);
  }
}

bool InverseKinematicManager::isInsideIKChain(const std::string& name)
{
  auto& ikChain = m_CCDSolver.getChain();

  for (int i = 0; i < ikChain.size(); ++i)
  {
    if (ikChain[i].name == name)
      return true;
  }

  return false;
}

