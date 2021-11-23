#include "Quaternion.h"
#include <glm/glm/gtx/norm.hpp>

Quaternion::Quaternion() : _s(1.0), _v(0) // identity quaternion
{
}

Quaternion::Quaternion(float scale, float x, float y, float z) : _s(scale)
{
  _v.x = x;
  _v.y = y;
  _v.z = z;
}

Quaternion::Quaternion(float scale, glm::vec3 v) : _s(scale), _v(v)
{
}

Quaternion::Quaternion(float eulerAngle)
{
  glm::vec3 c = glm::vec3(glm::cos(eulerAngle * 0.5f));
  glm::vec3 s = glm::vec3(glm::sin(eulerAngle * 0.5f));

  _s = c.x * c.y * c.z + s.x * s.y * s.z;
  _v.x = s.x * c.y * c.z - c.x * s.y * s.z;
  _v.y = c.x * s.y * c.z + s.x * c.y * s.z;
  _v.z = c.x * c.y * s.z - s.x * s.y * c.z;
}

Quaternion& Quaternion::operator=(const Quaternion& q)
{
  _s = q._s;
  _v = q._v;

  return *this;
}

Quaternion Quaternion::operator/(float s)
{
  return Quaternion(_s / s, _v.x / s, _v.y / s, _v.z / s);
}

Quaternion Quaternion::operator*(float s)
{
  return Quaternion(_s * s, _v.x * s, _v.y * s, _v.z * s);
}

Quaternion Quaternion::operator*(Quaternion q)
{
  float scalar = _s * q._s - glm::dot(_v, q._v);
  glm::vec3 vector = _s* q._v + q._s * _v + glm::cross(_v, q._v);

  return Quaternion(scalar, vector);
}

glm::vec3 Quaternion::operator*(glm::vec3 r)
{
  return (_s * _s - glm::dot(_v, _v)) * r + 2.f * glm::dot(_v, r) * _v + 2.f * _s * glm::cross(_v, r);
}

Quaternion& Quaternion::operator*=(Quaternion q)
{
  *this = *this * q;

  return *this;
}

Quaternion Quaternion::operator+(Quaternion q)
{
  return Quaternion(_s + q._s, _v + q._v);
}

Quaternion& Quaternion::operator+=(Quaternion q)
{
  *this = *this + q;

  return *this;
}

Quaternion Quaternion::conjugate()
{
  return Quaternion(_s, -_v.x, -_v.y, -_v.z);
}

Quaternion Quaternion::normalize()
{
  float length = this->magnitude();

  float oneOverLength = 1.0f / length;

  return *this * oneOverLength;
}

Quaternion Quaternion::inverse()
{
  return conjugate() / dot(*this);
}

// Rotates a quaternion from an vector of 3 components normalized axis and an angle.
Quaternion Quaternion::angle_axis_rotation(float angle, glm::vec3 normalizedAxis)
{
  // assuming axis is already normalized
  float halfCos = glm::cos(angle * 0.5f);
  float halfSin = glm::sin(angle * 0.5f);
  glm::vec3 vector = normalizedAxis * halfSin;

  return *this * Quaternion(halfCos, vector);
}

float Quaternion::dot(Quaternion& q1)
{
  return _s * q1._s + glm::dot(_v, q1._v);
}

float Quaternion::magnitude()
{
  return glm::sqrt(dot(*this));
}

float Quaternion::angleRadian()
{
  return glm::acos(_s) * 2.f;
}

float Quaternion::angleDegree()
{
  return glm::degrees(glm::acos(_s) * 2.f);
}

glm::mat3 Quaternion::toMat3()
{
  glm::mat3 res;
  res[0][0] = 1.f - 2.f * (_v.y * _v.y + _v.z * _v.z);
  res[0][1] = 2.f * (_v.x * _v.y + _s * _v.z);
  res[0][2] = 2.f * (_v.x * _v.z - _s * _v.y);

  res[1][0] = 2.f * (_v.x * _v.y - _s * _v.z);
  res[1][1] = 1.f - 2.f * (_v.x * _v.x + _v.z * _v.z);
  res[1][2] = 2.f * (_v.y * _v.z + _s * _v.x);

  res[2][0] = 2.f * (_v.x * _v.z + _s * _v.y);
  res[2][1] = 2.f * (_v.y * _v.z - _s * _v.x);
  res[2][2] = 1.f - 2.f * (_v.x * _v.x + _v.y * _v.y);

  return res;
}

glm::mat4 Quaternion::toMat4()
{
  glm::mat3 res = this->toMat3();

  return glm::mat4(res);
}

Quaternion Quaternion::rotate(glm::vec3& orig, glm::vec3& dest)
{
  Quaternion result;

  // ensure vectors are normalized
  orig = glm::normalize(orig);
  dest = glm::normalize(dest);

  float cosTheta = glm::dot(orig, dest);
  glm::vec3 rotationAxis;

  if (cosTheta < -1.f + std::numeric_limits<float>::epsilon())
  {
    rotationAxis = glm::cross(glm::vec3(0, 0, 1), orig);
    if (glm::length2(rotationAxis) < std::numeric_limits<float>::epsilon())
      rotationAxis = glm::cross(glm::vec3(1, 0, 0), orig);

    rotationAxis = glm::normalize(rotationAxis);
    return angleAxis(glm::pi<float>(), rotationAxis);
  }

  // Implementation from Stan Melax's Game Programming Gems 1 article
  rotationAxis = glm::cross(orig, dest);
  float s = glm::sqrt((1.f + cosTheta) * 2.f);
  float invs = 1.f / s;

  result._s = s * 0.5f;
  result._v = rotationAxis * invs;

  return result;
}

Quaternion Quaternion::angleAxis(float angle, glm::vec3& v)
{
  Quaternion result;

  float s = glm::sin(angle * 0.5f);

  result._s = glm::cos(angle * 0.5f);
  result._v = v * s;

  return result;
}

Quaternion Interpolation::lerp(Quaternion q1, Quaternion q2, float t)
{
  return q1* (1.f - t) + q2 * t;
}

Quaternion Interpolation::Slerp(Quaternion q1, Quaternion q2, float t)
{
  Quaternion q = q2;

  float cosTheta = q1.dot(q2);

  // If cosTheta < 0, the interpolation will take the long way around the sphere. 
  // To fix this, one quat must be negated.
  if (cosTheta < 0.f)
  {
    q = q2 * -1.f;
    cosTheta = -cosTheta;
  }

  // Perform a linear interpolation when cosTheta is close to 1 to avoid side effect of sin(angle) becoming a zero
  if (cosTheta > 1.f - std::numeric_limits<float>::epsilon())
  {
    // Linear interpolation
    return lerp(q1, q, t);
  }
  else
  {
    // Slerp formula
    float alpha = glm::acos(cosTheta);
    float t1 = glm::sin(alpha - t * alpha) / glm::sin(alpha);
    float t2 = glm::sin(t * alpha) / glm::sin(alpha);

    return q1 * t1 + q * t2;
  }
}

glm::vec3 Interpolation::lerp(glm::vec3 v1, glm::vec3 v2, float t)
{
  return (1 - t) * v1 + t * v2;
}

glm::vec3 Interpolation::Elerp(glm::vec3 s1, glm::vec3 s2, float t)
{
  glm::vec3 divide = (s2 / s1);
  divide.x = glm::pow(divide.x, t);
  divide.y = glm::pow(divide.y, t);
  divide.z = glm::pow(divide.z, t);

  return s1 * divide;
}

Quaternion Utility::ConvertMatrixToQuaternion(glm::mat3 m)
{
  float scalar = glm::sqrt(m[0][0] + m[1][1] + m[2][2] + 1.f) / 2.f;
  float denom = 4.f * scalar;
  glm::vec3 vector = { (m[1][2] - m[2][1]) / denom, (m[2][0] - m[0][2]) / denom, (m[0][1] - m[1][0]) / denom };

  return Quaternion(scalar, vector);
}
