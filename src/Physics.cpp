#include "Physics.h"

#include "Shape.h"

void Physics::Setup(Object* owner, Shape* polygonData, float mass, glm::vec3 scaleFactor_)
{
  owner_ = owner;
  std::vector<glm::vec4>& q = polygonData->Pnt;
  unsigned size = q.size();

  // total mass
  for (int i = 0; i < size; ++i)
  {
    M += mass;
  }
  inv_M = 1.f / M;

  // center of mass
  for (int i = 0; i < size; ++i)
  {
    c += mass * glm::vec3(q[i]);
  }
  // too small -> round up to zero
  PrecisionCheck(c);
  c *= inv_M;

  // orientation vectors
  r.resize(size);
  for (int i = 0; i < size; ++i)
  {
    r[i] = glm::vec3(q[i]) - c;
  }

  scaleFactor = scaleFactor_;
  CalculateInertiaTensorMatrix();
}

float Physics::getTotalMass()
{
  return M;
}

void Physics::setTotalMass(float newMass)
{
  M = newMass;
  inv_M = 1.f / M;
  CalculateInertiaTensorMatrix();
}

float Physics::getInvTotalMass()
{
  return inv_M;
}

glm::vec3 Physics::getCenterOfMass()
{
  return c;
}

std::vector<glm::vec3>& Physics::getOrientationVector()
{
  return r;
}

glm::mat3 Physics::getInertiaTensorMatrix()
{
  return I_obj;
}

glm::mat3 Physics::getInvInertiaTensorMatrix()
{
  return I_obj_inv;
}

void Physics::setInvInertiaTensorMatrix(glm::mat3 new_I_obj_inv)
{
  I_obj_inv = new_I_obj_inv;
}

Object* Physics::getOwner()
{
  return owner_;
}

void Physics::PrecisionCheck(glm::vec3& value)
{
  for (int i = 0; i < 3; ++i)
  {
    value[i] = value[i] < epsilon ? 0.f : value[i];
  }
}

void Physics::CalculateInertiaTensorMatrix()
{
  // inertia tensor matrix
  I_obj = M * glm::mat3(
    { scaleFactor.y * scaleFactor.y + scaleFactor.z * scaleFactor.z,0.f,0.f },
    { 0.f, scaleFactor.x * scaleFactor.x + scaleFactor.z * scaleFactor.z,0.f },
    { 0.f,0.f,scaleFactor.x * scaleFactor.x + scaleFactor.y * scaleFactor.y }
  );

  I_obj_inv = inv_M * glm::mat3(
    { 1.f / (scaleFactor.y * scaleFactor.y + scaleFactor.z * scaleFactor.z),0.f,0.f },
    { 0.f, 1.f / (scaleFactor.x * scaleFactor.x + scaleFactor.z * scaleFactor.z),0.f },
    { 0.f,0.f,1.f / (scaleFactor.x * scaleFactor.x + scaleFactor.y * scaleFactor.y) }
  );
}

