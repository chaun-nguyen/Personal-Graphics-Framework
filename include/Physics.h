#pragma once
#include <glm/glm/vec3.hpp>
#include <glm/glm/mat3x3.hpp>
#include <vector>

class Object;
class Shape;

class Physics
{
public:
  Physics() = default;
  ~Physics() = default;

  void Setup(Object* owner, Shape* polygonData, float mass, glm::vec3 scaleFactor);

  float getTotalMass();
  void setTotalMass(float newMass);
  float getInvTotalMass();
  glm::vec3 getCenterOfMass();
  std::vector<glm::vec3>& getOrientationVector();
  glm::mat3 getInertiaTensorMatrix();
  glm::mat3 getInvInertiaTensorMatrix();
  void setInvInertiaTensorMatrix(glm::mat3 new_I_obj_inv);
  Object* getOwner();

private:
  // Epsilon check
  void PrecisionCheck(glm::vec3& value);
  void CalculateInertiaTensorMatrix();
  float epsilon = 0.00001f;

  // detail geometry
  float M = 0.f; // total mass
  float inv_M = 0.f;
  glm::vec3 c = glm::vec3(0.f); // center of mass
  std::vector<glm::vec3> r; // orientaion vectors
  glm::vec3 scaleFactor;
  // inertia tensor
  glm::mat3 I_obj = glm::mat3(0.f);
  glm::mat3 I_obj_inv = glm::mat3(0.f);

  // object own this physics component
  Object* owner_ = nullptr;
};