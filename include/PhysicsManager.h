#pragma once
#include <vector>
#include <glm/glm/vec3.hpp>
#include <glm/glm/mat3x3.hpp>
#include "Quaternion.h"
#include "ManagerBase.h"

class ShaderProgram;

struct Derivative
{
  glm::vec3 derivedVelocity = glm::vec3(0.f);
  glm::vec3 derivedAcceleration = glm::vec3(0.f);
};

class PhysicsManager : public ManagerBase<PhysicsManager>
{
public:
  PhysicsManager() = default;
  ~PhysicsManager() override;

  void Setup() override;
  void Update() override;

  void Draw(ShaderProgram* shader);

  bool simulateFlag = false;
  int key;
private:
  void Movement();

  void ComputeExternalForce(int i);
  void DynamicSimulation(float dt);

  glm::mat3 TildeMatrix(glm::vec3& v);

  void PopulateQB();
  void PopulateQA();

  // world space location of 2 end points
  std::vector<glm::vec3> qA_;
  std::vector<glm::vec3> qB_;

  // velocity of sticks of 2 end points
  std::vector<Derivative> vA_;
  std::vector<Derivative> vB_;

  // total linear force on sticks
  std::vector<glm::vec3> fA_;
  std::vector<glm::vec3> fB_;
  std::vector<glm::vec3> F_;

  // total angular force on sticks
  std::vector<glm::vec3> tA_;
  std::vector<glm::vec3> tB_;
  std::vector<glm::vec3> T_;

  // total linear momentum
  std::vector<glm::vec3> pA_;
  std::vector<glm::vec3> pB_;
  std::vector<glm::vec3> P_;
  // total angular momentum
  std::vector<glm::vec3> L_;

  // spring damper system constants for
  // 5 sticks, 6 springs, 2 anchor points
  std::vector<float> k;
  std::vector<float> d;

  // spring damper system constants
  float g = 9.8f; // gravity constant
  glm::vec3 verticalVector = { 0.f,-1.f,0.f };
  unsigned size;

  // Epsilon check
  void PrecisionCheck(glm::vec3& value);

  float epsilon = 0.00001f;

  // draw springs
  unsigned springVAO;
  unsigned springVBO;
  unsigned springEBO;
  std::vector<glm::vec3> springPosition;
  std::vector<unsigned int> springIndices;

  void PopulateDrawData();
  void SetUpVAO();
  void UpdateVBO();
};