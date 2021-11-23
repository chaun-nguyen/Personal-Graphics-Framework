#pragma once
#include <glm/glm/vec3.hpp>
#include <glm/glm/mat3x3.hpp>
#include <glm/glm/mat4x4.hpp>

class Quaternion;

// interpolation
namespace Interpolation
{
  Quaternion lerp(Quaternion q1, Quaternion q2, float t);
  Quaternion Slerp(Quaternion q1, Quaternion q2, float t);
  glm::vec3 lerp(glm::vec3 v1, glm::vec3 v2, float t);
  glm::vec3 Elerp(glm::vec3 s1, glm::vec3 s2, float t);
}

namespace Utility
{
  Quaternion ConvertMatrixToQuaternion(glm::mat3 m);
}

class Quaternion
{
public:
  Quaternion();
  ~Quaternion() = default;

  // construct quaternion
  Quaternion(float scale, float x, float y, float z);
  Quaternion(float scale, glm::vec3 v);

  // construct quaternion from euler angle
  Quaternion(float eulerAngle);

  // assignment operator
  Quaternion& operator=(const Quaternion& q);

  // division
  Quaternion operator/(float s);

  // multiplication
  Quaternion operator*(float s);
  Quaternion operator*(Quaternion q);
  glm::vec3 operator*(glm::vec3 r);
  Quaternion& operator*=(Quaternion q);

  // addition
  Quaternion operator+(Quaternion q);
  Quaternion& operator+=(Quaternion q);

  // quaternion properties
  Quaternion conjugate();
  Quaternion normalize();
  Quaternion inverse();

  // Rotates a quaternion from an vector of 3 components normalized axis and an angle.
  Quaternion angle_axis_rotation(float angle, glm::vec3 normalizedAxis);

  // dot product
  float dot(Quaternion& q);

  // length
  float magnitude();

  // get rotation angle from quaternion
  float angleRadian();
  float angleDegree();
  glm::vec3 getAxis();

  // convert quaternion to matrix
  glm::mat3 toMat3();
  glm::mat4 toMat4();

  // representation of a quaternion
  float _s;     // scalar factor
  glm::vec3 _v; // vector
private:

};

namespace QUATERNION
{
  // rotate from vector to vector
  Quaternion rotate(glm::vec3& orig, glm::vec3& dest);

  // build a quaternion from an angle and an axis
  Quaternion angleAxis(float angle, glm::vec3& v);
}
