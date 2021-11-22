#include "CCDSolver.h"

CCDSolver::CCDSolver()
{
  numSteps = 15;
  threshHold = 0.00001f;
}

IKData& CCDSolver::operator[](unsigned index)
{
  return IKChain[index];
}

glm::vec3 CCDSolver::getWorldLocation(unsigned index)
{
  return glm::vec3();
}

unsigned CCDSolver::getSize()
{
  return IKChain.size();
}

void CCDSolver::Resize(unsigned newSize)
{
  IKChain.resize(newSize);
}

// make sure bones are read hierarchically
void CCDSolver::AddBoneToChain(IKData newBone)
{
  IKChain.push_back(newBone);
}

std::vector<IKData>& CCDSolver::getChain()
{
  return IKChain;
}

unsigned CCDSolver::getNumSteps()
{
  return numSteps;
}

void CCDSolver::setNumSteps(unsigned newNumSteps)
{
  numSteps = newNumSteps;
}

float CCDSolver::getThreshHold()
{
  return threshHold;
}

void CCDSolver::setThreshHold(float value)
{
  threshHold = value;
}

bool CCDSolver::Solve(glm::vec3& worldLocationGoal)
{
  return true;
}
