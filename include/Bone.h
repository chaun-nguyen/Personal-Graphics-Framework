#pragma once
#include "LibHeader.h"
#include "Quaternion.h"

#include <vector>
#include <string>

struct aiNodeAnim;

struct KeyPosition
{
  glm::vec3 position;
  float timeStamp;
};
struct KeyRotation
{
  Quaternion orientation;
  float timeStamp;
};
struct KeyScale
{
  glm::vec3 scale;
  float timeStamp;
};

class Bone
{
  float GetScaleFactor(float lastTimeStamp, float nextTimeStamp, float animationTime);
  glm::mat4 InterpolatePosition(float animationTime);
  glm::mat4 InterpolateRotation(float animationTime);
  glm::mat4 InterpolateScaling(float animationTime);
public:
  Bone() = default;
  ~Bone() = default;

  Bone(const std::string& name, int ID, const aiNodeAnim* channel);
  void Update(float animationTime);

  glm::mat4 getLocalTransform();
  std::string getBoneName() const;
  int getBoneID();

  int getPositionIndex(float animationTime);
  int getRotationIndex(float animationTime);
  int getScaleIndex(float animationTime);

private:
  std::vector<KeyPosition> m_Positions;
  std::vector<KeyRotation> m_Rotations;
  std::vector<KeyScale> m_Scales;
  int m_NumPositions;
  int m_NumRotations;
  int m_NumScalings;

  glm::mat4 m_LocalTransform;
  std::string m_Name;
  int m_ID;
};
