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

  // Verlet integration method
  PrevPosition.resize(size);
  for (int i = 0; i < size; ++i)
  {
    PrevPosition[i] = om->SpringMassDamperGeometry_[i]->GetPosition();
  }

  // go from 1 to size
  prev_qA_.resize(size);
  qA_.resize(size);
  qA_local_.resize(size);
  vA_.resize(size);

  // go from 0 to size - 1
  prev_qB_.resize(size - 1);
  qB_.resize(size - 1);
  qB_local_.resize(size - 1);
  vB_.resize(size - 1);

  // populate qA points
  PopulateQA();
  // populate qB points
  PopulateQB();

  for (int i = 1; i < size; ++i)
  {
    prev_qA_[i] = qA_[i];
  }

  for (int i = 0; i < size - 1; ++i)
  {
    prev_qB_[i] = qB_[i];
  }

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
    0.9f, 0.9f, 0.9f, 0.9f, 0.9f, 0.9f
  };

  k = {
    // spring coefficient
    //40.f, 50.f, 45.f, 65.f, 70.f, 35.f
    20.f, 30.f, 10.f, 15.f, 25.f, 35.f
  };
  g = 9.8f; // gravity constant
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

glm::vec3 PhysicsManager::VerletIntegrationPosition(float dt, int index)
{
  auto* om = Engine::managers_.GetManager<ObjectManager*>();
  // get physic component
  Physics* physic = om->SpringMassDamperGeometry_[index]->physics;

  glm::vec3 nextPosition = CurrPosition + (CurrPosition - PrevPosition[index]) + (F_[index] / physic->getTotalMass()) * dt * dt;
  PrevPosition[index] = CurrPosition;

  return nextPosition;
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
    glm::mat3 I_inv = physic->getInvInertiaTensorMatrix();

    // need to apply numerical integration method s.t Runge-Kutta 2nd order
    P_[i] = P_[i] + F_[i] * dt; // total linear force (i.e change in linear momentum)
    L_[i] = L_[i] + T_[i] * dt; // total torque (i.e change in angular momentum)

    // linear velocity
    glm::vec3 c_dot = P_[i] * physic->getInvTotalMass();

    // angular velocity
    glm::vec3 omega = I_inv * L_[i];

    // the orientation of the stick
    Quaternion prev_q = om->SpringMassDamperGeometry_[i]->GetOrientation();

    // change in rotation
    glm::mat3 tildeOmega = TildeMatrix(omega);
    glm::mat3 R_dot_mat = tildeOmega * prev_q.toMat3();

    glm::quat temp_R_dot = glm::quat_cast(R_dot_mat);

    Quaternion R_dot = { temp_R_dot.w,temp_R_dot.x,temp_R_dot.y,temp_R_dot.z };

    Quaternion next_q = prev_q + R_dot * dt;
    next_q = next_q.normalize();

    om->SpringMassDamperGeometry_[i]->SetRotation(next_q);

    // the location of the mass center
    CurrPosition = om->SpringMassDamperGeometry_[i]->GetPosition();

    // verlet integration method
    glm::vec3 Verlet_x = VerletIntegrationPosition(dt, i);
    om->SpringMassDamperGeometry_[i]->SetPosition(Verlet_x);

    curr_qA_ = qA_[i];
    curr_qB_ = qB_[i];

    // update velocity at A and B
    vA_[i].derivedVelocity = (curr_qA_ - prev_qA_[i]) / dt;
    vB_[i].derivedVelocity = (curr_qB_ - prev_qB_[i]) / dt;

    prev_qA_[i] = curr_qA_;
    prev_qB_[i] = curr_qB_;

    // Update qB points
    qB_[i] -= CurrPosition;
    qB_[i] = glm::inverse(prev_q.toMat3()) * qB_[i];
    qB_[i] = next_q.toMat3() * qB_[i];
    qB_[i] += Verlet_x;
    // Update qA points
    qA_[i] -= CurrPosition;
    qA_[i] = glm::inverse(prev_q.toMat3()) * qA_[i];
    qA_[i] = next_q.toMat3() * qA_[i];
    qA_[i] += Verlet_x;

    // update draw data
    PopulateDrawData();

    // update the inverse inertia tensor
    I_inv = next_q.toMat3() * I_inv * glm::transpose(next_q.toMat3());
    physic->setInvInertiaTensorMatrix(I_inv);

    if (i == 1)
    {
      //std::cout << "verlet position " << glm::to_string(Verlet_x) << std::endl;
      //std::cout << "euler position " << glm::to_string(x) << std::endl;
      //std::cout << "linear velocity " << glm::to_string(c_dot) << std::endl;
      std::cout << "linear force is " << glm::to_string(F_[i]) << std::endl;
      std::cout << "angular force is " << glm::to_string(T_[i]) << std::endl;
      std::cout << "omega is " << glm::to_string(omega) << std::endl;
      std::cout << "inverse inertia tensor matrix " << glm::to_string(I_inv) << std::endl;
    }

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
