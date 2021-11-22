#include "SkeletalAnimation.h"

#include <iostream>

#include "Engine.h"
#include "Transform.h"

void SkeletalAnimation::ReadMissingBones(const aiAnimation* animation, Model& model)
{
  int size = animation->mNumChannels;

  auto& boneInfoMap = model.getBoneInfoMap();//getting m_BoneInfoMap from Model class
  int& boneCount = model.getBoneCount(); //getting the m_BoneCounter from Model class

  //reading channels(bones engaged in an animation and their keyframes)
  for (int i = 0; i < size; i++)
  {
    auto channel = animation->mChannels[i];
    std::string boneName = channel->mNodeName.data;

    if (boneInfoMap.find(boneName) == boneInfoMap.end())
    {
      boneInfoMap[boneName].id = boneCount;
      boneCount++;
    }
    m_Bones.push_back(
      Bone(channel->mNodeName.data, boneInfoMap[channel->mNodeName.data].id, channel)
    );
  }

  m_BoneInfoMap = boneInfoMap;
}

void SkeletalAnimation::ReadHeirarchyData(NodeData& dest, const aiNode* src)
{
  assert(src);

  aiMatrix4x4 matrix = src->mTransformation; // bone local transformation

  // read current node
  dest.name = src->mName.data;
  dest.transformation = AssimpHelper::ConvertRowMajorToColumnMajor(matrix);
  dest.childrenCount = src->mNumChildren;

  // go through each children and read them
  for (int i = 0; i < src->mNumChildren; i++)
  {
    NodeData newData;
    ReadHeirarchyData(newData, src->mChildren[i]);
    dest.children.push_back(newData);
  }
}

SkeletalAnimation::SkeletalAnimation(const std::string& animationPath, Model* model)
{
  Assimp::Importer importer;
  const aiScene* scene = importer.ReadFile(animationPath, aiProcess_Triangulate);
  assert(scene && scene->mRootNode);
  auto animation = scene->mAnimations[0];
  m_Duration = animation->mDuration;
  m_TicksPerSecond = animation->mTicksPerSecond;
  ReadHeirarchyData(m_RootNode, scene->mRootNode);
  ReadMissingBones(animation, *model);
}

Bone* SkeletalAnimation::FindBone(const std::string& name)
{
  for (int i = 0; i < m_Bones.size(); ++i)
  {
    if (m_Bones[i].getBoneName() == name)
    {
      return &m_Bones[i];
    }
  }
  return nullptr;
}

float SkeletalAnimation::GetTicksPerSecond()
{
  return m_TicksPerSecond;
}

float SkeletalAnimation::GetDuration()
{
  return m_Duration;
}

const NodeData& SkeletalAnimation::GetRootNode()
{
  return m_RootNode;
}

std::map<std::string, BoneInfo>& SkeletalAnimation::GetBoneIDMap()
{
  return m_BoneInfoMap;
}

std::vector<Bone>& SkeletalAnimation::GetBonesData()
{
  return m_Bones;
}

void SkeletalAnimation::SetBoneWorldPosition(glm::vec3 pos)
{
  boneWorldLocation = pos;
}

void SkeletalAnimation::SetBoneOrientation(glm::mat4& mat)
{
  orientationMatrix = mat;
}

void SkeletalAnimation::DrawBone(ShaderProgram* shaderProgram)
{
  CHECKERROR;
  int loc = glGetUniformLocation(shaderProgram->programID, "color");
  glUniform3fv(loc, 1, glm::value_ptr(glm::vec3(0.f, 1.f, 0.f)));

  // world space
  glm::mat4 modelTr = 
    Translate(boneWorldLocation.x, boneWorldLocation.y, boneWorldLocation.z) * Scale(5.f, 5.f, 5.f);

  modelTr *= orientationMatrix;

  loc = glGetUniformLocation(shaderProgram->programID, "ModelTr");
  glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(modelTr));
  CHECKERROR;

  glBindVertexArray(boneVAO);
  CHECKERROR;
  glDrawElements(GL_LINES, static_cast<GLsizei>(boneIndices.size()), GL_UNSIGNED_INT, 0);
  CHECKERROR;
  glBindVertexArray(0);
  CHECKERROR;
}

void SkeletalAnimation::SetUpHierarchicalRender(const NodeData& root, std::map<std::string, BoneInfo>& boneIDMap,
                                                int index)
{
  Bone* bone = FindBone(root.name);

  if (bone)
  {
    //std::cout << bone->getBoneName() << std::endl;
    // preset index = -1 to delay 1 call to draw hierarchial bones correctly where it starts at hips, start recording at spine
    if (index != -1)
      boneIndices.push_back(index);

    // local-space position
    glm::vec4 localPosition = bone->getLocalTransform()[3];

    // save local bone position for later animating bones in updateVBO function
    boneLocalPosition.push_back(localPosition.xyz);

    // get each bone offset matrix to move to bone-space
    glm::mat4 offsetMatrix = boneIDMap[root.name].offset;

    // get bone-space position
    glm::vec4 finalPosition = Scale(1.f, -1.f, 1.f) * offsetMatrix * localPosition;

    // get the bone position
    bonePosition.push_back(glm::vec3(finalPosition));

    // get name of each bone in the continuous order later good for updating vbo for animating bones
    boneName.push_back(root.name);

    // preset index = -1 to delay 1 call to draw hierarchial bones correctly where it starts at hips, start recording at spine
    if (index != -1)
      boneIndices.push_back(bonePosition.size() - 1);

    // update index
    index = bonePosition.size() - 1;

    // Set Up IK Chain
    if (root.name == "LeftShoulder" || root.name == "LeftArm" || root.name == "LeftForeArm" || root.name == "LeftHand"
      || root.name == "LeftHandIndex1" || root.name == "LeftHandIndex2" || root.name == "LeftHandIndex3")
    {
      auto* ikm = Engine::managers_.GetManager<InverseKinematicManager*>();
      IKData data;
      data.name = bone->getBoneName();
      data.worldPosition = glm::vec3(finalPosition);
      data.localPosition = localPosition.xyz;

      ikm->m_CCDSolver.AddBoneToChain(data);

      // data to draw IK chain (final position)
      ikm->IKChainPosition.push_back(data.worldPosition);
    }
  }

  // recursively through the hierarchical bones
  for (int i = 0; i < root.childrenCount; ++i)
  {
    SetUpHierarchicalRender(root.children[i], boneIDMap, index);
  }
}

void SkeletalAnimation::SetUpVAO()
{
  // preset index = -1 to delay 1 call to draw hierarchial bones correctly where it starts at hips, start recording at spine
  SetUpHierarchicalRender(m_RootNode, m_BoneInfoMap, -1);

  CHECKERROR;
  glCreateVertexArrays(1, &boneVAO);
  CHECKERROR;
  glCreateBuffers(1, &boneVBO);
  CHECKERROR;
  glCreateBuffers(1, &boneEBO);
  CHECKERROR;
  glNamedBufferStorage(boneVBO,
    bonePosition.size() * sizeof(glm::vec3),
    nullptr, GL_DYNAMIC_STORAGE_BIT);
  CHECKERROR;

  glNamedBufferSubData(boneVBO,
    0,
    bonePosition.size() * sizeof(glm::vec3),
    bonePosition.data());
  CHECKERROR;

  // bone position
  glEnableVertexArrayAttrib(boneVAO, 0);
  glVertexArrayVertexBuffer(boneVAO, 0, boneVBO, 0, sizeof(glm::vec3));
  glVertexArrayAttribFormat(boneVAO, 0, 3, GL_FLOAT, GL_FALSE, 0);
  glVertexArrayAttribBinding(boneVAO, 0, 0);

  CHECKERROR;
  // setting up indices
  glNamedBufferStorage(boneEBO, boneIndices.size() * sizeof(unsigned int), boneIndices.data(), GL_DYNAMIC_STORAGE_BIT);
  glVertexArrayElementBuffer(boneVAO, boneEBO);
  CHECKERROR;
}
