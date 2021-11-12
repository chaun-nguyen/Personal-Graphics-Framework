#include "Animator.h"
#include "SkeletalAnimation.h"
#include "Bone.h"

void Animator::CalculateBoneTransform(const NodeData* node, glm::mat4 parentTransform)
{
  std::string nodeName = node->name;
  glm::mat4 nodeTransform = node->transformation;
  
  Bone* Bone = m_CurrentAnimation->FindBone(nodeName);
  
  if (Bone)
  {
    Bone->Update(m_CurrentTime);
    nodeTransform = Bone->getLocalTransform();
  }
  
  glm::mat4 globalTransformation = parentTransform * nodeTransform;
  
  auto boneInfoMap = m_CurrentAnimation->GetBoneIDMap();
  if (boneInfoMap.find(nodeName) != boneInfoMap.end())
  {
    // get id of bone from map
    int index = boneInfoMap[nodeName].id;

    // get offset matrix of bone from map
    glm::mat4 offset = boneInfoMap[nodeName].offset;

    // use id to index in the array of final matrices and concatenate offset matrix
    m_FinalBoneMatrices[index] = globalTransformation * offset;
    m_PreOffSetMatrices[index] = globalTransformation;
  }
  
  for (int i = 0; i < node->childrenCount; i++)
    CalculateBoneTransform(&node->children[i], globalTransformation);
}

Animator::Animator(SkeletalAnimation* currentAnimation)
{
  m_CurrentTime = 0.0;
  m_CurrentAnimation = currentAnimation;

  m_FinalBoneMatrices.reserve(100);
  m_PreOffSetMatrices.reserve(100);

  for (int i = 0; i < 100; i++)
    m_FinalBoneMatrices.push_back(glm::mat4(1.0f));

  for (int i = 0; i < 100; i++)
    m_PreOffSetMatrices.push_back(glm::mat4(1.0f));
}

void Animator::UpdateAnimation(float dt)
{
  m_DeltaTime = dt;
  if (m_CurrentAnimation)
  {
    m_CurrentTime += m_CurrentAnimation->GetTicksPerSecond() * dt * speed * SlidingSkiddingControl;
    m_CurrentTime = fmod(m_CurrentTime, m_CurrentAnimation->GetDuration());
    CalculateBoneTransform(&m_CurrentAnimation->GetRootNode(), glm::mat4(1.0f));
  }
}

void Animator::PlayAnimation(SkeletalAnimation* pAnimation)
{
  m_CurrentAnimation = pAnimation;
  m_CurrentTime = 0.0f;
}

std::vector<glm::mat4> Animator::GetFinalBoneMatrices()
{
  return m_FinalBoneMatrices;
}

std::vector<glm::mat4> Animator::GetPreOffSetMatrices()
{
  return m_PreOffSetMatrices;
}

void Animator::UpdateVBO()
{
  // update position of bone when model is animating
  CHECKERROR;
  auto& boneInfoMap = m_CurrentAnimation->GetBoneIDMap();

  // get each bone local position
  // and layout continuously
  for (int i = 0; i < m_CurrentAnimation->bonePosition.size(); ++i)
  {
    // local-space position
    m_CurrentAnimation->bonePosition[i] = 
      glm::vec3(m_PreOffSetMatrices[boneInfoMap[m_CurrentAnimation->boneName[i]].id] * 
        glm::vec4(m_CurrentAnimation->boneLocalPosition[i], 1.f));
  }
  CHECKERROR;
  glInvalidateBufferData(m_CurrentAnimation->boneVBO);
  CHECKERROR;
  glNamedBufferSubData(m_CurrentAnimation->boneVBO, 0, m_CurrentAnimation->bonePosition.size() * sizeof(glm::vec3), 
    m_CurrentAnimation->bonePosition.data());
  CHECKERROR;
}

