#include "BoundingVolume.h"
#include "Object.h"
#include "Box.h"
#include "Transform.h"
#include <algorithm>

////////////////////////////// Bounding Volume Sphere //////////////////////////////
// model loading
BV_Sphere::BV_Sphere(const std::vector<glm::vec3>& vertices, Object* owner)
{
}

// procedural shape (box, sphere, plane)
BV_Sphere::BV_Sphere(glm::vec3 min, glm::vec3 max, Object* owner)
{
  parent = owner;
  min_ = min;
  max_ = max;
}

void BV_Sphere::extend(const glm::vec3& P)
{
}

void BV_Sphere::Update()
{
}

void BV_Sphere::Draw()
{
}

bool BV_Sphere::containsPoint(glm::vec3 P)
{
  return false;
}

bool BV_Sphere::containsBV(BoundingVolume* bv)
{
  return false;
}

bool BV_Sphere::intersect(BoundingVolume* other)
{
  return false;
}

////////////////////////////// Bounding Volume AABB //////////////////////////////
// loading model
BV_AABB::BV_AABB(const std::vector<glm::vec3>& vertices, Object* owner, glm::vec3 diffuse) : diffuse_(diffuse)
{
  parent = owner;
  min_ = glm::vec3(std::numeric_limits<float>::max());
  max_ = glm::vec3(std::numeric_limits<float>::min());

  for (unsigned i = 0; i < vertices.size(); ++i)
  {
    // 3 axes
    for (unsigned c = 0; c < 3; ++c)
    {
      min_[c] = std::min(min_[c], vertices[i][c]);
      max_[c] = std::max(max_[c], vertices[i][c]);
    }
  }

  bv_object = new Object(new Box(), diffuse_, { 1.f,1.f,1.f });
  center_ = (max_ + min_) / 2.0f;
  for (int c = 0; c < 3; c++)
    size_ = std::max(size_, (max_[c] - min_[c]) / 2.0f);
}

// procedural shape (box, sphere, plane)
BV_AABB::BV_AABB(glm::vec3 min, glm::vec3 max, Object* owner, glm::vec3 diffuse) : diffuse_(diffuse)
{
  parent = owner;
  min_ = min;
  max_ = max;

  bv_object = new Object(new Box(), diffuse_, { 1.f,1.f,1.f });
  center_ = (max_ + min_) / 2.0f;
  for (int c = 0; c < 3; c++)
    size_ = std::max(size_, (max_[c] - min_[c]) / 2.0f);
}

void BV_AABB::extend(const glm::vec3& P)
{
  // 3 axes
  for (unsigned c = 0; c < 3; ++c)
  {
    min_[c] = std::min(min_[c], P[c]);
    max_[c] = std::max(max_[c], P[c]);
  }

  center_ = (max_ + min_) / 2.0f;
  for (int c = 0; c < 3; c++)
    size_ = std::max(size_, (max_[c] - min_[c]) / 2.0f);
}

// update internal center and min and max of the bounding volume aabb
void BV_AABB::Update()
{
  if (bv_object->GetDirtyFlag())
  {
    glm::vec4 center = { 0.f,0.f,0.f, 1.0f };
    glm::vec3 halfSize = { 1.f,1.f,1.f };

    // transform center
    // need to add relative transform and multiply relative scale
    glm::vec3 pos = bv_object->GetPosition();
    // world space scale * object space scale
    glm::vec3 scale = bv_object->GetScale();
    Quaternion orientation = bv_object->GetOrientation();
    glm::mat4 R = orientation.toMat4();

    center_ = glm::vec3(Translate(pos.x, pos.y, pos.z) * R * Scale(scale.x, scale.y, scale.z) * center);

    glm::vec3 newHalfSize = glm::vec3(0.f);

    for (unsigned c = 0; c < 3; ++c)
    {
      newHalfSize[c] = glm::abs(R[c][0]) * (halfSize[0] * scale[0]) +
        glm::abs(R[c][1]) * (halfSize[1] * scale[1]) +
        glm::abs(R[c][2]) * (halfSize[2] * scale[2]);
    }
    max_ = center_ + newHalfSize;
    min_ = center_ - newHalfSize;
    size_ = 0.f; // reset
    for (int c = 0; c < 3; c++)
      size_ = std::max(size_, (max_[c] - min_[c]) / 2.0f);
  }
}

void BV_AABB::Draw()
{
  CHECKERROR;
  if (bv_object)
  {
    if (bv_object->shape)
    {
      glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
      bv_object->shape->DrawVAO();
    }
  }
  CHECKERROR;
}

bool BV_AABB::containsPoint(glm::vec3 P)
{
  return (P.x >= min_.x) && (P.x <= max_.x) &&
         (P.y >= min_.y) && (P.y <= max_.y) &&
         (P.z >= min_.z) && (P.z <= max_.z);
}

bool BV_AABB::containsBV(BoundingVolume* bv)
{
  return containsPoint(bv->min_) && containsPoint(bv->max_);
}

bool BV_AABB::intersect(BoundingVolume* other)
{
  for (unsigned c = 0; c < 3; ++c)
  {
    if (max_[c] < other->min_[c] || other->max_[c] < min_[c])
      return false;
  }

  return true;
}

