#include "PhysicsManager.h"
#include "Engine.h"
#include <iostream>
#include "Physics.h"
#include "Transform.h"

void PhysicsManager::Setup()
{
  auto* om = Engine::managers_.GetManager<ObjectManager*>();
  size = om->SpringMassDamperGeometry_.size();

  // go from 1 to size
  qA_.resize(size);
  vA_.resize(size);

  // go from 0 to size - 1
  qB_.resize(size - 1);
  vB_.resize(size - 1);

  // populate qA points
  PopulateQA();
  // populate qB points
  PopulateQB();

  // linear force
  fA_.resize(size);
  fB_.resize(size);
  F_.resize(size);
  P_.resize(size); // linear momentum

  // angular force
  tA_.resize(size);
  tB_.resize(size);
  T_.resize(size);
  L_.resize(size); // angular momentum

  // only go through sticks, ignore anchor points
  for (int i = 1; i < size - 1; ++i)
  {
    ComputeExternalForce(i);
  }
}

void PhysicsManager::Update()
{
  double dt = Engine::managers_.GetManager<FrameRateManager*>()->delta_time;
  if (simulateFlag)
    DynamicSimulation(static_cast<float>(1.f / 60.f));
}

void PhysicsManager::ComputeExternalForce(int i)
{
  auto* om = Engine::managers_.GetManager<ObjectManager*>();

  // get physic component
  Physics* physic = om->SpringMassDamperGeometry_[i]->physics;

  // total linear force exerted on point A
  fA_[i] =
    k[i - 1] * (qB_[i - 1] - qA_[i]) +
    0.5f * physic->getTotalMass() * g * verticalVector +
    d[i - 1] * (vB_[i - 1].derivedVelocity - vA_[i].derivedVelocity);

  // total linear force exerted on point B
  fB_[i] =
    k[i] * (qA_[i + 1] - qB_[i]) +
    0.5f * physic->getTotalMass() * g * verticalVector +
    d[i] * (vA_[i + 1].derivedVelocity - vB_[i].derivedVelocity);

  // total linear force exerted on stick
  F_[i] = fA_[i] + fB_[i];

    // center of mass world space
  glm::vec3 c = om->SpringMassDamperGeometry_[i]->GetPosition();

  // total angular force exerted on point A
  tA_[i] = glm::cross(qA_[i] - c, fA_[i]);

  // total angular force exerted on point B
  tB_[i] = glm::cross(qB_[i] - c, fB_[i]);

  // total angular force exerted on stick
  T_[i] = tA_[i] + tB_[i];
}

void PhysicsManager::DynamicSimulation(float dt)
{
  auto* om = Engine::managers_.GetManager<ObjectManager*>();

  // only go through sticks, ignore anchor points
  for (int i = 1; i < size - 1; ++i)
  {
    Physics* physic = om->SpringMassDamperGeometry_[i]->physics;
    glm::mat3 I_inv = physic->getInvInertiaTensorMatrix();

    // need to apply numerical integration method s.t Runge-Kutta 2nd order
    P_[i] = /*P_[i] +*/ F_[i] * dt;
    L_[i] = /*L_[i] +*/ T_[i] * dt;

    // linear velocity
    glm::vec3 v = P_[i] * physic->getInvTotalMass();

    // angular velocity
    glm::vec3 omega = I_inv * L_[i];

    // the location of the mass center
    glm::vec3 prev_c = om->SpringMassDamperGeometry_[i]->GetPosition();
    glm::vec3 c = prev_c + v * dt;
    om->SpringMassDamperGeometry_[i]->SetPosition(c);

    // the orientation of the stick
    Quaternion q = om->SpringMassDamperGeometry_[i]->GetOrientation();
    
    glm::mat3 mq = q.toMat3();
    glm::mat3 tildeOmega = TildeMatrix(omega);
    glm::mat3 next_mq = mq + tildeOmega * (mq * dt);

    glm::quat testQ = glm::quat_cast(next_mq);
    Quaternion next_q = { testQ.w,testQ.x,testQ.y,testQ.z };
    //Quaternion next_q1 = QUATERNION::fromMat3(next_mq); // nan?????
    next_q = next_q.normalize();
    om->SpringMassDamperGeometry_[i]->SetRotation(next_q);

    // Update qB points
    qB_[i] -= prev_c;
    qB_[i] = next_q.toMat3() * qB_[i];
    qB_[i] += c;
    // Update qB points
    qA_[i] -= prev_c;
    qA_[i] = next_q.toMat3() * qA_[i];
    qA_[i] += c;

    // update the inverse inertia tensor
    I_inv = next_q.toMat3() * I_inv * glm::transpose(next_q.toMat3());
    //PrecisionCheck(I_inv[0]);
    //PrecisionCheck(I_inv[1]);
    //PrecisionCheck(I_inv[2]);
    physic->setInvInertiaTensorMatrix(I_inv);

    ComputeExternalForce(i);
  }
}

glm::mat3 PhysicsManager::TildeMatrix(glm::vec3& v)
{
  glm::mat3 result(0.f);

  result[0][1] = v.z;
  result[1][0] = -v.z;

  result[0][2] = -v.y;
  result[2][0] = v.y;

  result[1][2] = v.x;
  result[2][1] = -v.x;

  return result;
}

void PhysicsManager::PopulateQB()
{
  auto* om = Engine::managers_.GetManager<ObjectManager*>();
  // populate qB points
  for (int i = 0; i < size - 1; ++i)
  {
    Physics* physic = om->SpringMassDamperGeometry_[i]->physics;

    // sticks
    if (physic)
    {
      glm::vec3 worldPosition = om->SpringMassDamperGeometry_[i]->GetPosition(); // world position
      glm::vec3 worldSize = om->SpringMassDamperGeometry_[i]->GetScale(); // world size
      qB_[i] = worldPosition + (worldSize / 2.f);
    }
    else // anchor points
    {
      qB_[i] = om->SpringMassDamperGeometry_[i]->GetPosition(); // world position
    }
  }
}

void PhysicsManager::PopulateQA()
{
  auto* om = Engine::managers_.GetManager<ObjectManager*>();
  // populate qA points
  for (int i = 1; i < size; ++i)
  {
    Physics* physic = om->SpringMassDamperGeometry_[i]->physics;

    // sticks
    if (physic)
    {
      glm::vec3 worldPosition = om->SpringMassDamperGeometry_[i]->GetPosition(); // world position
      glm::vec3 worldSize = om->SpringMassDamperGeometry_[i]->GetScale(); // world size
      qA_[i] = worldPosition - (worldSize / 2.f);
    }
    else // anchor points
    {
      qA_[i] = om->SpringMassDamperGeometry_[i]->GetPosition(); // world position
    }
  }
}

void PhysicsManager::PrecisionCheck(glm::vec3& value)
{
  for (int i = 0; i < 3; ++i)
  {
    value[i] = value[i] < epsilon ? 0.f : value[i];
  }
}

