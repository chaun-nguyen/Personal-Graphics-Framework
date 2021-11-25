#include "CCDSolver.h"
#include "Engine.h"
#include "LibHeader.h"
#include "Transform.h"
#include <glm/glm/ext/vector_angle.hpp>

CCDSolver::CCDSolver()
{
  numSteps = 5;
  threshHold = 0.00001f;
  applyConstraint = true;
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

std::vector<std::vector<IKData>>& CCDSolver::getIntermediateValue()
{
  return intermediateValue;
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
  // get IK size
  unsigned size = IKChain.size();

  // if IK chain is empty then do nothing
  if (size == 0)
    return false;

  // get end effector index
  unsigned last = size - 1;

  // make sure everything is in same space (world space)
  // get goal position
  Pd = worldLocationGoal;

  // get current position of end effector
  Pc = getWorldTransform(last).worldPosition;

  int counter = 0;
  do
  {
    // update previous position
    Pv = Pc;

    // minus 2 because we want to skip the end effector
    for (int j = (int)size - 2; j >= 0; --j)
    {
      // get transformation info
      IKData& world = getWorldTransform(j);
      glm::vec3 Vck = Pc - world.worldPosition; // from jk to Pc
      glm::vec3 Vdk = Pd - world.worldPosition; // from jk to Pd

      // normalize vectors
      Vck = glm::normalize(Vck);
      Vdk = glm::normalize(Vdk);

      // build a quaternion that rotation from Vck to Vdk
      world.worldRotation = QUATERNION::rotate(Vck, Vdk);
      world.worldRotation = world.worldRotation.normalize();

      // find angle between Vck and Vdk
      float alpha = world.worldRotation.angleRadian();

      // remove small angle
      if (alpha < 0.1f)
        continue;

      // want to apply constraint?
      if (applyConstraint)
      {
        // apply hinge constraint to elbow joint to prevent bending outwardly which is unrealistic
        if (world.name == "LeftForeArm")
          ApplyHingeConstraint(j, world.worldRotation.getAxis());

        // apply hinge constraint to finger joints to prevent bending outwardly which is unrealistic
        if (world.name == "LeftHandIndex1" || world.name == "LeftHandIndex2" || world.name == "LeftHandIndex3")
          ApplyHingeConstraint(j, world.worldRotation.getAxis());

        // apply ball-and-socket constraint to shoulder joint
        if (world.name == "LeftArm")
          ApplyBallSocketConstraint(j, 2.617993878f); // 150 degree

        // apply ball-and-socket constraint to hand joint
        if (world.name == "LeftHand")
          ApplyBallSocketConstraint(j, 0.5235987756f); // 30 degree
      }

      // apply transformation hierarchically through the IK chain
      ApplyTransformHierarchically(j, world.worldRotation.toMat4());

      // update Pc after applying transformation
      Pc = getWorldTransform(last).worldPosition;

      // close enough
      if (glm::length(Pd - Pc) < threshHold + std::numeric_limits<float>::epsilon())
      {
        return true;
      }
    }
    // only allow a certain steps
    if (counter > numSteps)
      break;
    ++counter;
  } while (glm::length(Pc - Pv) > threshHold + std::numeric_limits<float>::epsilon());

  return true;
}

void CCDSolver::ApplyTransformHierarchically(int startIndex, glm::mat4 M)
{
  // get most parent joint
  glm::vec3 fPInChain = IKChain[startIndex].worldPosition;
  // rotation with respect to the first parent in the current chain
  glm::mat4 transformation = Translate(fPInChain.x, fPInChain.y, fPInChain.z) * M * Translate(-fPInChain.x, -fPInChain.y, -fPInChain.z);

  // hierarchically
  for (int i = startIndex + 1; i < IKChain.size(); ++i)
  {
    // concatenate matrix to build the final transformation matrix
    IKChain[i].Transformation = transformation * IKChain[i].Transformation;
    IKChain[i].worldPosition = glm::vec3(transformation * glm::vec4(IKChain[i].worldPosition, 1.f));
  }
  // gather intermediate value
  intermediateValue.push_back(IKChain);
}

// implementation of hinge constraint
// From Hands-On C++ Game Animation Programming Book by Gabor Szauer
void CCDSolver::ApplyHingeConstraint(int index, glm::vec3 axis)
{
  // get current joint
  IKData& joint = getWorldTransform(index);

  // get parent of current joint
  IKData& parent = getWorldTransform(index + 1);

  // get current and destination hinge
  glm::vec3 currHinge = joint.worldRotation * axis;
  glm::vec3 desHinge = parent.worldRotation * axis;

  // rotate from curr to dest
  IKChain[index].worldRotation *= QUATERNION::rotate(currHinge, desHinge);
}

// implementation of ball-and-socket constraint
// From Hands-On C++ Game Animation Programming Book by Gabor Szauer
void CCDSolver::ApplyBallSocketConstraint(int index, float limit)
{
  // get current joint
  IKData& joint = getWorldTransform(index);

  // get parent of current joint
  IKData& parent = getWorldTransform(index + 1);

  glm::vec3 jointForwardDir = joint.worldRotation * glm::vec3(0, 0, 1);
  glm::vec3 parentForwardDir = parent.worldRotation * glm::vec3(0, 0, 1);

  float angle = glm::angle(jointForwardDir, parentForwardDir);

  // over angle limit
  if (angle > limit)
  {
    // correcting rotation
    glm::vec3 correction = glm::cross(parentForwardDir, jointForwardDir);
    Quaternion wRotation = parent.worldRotation * QUATERNION::angleAxis(limit, correction);
    IKChain[index].worldRotation = wRotation * parent.worldRotation.inverse();
  }
}
