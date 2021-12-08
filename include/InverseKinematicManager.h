#pragma once
#include "ManagerBase.h"
#include "CCDSolver.h"

class Object;
class ShaderProgram;
struct NodeData;

class InverseKinematicManager : public ManagerBase<InverseKinematicManager>
{
public:
  InverseKinematicManager() = default;
  ~InverseKinematicManager() override;

  void Setup() override;
  void Update() override;

  void DrawIKChain(ShaderProgram* shaderProgram);

  Object* Goal = nullptr;
  int key;
  int SpaceCurveIndex;
  bool runFlag;
  bool updateFlag;
  CCDSolver m_CCDSolver;

  // bones' world location (motion along a space curve)
  void SetBoneWorldPosition(glm::vec3 pos);
  // bones' orientation along a space curve
  void SetBoneOrientation(glm::mat4& mat);

  glm::mat4& getWorldMatrix();

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
  void AnimateIK();
  float step;
  unsigned keyFrame;
  unsigned jointIndex;

  void ApplyTransformationHierarchy(const NodeData* node,
    std::vector<std::vector<IKData>>& intermediateValue,
    unsigned keyFrame,
    float step,
    std::vector<glm::mat4>& preOffSetMatrices, glm::mat4 parentTransform, bool enterIKChain);
  bool isInsideIKChain(const std::string& name, int& index);
};