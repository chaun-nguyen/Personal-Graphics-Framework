//#include "math.h"
#include "transform.h"

float* Pntr(glm::mat4& M)
{
  return &(M[0][0]);
}

//@@ The following procedures should calculate and return 4x4
//transformation matrices instead of the identity.

// Return a rotation matrix around an axis (0:X, 1:Y, 2:Z) by an angle
// measured in degrees.  NOTE: Make sure to convert degrees to radians
// before using sin and cos.  HINT: radians = degrees*PI/180
const float pi = 3.14159f;
glm::mat4 Rotate(const int i, const float theta)
{
  glm::mat4 R(1.0f);
  int j = (i + 1) % 3;
  int k = (j + 1) % 3;
  R[j][j] = R[k][k] = cos(theta * pi / 180.0f);
  R[j][k] = sin(theta * pi / 180.0f);
  R[k][j] = -R[j][k];
  return R;
}

// Return a scale matrix
glm::mat4 Scale(const float x, const float y, const float z)
{
  glm::mat4 S(1.0f);
  S[0][0] = x;
  S[1][1] = y;
  S[2][2] = z;
  return S;
}

// Return a translation matrix
glm::mat4 Translate(const float x, const float y, const float z)
{
  glm::mat4 T(1.0f);
  T[3][0] = x;
  T[3][1] = y;
  T[3][2] = z;
  return T;
}

// Returns a perspective projection matrix
glm::mat4 Perspective(const float rx, const float ry,
  const float front, const float back)
{
  glm::mat4 P;
  P[0][0] = 1.0f / rx;
  P[1][1] = 1.0f / ry;
  P[2][2] = -(back + front) / (back - front);
  P[3][2] = -(2.0f * front * back) / (back - front);
  P[2][3] = -1.f;
  P[3][3] = 0.f;
  return P;
}

glm::mat4 LookAt(const glm::vec3 Eye, const glm::vec3 Center, const glm::vec3 Up)
{
  glm::mat4 R;
  glm::vec3 V = glm::normalize(Center - Eye);
  glm::vec3 A = glm::normalize(glm::cross(V, Up));
  glm::vec3 B = glm::cross(A, V);

  R[0][0] = A.x;  R[1][0] = A.y;  R[2][0] = A.z;  R[3][0] = -glm::dot(A, Eye);
  R[0][1] = B.x;  R[1][1] = B.y;  R[2][1] = B.z;  R[3][1] = -glm::dot(B, Eye);
  R[0][2] = -V.x; R[1][2] = -V.y; R[2][2] = -V.z; R[3][2] = glm::dot(V, Eye);
  R[0][3] = 0.f;  R[1][3] = 0.f;  R[2][3] = 0.f;  R[3][3] = 1.f;

  return R;
}

glm::mat4 PerspectiveFov(const float fov, const float aspect, const float front, const float back)
{
  float tanHalfFov = tanf(fov / 2.0f);
  
  glm::mat4 P;
  P[0][0] = 1.0f / (aspect * tanHalfFov);
  P[1][1] = 1.0f / tanHalfFov;
  P[2][2] = -(back + front) / (back - front);
  P[3][2] = -(2.0f * front * back) / (back - front);
  P[2][3] = -1.f;
  P[3][3] = 0.f;
  return P;
}

