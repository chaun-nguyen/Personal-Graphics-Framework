#include "VQS.h"

VQS::VQS(glm::vec3 translation, Quaternion rotation, glm::vec3 scale) : v(translation), q(rotation), s(scale)
{
}

VQS& VQS::operator=(const VQS& rhs)
{
  v = rhs.v;
  q = rhs.q;
  s = rhs.s;

  return *this;
}

VQS VQS::operator+(VQS rhs)
{
  VQS vqs;

  vqs.v += rhs.v;
  vqs.q += rhs.q;
  vqs.s += rhs.s;

  return vqs;
}

VQS VQS::operator*(VQS rhs)
{
  VQS vqs;

  vqs.v = rhs * v;
  vqs.q = q * rhs.q;
  vqs.s = rhs.s * s;

  return vqs;
}

glm::vec3 VQS::operator*(glm::vec3 r)
{
  glm::vec3 temp = s * r;
  temp = q * temp;
  temp = q.inverse() * temp;
  temp = temp + v;

  return temp;
}

VQS VQS::operator*(float scalar)
{
  VQS vqs;

  vqs.v = v * scalar;
  vqs.q = q * scalar;
  vqs.s = s * scalar;

  return vqs;
}

VQS VQS::inverse()
{
  VQS vqs;

  glm::vec3 s_inverse(1.f / s.x, 1.f / s.y, 1.f / s.z);
  glm::vec3 temp;
  temp = -v * s_inverse;
  temp = q.inverse() * temp;
  temp = q * temp;

  vqs.v = temp;
  vqs.q = q.inverse();
  vqs.s = s_inverse;

  return vqs;
}

VQS VQS::identity()
{
  VQS vqs;

  vqs.v = glm::vec3(0);
  vqs.q = Quaternion();
  vqs.s = glm::vec3(1.f);

  return vqs;
}
