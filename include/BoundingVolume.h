#pragma once
#include "LibHeader.h"
#include <vector>

class Object;

class BoundingVolume
{
public:
  BoundingVolume() = default;
  virtual ~BoundingVolume() = default;

  virtual void extend(const glm::vec3& P) = 0;

  virtual void Update() = 0;
  virtual void Draw() = 0;

  virtual bool containsPoint(glm::vec3 P) = 0;
  virtual bool containsBV(BoundingVolume* bv) = 0;
  virtual bool intersect(BoundingVolume* other) = 0;

  glm::vec3 min_ = {};
  glm::vec3 max_ = {};
  glm::vec3 center_ = {};
  float size_ = 0.f;
  Object* bv_object = nullptr;
  Object* parent = nullptr;
};

class BV_Sphere : public BoundingVolume
{
public:
  BV_Sphere(const std::vector<glm::vec3>& vertices, Object* owner); // model
  BV_Sphere(glm::vec3 min, glm::vec3 max, Object* owner);

  void extend(const glm::vec3& P) override;
  void Update() override;
  void Draw() override;

  bool containsPoint(glm::vec3 P) override;
  bool containsBV(BoundingVolume* bv) override;
  bool intersect(BoundingVolume* other) override;
};

class BV_AABB : public BoundingVolume
{
public:
  BV_AABB(const std::vector<glm::vec3>& vertices, Object* owner, glm::vec3 diffuse); // model
  BV_AABB(glm::vec3 min, glm::vec3 max, Object* owner, glm::vec3 diffuse);

  void extend(const glm::vec3& P) override;
  void Update() override;
  void Draw() override;

  bool containsPoint(glm::vec3 P) override;
  bool containsBV(BoundingVolume* bv) override;
  bool intersect(BoundingVolume* other) override;

  glm::vec3 diffuse_;
};