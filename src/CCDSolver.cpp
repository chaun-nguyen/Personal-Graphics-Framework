#include "CCDSolver.h"
#include "Engine.h"
#include "LibHeader.h"
#include "Transform.h"

CCDSolver::CCDSolver()
{
  numSteps = 15;
  threshHold = 0.00001f;
}

IKData& CCDSolver::operator[](unsigned index)
{
  return IKChain[index];
}

IKData& CCDSolver::getWorldTransform(unsigned index)
{
  return IKChain[index];
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
  unsigned size = IKChain.size();
  if (size == 0)
    return false;
  unsigned last = size - 1;
  
  Pd = worldLocationGoal;
  Pc = getWorldTransform(last).worldPosition;

  int counter = 0;
  do
  {
    Pv = Pc;

    for (int j = (int)size - 2; j >= 0; --j)
    {
      IKData& world = getWorldTransform(j);
      glm::vec3 Vck = Pc - world.worldPosition; // from jk to Pc
      glm::vec3 Vdk = Pd - world.worldPosition; // from jk to Pd

      Vck = glm::normalize(Vck);
      Vdk = glm::normalize(Vdk);

      // find angle between Vck and Vdk
      float alpha = glm::angle(Vck, Vdk);

      // compute the rotation axis
      glm::vec3 axis = glm::normalize(glm::cross(Vck, Vdk));

      // build a quaternion that rotation from Vck to Vdk
      glm::quat rotation = glm::angleAxis(alpha, axis);
      world.worldRotation = Quaternion(rotation.w, rotation.x, rotation.y, rotation.z);//world.worldRotation.rotate(Vck, Vdk);

      // apply transformation hierarchically through the IK chain
      ApplyTransformHierarchically(j, world.worldRotation.toMat4());

      // update Pc after applying transformation
      Pc = getWorldTransform(last).worldPosition;

      if (glm::length(Pd - Pc) < threshHold)
      {
        return true;
      }
    }
    if (counter > 200)
      break;
    ++counter;
  } while (glm::length(Pc - Pv) > threshHold);

  return true;
}

void CCDSolver::ApplyTransformHierarchically(int startIndex, glm::mat4 M)
{
  glm::vec3 fPInChain = IKChain[startIndex].worldPosition;
  // rotation with respect to the first parent in the current chain
  glm::mat4 transformation = Translate(fPInChain.x, fPInChain.y, fPInChain.z) * M * Translate(-fPInChain.x, -fPInChain.y, -fPInChain.z);

  for (int i = startIndex + 1; i < IKChain.size(); ++i)
  {
    IKChain[i].worldPosition = glm::vec3(transformation * glm::vec4(IKChain[i].worldPosition, 1.f));
  }
}
