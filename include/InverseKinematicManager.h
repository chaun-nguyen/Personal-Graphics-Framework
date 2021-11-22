#pragma once
#include "ManagerBase.h"
#include "CCDSolver.h"

class Object;
class ShaderProgram;

class InverseKinematicManager : public ManagerBase<InverseKinematicManager>
{
public:
  InverseKinematicManager() = default;
  ~InverseKinematicManager() override = default;

  void Setup() override;
  void Update() override;

  void DrawIKChain(ShaderProgram* shaderProgram);

  Object* Goal = nullptr;
  int key;
  int SpaceCurveIndex;
  bool runFlag;
  CCDSolver m_CCDSolver;

  // bones' world location (motion along a space curve)
  void SetBoneWorldPosition(glm::vec3 pos);
  // bones' orientation along a space curve
  void SetBoneOrientation(glm::mat4& mat);

  void UpdateVBO();
  // prepare data to draw IK chain
  void SetUpVAO();

  unsigned int IKChainVAO;
  unsigned int IKChainVBO;
  unsigned int IKChainEBO;

  std::vector<glm::vec3> IKChainPosition;
  std::vector<unsigned int> IKChainIndices;
private:
  // motion along a space curve
  glm::vec3 IKChainWorldLocation = { 0.f,0.f,0.f };
  glm::mat4 IKChainModelMatrix = glm::mat4(1.f);

  int speed;
  float t;
  void Movement();
  void UpdatePath();
  void StartIK();
  void CCDSolver();
};