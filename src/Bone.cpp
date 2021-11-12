#include "Bone.h"

#include <assimp/scene.h>

Bone::Bone(const std::string& name, int ID, const aiNodeAnim* channel) : m_Name(name), m_ID(ID), m_LocalTransform(1.0f)
{
  // read in position key frame data
  m_NumPositions = channel->mNumPositionKeys;
  for (int positionIndex = 0; positionIndex < m_NumPositions; ++positionIndex)
  {
    aiVector3D aiPosition = channel->mPositionKeys[positionIndex].mValue;
    float timeStamp = channel->mPositionKeys[positionIndex].mTime;
    KeyPosition data;
    data.position.x = aiPosition.x;
    data.position.y = aiPosition.y;
    data.position.z = aiPosition.z;
    data.timeStamp = timeStamp;
    m_Positions.push_back(data);
  }

  // read in orientation key frame data
  m_NumRotations = channel->mNumRotationKeys;
  for (int rotationIndex = 0; rotationIndex < m_NumRotations; ++rotationIndex)
  {
    //glm::quat a()
    aiQuaternion aiOrientation = channel->mRotationKeys[rotationIndex].mValue;
    float timeStamp = channel->mRotationKeys[rotationIndex].mTime;
    KeyRotation data;
    data.orientation._s = aiOrientation.w;
    data.orientation._v.x = aiOrientation.x;
    data.orientation._v.y = aiOrientation.y;
    data.orientation._v.z = aiOrientation.z;
    data.timeStamp = timeStamp;
    m_Rotations.push_back(data);
  }

  // read in scaling key frame data
  m_NumScalings = channel->mNumScalingKeys;
  for (int keyIndex = 0; keyIndex < m_NumScalings; ++keyIndex)
  {
    aiVector3D scale = channel->mScalingKeys[keyIndex].mValue;
    float timeStamp = channel->mScalingKeys[keyIndex].mTime;
    KeyScale data;
    data.scale.x = scale.x;
    data.scale.y = scale.y;
    data.scale.z = scale.z;
    data.timeStamp = timeStamp;
    m_Scales.push_back(data);
  }
}

void Bone::Update(float animationTime)
{
  glm::mat4 translation = InterpolatePosition(animationTime);
  glm::mat4 rotation = InterpolateRotation(animationTime);
  glm::mat4 scale = InterpolateScaling(animationTime);
  m_LocalTransform = translation * rotation * scale;
}

glm::mat4 Bone::getLocalTransform()
{
  return m_LocalTransform;
}

std::string Bone::getBoneName() const
{
  return m_Name;
}

int Bone::getBoneID()
{
  return m_ID;
}

int Bone::getPositionIndex(float animationTime)
{
  for (int index = 0; index < m_NumPositions - 1; ++index)
  {
    if (animationTime < m_Positions[index + 1].timeStamp)
      return index;
  }
  assert(0);
}

int Bone::getRotationIndex(float animationTime)
{
  for (int index = 0; index < m_NumRotations - 1; ++index)
  {
    if (animationTime < m_Rotations[index + 1].timeStamp)
      return index;
  }
  assert(0);
}

int Bone::getScaleIndex(float animationTime)
{
  for (int index = 0; index < m_NumScalings - 1; ++index)
  {
    if (animationTime < m_Scales[index + 1].timeStamp)
      return index;
  }
  assert(0);
}

/* Gets normalized value for Lerp & Slerp*/
float Bone::GetScaleFactor(float lastTimeStamp, float nextTimeStamp, float animationTime)
{
  float scaleFactor = 0.0f;
  float midWayLength = animationTime - lastTimeStamp;
  float framesDiff = nextTimeStamp - lastTimeStamp;
  scaleFactor = midWayLength / framesDiff;
  return scaleFactor;
}

/*figures out which position keys to interpolate b/w and performs the interpolation
and returns the translation matrix*/
glm::mat4 Bone::InterpolatePosition(float animationTime)
{
  if (1 == m_NumPositions)
    return glm::translate(glm::mat4(1.0f), m_Positions[0].position);

  int p0Index = getPositionIndex(animationTime);
  int p1Index = p0Index + 1;
  float scaleFactor = GetScaleFactor(m_Positions[p0Index].timeStamp,
    m_Positions[p1Index].timeStamp, animationTime);

  // lerp
  glm::vec3 finalPosition = Interpolation::lerp(m_Positions[p0Index].position,
    m_Positions[p1Index].position, scaleFactor);

  return glm::translate(glm::mat4(1.0f), finalPosition);
}

/*figures out which rotations keys to interpolate b/w and performs the interpolation
and returns the rotation matrix*/
glm::mat4 Bone::InterpolateRotation(float animationTime)
{
  if (1 == m_NumRotations)
  {
    Quaternion rotation = m_Rotations[0].orientation.normalize();
    return rotation.toMat4();
  }

  int p0Index = getRotationIndex(animationTime);
  int p1Index = p0Index + 1;
  float scaleFactor = GetScaleFactor(m_Rotations[p0Index].timeStamp,
    m_Rotations[p1Index].timeStamp, animationTime);

  // slerp
  Quaternion finalRotation = Interpolation::Slerp(
    m_Rotations[p0Index].orientation,
    m_Rotations[p1Index].orientation,
    scaleFactor);

  return finalRotation.normalize().toMat4();
}

/*figures out which scaling keys to interpolate b/w and performs the interpolation
and returns the scale matrix*/
glm::mat4 Bone::InterpolateScaling(float animationTime)
{
  if (1 == m_NumScalings)
    return glm::scale(glm::mat4(1.0f), m_Scales[0].scale);

  int p0Index = getScaleIndex(animationTime);
  int p1Index = p0Index + 1;
  float scaleFactor = GetScaleFactor(m_Scales[p0Index].timeStamp,
    m_Scales[p1Index].timeStamp, animationTime);

  // lerp
  glm::vec3 finalScale = Interpolation::Elerp(m_Scales[p0Index].scale, m_Scales[p1Index].scale
    , scaleFactor);

  return glm::scale(glm::mat4(1.0f), finalScale);
}