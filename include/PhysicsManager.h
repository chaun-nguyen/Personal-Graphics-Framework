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

  glm::vec3 getLeftAnchorPointPosition();
  glm::vec3 getRightAnchorPointPosition();

  void setLeftAnchorPointPosition(glm::vec3 newPos);
  void setRightAnchorPointPosition(glm::vec3 newPos);

  float getSpringConstants(int index);
  float getDampingConstants(int index);

  void setSpringConstants(int index, float newK);
  void setDampingConstants(int index, float newD);

  bool simulateFlag = false;
  int key;
private:
  // Verlet integration method
  std::vector<glm::vec3> PrevPosition;
  glm::vec3 CurrPosition;
  glm::vec3 VerletIntegrationPosition(float dt, int index);

  // Runge-Kutta 4th order integration method
  glm::vec3 RK4thOrderIntegrationPosition(float dt, int index);
  Derivative EvaluateDerivative(float dt, const Derivative& d);

  void Movement();

  void ComputeExternalForce(int i);
  void DynamicSimulation(float dt);

  glm::mat3 TildeMatrix(glm::vec3& v);

  void PopulateQB();
  void PopulateQA();

  // world space location of 2 end points
  std::vector<glm::vec3> prev_qA_;
  glm::vec3 curr_qA_;
  std::vector<glm::vec3> qA_;

  std::vector<glm::vec3> prev_qB_;
  glm::vec3 curr_qB_;
  std::vector<glm::vec3> qB_;


  std::vector<glm::vec3> qA_local_;
  std::vector<glm::vec3> qB_local_;

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
  std::vector<glm::vec3> P_;
  // total angular momentum
  std::vector<glm::vec3> L_;

  // spring damper system constants for
  // 5 sticks, 6 springs, 2 anchor points
  std::vector<float> k;
  std::vector<float> d;

  // spring damper system constants
  float g; // gravity constant
  glm::vec3 verticalVector = { 0.f,-1.f,0.f };
  unsigned size;
  float speed; // anchor point speed

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