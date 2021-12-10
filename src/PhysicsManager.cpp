#include "PhysicsManager.h"
#include "Engine.h"
#include <iostream>
#include "Physics.h"
#include "Transform.h"
#include <glm/glm/gtx/string_cast.hpp>
#include <glm/glm/ext/quaternion_trigonometric.hpp>

PhysicsManager::~PhysicsManager()
{
  glDeleteBuffers(1, &springVAO);
  glDeleteBuffers(1, &springVBO);
  glDeleteBuffers(1, &springEBO);
}

void PhysicsManager::Setup()
{
  auto* om = Engine::managers_.GetManager<ObjectManager*>();
  size = om->SpringMassDamperGeometry_.size();

  // go from 1 to size
  qA_.resize(size);
  qA_local_.resize(size);
  vA_.resize(size);

  // go from 0 to size - 1
  qB_.resize(size - 1);
  qB_local_.resize(size - 1);
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

  d = {
    // damper coefficient (good range from 0.4 to 0.7)
    //0.4f, 0.7f, 0.55f, 0.65f, 0.45f, 0.5f
    0.9f, 0.9f, 0.9f, 0.9f, 0.9f, 0.9f,0.9f,0.9f
    //100.f,100.f,100.f,100.f,100.f,100.f
  };

  k = {
    // spring coefficient
    65.f, 50.f, 30.f, 45.f, 50.f, 60.f, 35.f, 55.f
    //20.f, 30.f, 10.f, 15.f, 25.f, 35.f
    //1000.f,1000.f,1000.f,1000.f,1000.f,1000.f
  };
  g = 500.f; // gravity constant
  speed = 5000.f;

  // only go through sticks, ignore anchor points
  for (int i = 1; i < size - 1; ++i)
  {
    ComputeExternalForce(i);
  }

  // prepare data to draw
  PopulateDrawData();

  // set up VAO draw
  SetUpVAO();
}

void PhysicsManager::Update()
{
  if (key == GLFW_KEY_0 || key == GLFW_KEY_9 || key == GLFW_KEY_1 || key == GLFW_KEY_2)
  {
    Movement();
  }
  else
  {
    vB_.front().derivedVelocity = { 0.f,0.f,0.f };
    vA_.back().derivedVelocity = { 0.f,0.f,0.f };
  }
  double dt = Engine::managers_.GetManager<FrameRateManager*>()->delta_time;
  if (simulateFlag)
  {
    DynamicSimulation(static_cast<float>(1.f/60.f));
    UpdateVBO();
  }
}

void PhysicsManager::RK4thOrderIntegrationPosition(float dt, int index)
{
  Derivative a, b, c, d;

  // Finds 4 derivatives as dividing step by 4
  a = EvaluateDerivative(0.0f, Derivative(), index, IntegrationType::Position);
  b = EvaluateDerivative(dt * 0.5f, a, index, IntegrationType::Position);
  c = EvaluateDerivative(dt * 0.5f, b, index, IntegrationType::Position);
  d = EvaluateDerivative(dt, c, index, IntegrationType::Position);

  // Finds weighted sum of derivatives for each:
  // Position from velocity
  glm::vec3 dpdt = 1.0f / 6.0f * (a.derivedVelocity + 2.0f * (b.derivedVelocity + c.derivedVelocity) + d.derivedVelocity);

  // Velocity from acceleration
  glm::vec3 dvdt = 1.0f / 6.0f *
    (a.derivedAcceleration + 2.0f * (b.derivedAcceleration + c.derivedAcceleration) + d.derivedAcceleration);

  // Change position and velocity accordingly
  CurrPosition += dpdt * dt;
  CurrLinearVelocity += dvdt * dt;
}

void PhysicsManager::RK4thOrderIntegrationRotation(float dt, int index)
{
  Derivative a, b, c, d;

  // Finds 4 derivatives as dividing step by 4
  a = EvaluateDerivative(0.0f, Derivative(), index, IntegrationType::Rotation);
  b = EvaluateDerivative(dt * 0.5f, a, index, IntegrationType::Rotation);
  c = EvaluateDerivative(dt * 0.5f, b, index, IntegrationType::Rotation);
  d = EvaluateDerivative(dt, c, index, IntegrationType::Rotation);

  // Finds weighted sum of derivatives for each:
  // Position from velocity
  glm::vec3 drdt = 1.0f / 6.0f * (a.derivedVelocity + 2.0f * (b.derivedVelocity + c.derivedVelocity) + d.derivedVelocity);

  // Velocity from acceleration
  glm::vec3 dvdt = 1.0f / 6.0f *
    (a.derivedAcceleration + 2.0f * (b.derivedAcceleration + c.derivedAcceleration) + d.derivedAcceleration);

  // Change position and velocity accordingly
  glm::mat3 tildeOmega = TildeMatrix(drdt);
  glm::mat3 R_dot_mat = tildeOmega * CurrRotation.toMat3();

  glm::quat temp_R_dot = glm::quat_cast(R_dot_mat);

  Quaternion R_dot = { temp_R_dot.w,temp_R_dot.x,temp_R_dot.y,temp_R_dot.z };

  Quaternion next_q = CurrRotation + R_dot * dt;
  next_q = next_q.normalize();

  CurrRotation = next_q;
  CurrAngularVelocity += dvdt * dt;
}

void PhysicsManager::RK4thOrderIntegrationLinearMomentum(float dt, int index)
{
  Derivative a, b, c, d;

  // Finds 4 derivatives as dividing step by 4
  a = EvaluateDerivative(0.0f, Derivative(), index, IntegrationType::LinearMomentum);
  b = EvaluateDerivative(dt * 0.5f, a, index, IntegrationType::LinearMomentum);
  c = EvaluateDerivative(dt * 0.5f, b, index, IntegrationType::LinearMomentum);
  d = EvaluateDerivative(dt, c, index, IntegrationType::LinearMomentum);

  // Finds weighted sum of derivatives for each:
  // Position from velocity
  glm::vec3 dfdt = 1.0f / 6.0f * (a.derivedVelocity + 2.0f * (b.derivedVelocity + c.derivedVelocity) + d.derivedVelocity);

  // Velocity from acceleration
  glm::vec3 dvdt = 1.0f / 6.0f *
    (a.derivedAcceleration + 2.0f * (b.derivedAcceleration + c.derivedAcceleration) + d.derivedAcceleration);

  P_[index] += dfdt * dt;
  CurrChangeInLinearMomentum += dvdt * dt;
}

void PhysicsManager::RK4thOrderIntegrationAngularMomentum(float dt, int index)
{
  Derivative a, b, c, d;

  // Finds 4 derivatives as dividing step by 4
  a = EvaluateDerivative(0.0f, Derivative(), index, IntegrationType::AngularMomentum);
  b = EvaluateDerivative(dt * 0.5f, a, index, IntegrationType::AngularMomentum);
  c = EvaluateDerivative(dt * 0.5f, b, index, IntegrationType::AngularMomentum);
  d = EvaluateDerivative(dt, c, index, IntegrationType::AngularMomentum);

  // Finds weighted sum of derivatives for each:
  // Position from velocity
  glm::vec3 dTdt = 1.0f / 6.0f * (a.derivedVelocity + 2.0f * (b.derivedVelocity + c.derivedVelocity) + d.derivedVelocity);

  // Velocity from acceleration
  glm::vec3 dvdt = 1.0f / 6.0f *
    (a.derivedAcceleration + 2.0f * (b.derivedAcceleration + c.derivedAcceleration) + d.derivedAcceleration);

  L_[index] += dTdt * dt;
  CurrChangeInAngularMomentum += dvdt * dt;
}

Derivative PhysicsManager::EvaluateDerivative(float dt, const Derivative& d, int index, IntegrationType type)
{
  Derivative output;

  auto* om = Engine::managers_.GetManager<ObjectManager*>();
  // get physic component
  Physics* physic = om->SpringMassDamperGeometry_[index]->physics;

  glm::vec3 velocityTemp;
  
  switch (type)
  {
  case IntegrationType::Position:
    velocityTemp = CurrLinearVelocity;
    velocityTemp += d.derivedAcceleration * dt;
    output.derivedVelocity = velocityTemp;
    output.derivedAcceleration = F_[index] / physic->getTotalMass();
    break;
  case IntegrationType::Rotation:
    velocityTemp = CurrAngularVelocity;
    velocityTemp += d.derivedAcceleration * dt;
    output.derivedVelocity = velocityTemp;
    output.derivedAcceleration = T_[index] * physic->getInvInertiaTensorMatrix();
    break;
  case IntegrationType::LinearMomentum:
    velocityTemp = CurrChangeInLinearMomentum;
    velocityTemp += d.derivedAcceleration * dt;
    output.derivedVelocity = velocityTemp;
    output.derivedAcceleration = glm::vec3(0.f);
    break;
  case IntegrationType::AngularMomentum:
    velocityTemp = CurrChangeInAngularMomentum;
    velocityTemp += d.derivedAcceleration * dt;
    output.derivedVelocity = velocityTemp;
    output.derivedAcceleration = glm::vec3(0.f);
    break;
  }
  
  return output;
}

void PhysicsManager::Movement()
{
  auto* om = Engine::managers_.GetManager<ObjectManager*>();
  auto* fr = Engine::managers_.GetManager<FrameRateManager*>();
  float dist = speed * static_cast<float>(fr->delta_time);
  glm::vec3 leftAnchorPointPos = om->SpringMassDamperGeometry_.front()->GetPosition();
  glm::vec3 rightAnchorPointPos = om->SpringMassDamperGeometry_.back()->GetPosition();

  switch (key)
  {
  case GLFW_KEY_1:
    leftAnchorPointPos.y -= dist;
    om->SpringMassDamperGeometry_.front()->SetPosition(leftAnchorPointPos);
    qB_.front() = leftAnchorPointPos;
    vB_.front().derivedVelocity = { 0.f,-speed,0.f };
    break;
  case GLFW_KEY_2:
    leftAnchorPointPos.y += dist;
    om->SpringMassDamperGeometry_.front()->SetPosition(leftAnchorPointPos);
    qB_.front() = leftAnchorPointPos;
    vB_.front().derivedVelocity = { 0.f,speed,0.f };
    break;
  case GLFW_KEY_0:
    rightAnchorPointPos.y += dist;
    om->SpringMassDamperGeometry_.back()->SetPosition(rightAnchorPointPos);
    qA_.back() = rightAnchorPointPos;
    vA_.back().derivedVelocity = { 0.f,speed,0.f };
    break;
  case GLFW_KEY_9:
    rightAnchorPointPos.y -= dist;
    om->SpringMassDamperGeometry_.back()->SetPosition(rightAnchorPointPos);
    qA_.back() = rightAnchorPointPos;
    vA_.back().derivedVelocity = { 0.f,-speed,0.f };
    break;
  }
}

void PhysicsManager::ComputeExternalForce(int i)
{
  auto* om = Engine::managers_.GetManager<ObjectManager*>();

  // get physic component
  Physics* physic = om->SpringMassDamperGeometry_[i]->physics;

  glm::vec3 relativeVelocityA = vB_[i - 1].derivedVelocity - vA_[i].derivedVelocity;
  glm::vec3 relativeVelocityB = vA_[i + 1].derivedVelocity - vB_[i].derivedVelocity;

  // total linear force exerted on point A
  fA_[i] =
    k[i - 1] * (qB_[i - 1] - qA_[i]) +
    0.5f * physic->getTotalMass() * g * verticalVector +
    d[i - 1] * relativeVelocityA;

  // total linear force exerted on point B
  fB_[i] =
    k[i] * (qA_[i + 1] - qB_[i]) +
    0.5f * physic->getTotalMass() * g * verticalVector +
    d[i] * relativeVelocityB;

  // total linear force exerted on stick
  F_[i] = fA_[i] + fB_[i];

    // center of mass world space
  glm::vec3 c = om->SpringMassDamperGeometry_[i]->GetPosition();

  // total angular force exerted on point A
  tA_[i] = glm::cross(glm::normalize(qA_[i] - c), fA_[i]);

  // total angular force exerted on point B
  tB_[i] = glm::cross(glm::normalize(qB_[i] - c), fB_[i]);

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
    glm::mat3 I_inv = physic->getInvInertiaTensorMatrix(); // inverse inertia tensor matrix

    // total force === change in linear momentum
    CurrChangeInLinearMomentum = F_[i];

    // RK4 integration method
    RK4thOrderIntegrationLinearMomentum(dt, i);

    // total torque === change in angular momentum
    CurrChangeInAngularMomentum = T_[i];

    // RK4 integration method
    RK4thOrderIntegrationAngularMomentum(dt, i);

    // linear velocity
    CurrLinearVelocity = P_[i] * physic->getInvTotalMass();

    // angular velocity
    CurrAngularVelocity = I_inv * L_[i];

    // the orientation of the stick
    CurrRotation = om->SpringMassDamperGeometry_[i]->GetOrientation();

    // the location of the mass center
    CurrPosition = om->SpringMassDamperGeometry_[i]->GetPosition();

    // RK4 integration method
    RK4thOrderIntegrationRotation(dt, i);

    // update new rotation (orientation)
    om->SpringMassDamperGeometry_[i]->SetRotation(CurrRotation);

    // RK4 integration method
    RK4thOrderIntegrationPosition(dt, i);

    // update new position (center of mass)
    om->SpringMassDamperGeometry_[i]->SetPosition(CurrPosition);
    
    glm::vec3 scale = om->SpringMassDamperGeometry_[i]->GetScale();
    // Update qB points
    qB_[i] = CurrRotation.toMat3()* scale * qB_local_[i] + CurrPosition;

    // Update qA points
    qA_[i] = CurrRotation.toMat3() * scale * qA_local_[i] + CurrPosition;

    // update velocity at A and B
    vA_[i].derivedVelocity = TildeMatrix(CurrAngularVelocity) * CurrRotation.toMat3() * scale * qA_local_[i] + CurrLinearVelocity;
    vB_[i].derivedVelocity = TildeMatrix(CurrAngularVelocity) * CurrRotation.toMat3() * scale * qB_local_[i] + CurrLinearVelocity;

    // update draw data
    PopulateDrawData();

    // update the inverse inertia tensor
    I_inv = CurrRotation.toMat3() * I_inv * glm::transpose(CurrRotation.toMat3());
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
      qB_[i] = worldPosition + (worldSize);
      qB_local_[i] = glm::vec3(glm::inverse(om->SpringMassDamperGeometry_[i]->modelTr) * glm::vec4(qB_[i], 1.f));
    }
    else // anchor points
    {
      qB_[i] = om->SpringMassDamperGeometry_[i]->GetPosition(); // world position
      qB_local_[i] = glm::vec3(glm::inverse(om->SpringMassDamperGeometry_[i]->modelTr) * glm::vec4(qB_[i], 1.f));
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
      qA_[i] = worldPosition - (worldSize);
      qA_local_[i] = glm::vec3(glm::inverse(om->SpringMassDamperGeometry_[i]->modelTr) * glm::vec4(qA_[i], 1.f));
    }
    else // anchor points
    {
      qA_[i] = om->SpringMassDamperGeometry_[i]->GetPosition(); // world position
      qA_local_[i] = glm::vec3(glm::inverse(om->SpringMassDamperGeometry_[i]->modelTr) * glm::vec4(qA_[i], 1.f));
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

void PhysicsManager::Draw(ShaderProgram* shader)
{
  CHECKERROR;
  int loc = glGetUniformLocation(shader->programID, "color");
  glUniform3fv(loc, 1, glm::value_ptr(glm::vec3(0.f, 1.f, 0.f)));
  
  glm::mat4 modelTr(1.f);
  
  loc = glGetUniformLocation(shader->programID, "ModelTr");
  glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(modelTr));
  CHECKERROR;
  
  glBindVertexArray(springVAO);
  CHECKERROR;
  glDrawElements(GL_LINES, static_cast<GLsizei>(springIndices.size()), GL_UNSIGNED_INT, 0);
  CHECKERROR;
  glBindVertexArray(0);
  CHECKERROR;
}

glm::vec3 PhysicsManager::getLeftAnchorPointPosition()
{
  return Engine::managers_.GetManager<ObjectManager*>()->SpringMassDamperGeometry_.front()->GetPosition();
}

glm::vec3 PhysicsManager::getRightAnchorPointPosition()
{
  return Engine::managers_.GetManager<ObjectManager*>()->SpringMassDamperGeometry_.back()->GetPosition();
}

void PhysicsManager::setLeftAnchorPointPosition(glm::vec3 newPos)
{
  Engine::managers_.GetManager<ObjectManager*>()->SpringMassDamperGeometry_.front()->SetPosition(newPos);
  qB_.front() = newPos;
}

void PhysicsManager::setRightAnchorPointPosition(glm::vec3 newPos)
{
  Engine::managers_.GetManager<ObjectManager*>()->SpringMassDamperGeometry_.back()->SetPosition(newPos);
  qA_.back() = newPos;
}

float PhysicsManager::getSpringConstants(int index)
{
  return k[index];
}

float PhysicsManager::getDampingConstants(int index)
{
  return d[index];
}

void PhysicsManager::setSpringConstants(int index, float newK)
{
  k[index] = newK;
}

void PhysicsManager::setDampingConstants(int index, float newD)
{
  d[index] = newD;
}

float PhysicsManager::getTotalMass(int index)
{
  return Engine::managers_.GetManager<ObjectManager*>()->SpringMassDamperGeometry_[index]->physics->getTotalMass();
}

void PhysicsManager::setTotalMass(int index, float newMass)
{
  Engine::managers_.GetManager<ObjectManager*>()->SpringMassDamperGeometry_[index]->physics->setTotalMass(newMass);
}

float PhysicsManager::getGravity()
{
  return g;
}

void PhysicsManager::setGravity(float newG)
{
  g = newG;
}

void PhysicsManager::PopulateDrawData()
{
  springIndices.clear();
  springPosition.clear();
  for (int i = 0; i < size - 1; ++i)
  {
    springIndices.push_back(springPosition.size());
    springPosition.push_back(qB_[i]);
    springIndices.push_back(springPosition.size());
    springPosition.push_back(qA_[i + 1]);
  }
}

void PhysicsManager::SetUpVAO()
{
  CHECKERROR;
  glCreateVertexArrays(1, &springVAO);
  CHECKERROR;
  glCreateBuffers(1, &springVBO);
  CHECKERROR;
  glCreateBuffers(1, &springEBO);
  CHECKERROR;

  glNamedBufferStorage(springVBO, springPosition.size() * sizeof(glm::vec3), nullptr, GL_DYNAMIC_STORAGE_BIT);
  CHECKERROR;

  glNamedBufferSubData(springVBO, 0, springPosition.size() * sizeof(glm::vec3), springPosition.data());
  CHECKERROR;

  // spring position
  glEnableVertexArrayAttrib(springVAO, 0);
  glVertexArrayVertexBuffer(springVAO, 0, springVBO, 0, sizeof(glm::vec3));
  glVertexArrayAttribFormat(springVAO, 0, 3, GL_FLOAT, GL_FALSE, 0);
  glVertexArrayAttribBinding(springVAO, 0, 0);
  CHECKERROR;

  // spring indices
  glNamedBufferStorage(springEBO, springIndices.size() * sizeof(unsigned int), springIndices.data(), GL_DYNAMIC_STORAGE_BIT);
  glVertexArrayElementBuffer(springVAO, springEBO);
  CHECKERROR;
}

void PhysicsManager::UpdateVBO()
{
  CHECKERROR;
  glInvalidateBufferData(springVBO);
  CHECKERROR;
  glNamedBufferSubData(springVBO, 0, springPosition.size() * sizeof(glm::vec3),
    springPosition.data());
  CHECKERROR;
}
