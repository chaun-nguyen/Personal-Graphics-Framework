#pragma once
#include "LibHeader.h"
#include <vector>

struct NodeData;
class SkeletalAnimation;

class Animator
{
public:
  Animator(SkeletalAnimation* animation);
  ~Animator() = default;

  void UpdateAnimation(float dt);
  void PlayAnimation(SkeletalAnimation* pAnimation);

  std::vector<glm::mat4>& GetFinalBoneMatrices();
  std::vector<glm::mat4> GetPreOffSetMatrices();

  float speed = 1.f;
  float SlidingSkiddingControl = 1.f;

  void UpdateVBO();

  void CalculateBoneTransform(const NodeData* node, glm::mat4 parentTransform);

  
private:
  std::vector<glm::mat4> m_FinalBoneMatrices;
  std::vector<glm::mat4> m_PreOffSetMatrices;
  SkeletalAnimation* m_CurrentAnimation;
  float m_CurrentTime;
  float m_DeltaTime;
};
