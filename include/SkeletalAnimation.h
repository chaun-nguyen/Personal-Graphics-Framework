#pragma once
#include "LibHeader.h"
#include "Model.h"
#include "Bone.h"

#include <vector>
#include <map>

struct NodeData
{
  glm::mat4 transformation;
  std::string name;
  int childrenCount;
  std::vector<NodeData> children;
};

class SkeletalAnimation
{
  void ReadMissingBones(const aiAnimation* animation, Model& model);
  void ReadHeirarchyData(NodeData& dest, const aiNode* src);

public:
  SkeletalAnimation() = default;
  ~SkeletalAnimation();

  SkeletalAnimation(const std::string& animationPath, Model* model);

  Bone* FindBone(const std::string& name);

  float GetTicksPerSecond();
  float GetDuration();
  const NodeData& GetRootNode();
  std::map<std::string, BoneInfo>& GetBoneIDMap();
  std::vector<Bone>& GetBonesData();

  // bones' world location (motion along a space curve)
  void SetBoneWorldPosition(glm::vec3 pos);
  // bones' orientation along a space curve
  void SetBoneOrientation(glm::mat4& mat);

  // draw bones
  void DrawBone(ShaderProgram* shaderProgram);
  void SetUpVAO();

  unsigned int boneVAO;
  unsigned int boneVBO;
  unsigned int boneEBO;

  std::vector<glm::vec3> boneLocalPosition;
  std::vector<glm::vec3> bonePosition;
  std::vector<std::string> boneName;
  std::vector<unsigned int> boneIndices;

  void SetUpHierarchicalRender(const NodeData& root,
    std::map<std::string, BoneInfo>& boneIDMap,
    int index);

private:
  float m_Duration;
  int m_TicksPerSecond;
  std::vector<Bone> m_Bones;
  NodeData m_RootNode;
  std::map<std::string, BoneInfo> m_BoneInfoMap;

  // motion along a space curve
  glm::vec3 boneWorldLocation = { 0.f,0.f,0.f };
  glm::mat4 orientationMatrix = glm::mat4(1.f);
};
