#pragma once
#include "Bone.h"

struct IKData
{
  std::string name;
  glm::vec3 worldPosition;
  glm::vec3 localPosition;
  Quaternion worldRotation;
};

class CCDSolver
{
public:
  CCDSolver();

  IKData& operator[](unsigned index);
  glm::vec3 getWorldLocation(unsigned index);

  unsigned getSize();
  void Resize(unsigned newSize);
  // make sure bones are read hierarchically
  void AddBoneToChain(IKData newBone);
  std::vector<IKData>& getChain();

  unsigned getNumSteps();
  void setNumSteps(unsigned newNumSteps);

  float getThreshHold();
  void setThreshHold(float value);

  bool Solve(glm::vec3& worldLocationGoal);

private:
  std::vector<IKData> IKChain;
  unsigned numSteps;
  float threshHold;
};