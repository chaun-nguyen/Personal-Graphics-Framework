#include "Physics.h"

#include "Shape.h"

void Physics::Setup(Object* owner, Shape* polygonData, float mass)
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

  // inertia tensor matrix
  for (int i = 0; i < size; ++i)
  {
    // Ixx
    I_obj[0][0] += r[i].y * r[i].y + r[i].z * r[i].z;

    // Iyy
    I_obj[1][1] += r[i].x * r[i].x + r[i].z * r[i].z;

    // Izz
    I_obj[2][2] += r[i].x * r[i].x + r[i].y * r[i].y;

    // Ixy
    float Ixy = r[i].x * r[i].y;
    I_obj[1][0] += -Ixy;
    I_obj[0][1] += -Ixy;

    // Ixz
    float Ixz = r[i].x * r[i].z;
    I_obj[2][0] += -Ixz;
    I_obj[0][2] += -Ixz;

    // Iyz
    float Iyz = r[i].y * r[i].z;
    I_obj[2][1] += -Iyz;
    I_obj[1][2] += -Iyz;
  }
  PrecisionCheck(I_obj[0]);
  PrecisionCheck(I_obj[1]);
  PrecisionCheck(I_obj[2]);
  I_obj *= M;
  I_obj_inv = glm::inverse(I_obj);
}

float Physics::getTotalMass()
{
  return M;
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

