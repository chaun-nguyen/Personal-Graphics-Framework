#pragma once
#include "Bone.h"

struct IKData
{
  std::string name;
  glm::vec3 worldPosition;
  glm::vec3 localPosition;
  Quaternion worldRotation;
  glm::mat4 Transformation = glm::mat4(1.f);
  unsigned index;
};

class CCDSolver
{
public:
  CCDSolver();

  IKData& operator[](unsigned index);
  IKData& getWorldTransform(unsigned index);

  unsigned getSize();
  void Resize(unsigned newSize);
  // make sure bones are read hierarchically
  void AddBoneToChain(IKData newBone);
  std::vector<IKData>& getChain();

  std::vector<std::vector<IKData>>& getIntermediatePosition();

  unsigned getNumSteps();
  void setNumSteps(unsigned newNumSteps);

  float getThreshHold();
  void setThreshHold(float value);

  bool Solve(glm::vec3& worldLocationGoal);

  void ApplyTransformHierarchically(int startIndex, glm::mat4 M);

  void ApplyHingeConstraint(int index, glm::vec3 axis);

private:
  std::vector<IKData> IKChain;
  std::vector<std::vector<IKData>> intermediatePosition;
  unsigned numSteps;
  float threshHold;
  glm::vec3 Pd;
  glm::vec3 Pc;
  glm::vec3 Pv;
};