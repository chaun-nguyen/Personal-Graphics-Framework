#include "Octree.h"
#include "BoundingVolume.h"
#include "Engine.h"

#include <iostream>
#include <random>
std::random_device device;
std::mt19937_64 RNGen(device());
std::uniform_real_distribution<> myrandom(0.0, 1.0);

void Octree::Destroy(TreeNode** ppRoot)
{
  // go to leaf node
  if (*ppRoot == nullptr)
    return;

  Destroy((*ppRoot)->children_);

  // delete children
  for (int i = 0; i < MAX_CHILDREN; ++i)
  {
    if ((*ppRoot)->children_[i] != nullptr)
    {
      delete (*ppRoot)->children_[i];
      (*ppRoot)->children_[i] = nullptr;
    }
  }
  *ppRoot = nullptr;
}

Octree::~Octree()
{
  Destroy(&root_);
}

Octree::Octree(const std::vector<glm::vec3>& vertices, BoundingVolume* bv, int max_triangles) : max_triangles_(max_triangles)
{
  colors_.resize(100);
  for (auto& clr : colors_)
  {
    clr = glm::vec3(myrandom(RNGen), myrandom(RNGen), myrandom(RNGen));
  }
  root_ = BuildRec(vertices, bv, level);
  TreeNode* current = root_;
  MarkLeafNode(current);
}

Octree::TreeNode* Octree::BuildRec(const std::vector<glm::vec3>& vertices, BoundingVolume* bv, int level)
{
  // termination conditions
  if (vertices.size() < max_triangles_ * 3)
  {
    return nullptr;
  }

  TreeNode* newNode = new TreeNode(vertices, bv);

  // create bounding volume in each octant
  for (int i = 0; i < MAX_CHILDREN; ++i)
  {
    BoundingVolume* octant_bv = calculateBounds(static_cast<Octant>(1 << i), newNode->bv_, colors_[level]);
    // need to add relative transform and multiply relative scale
    octant_bv->bv_object->SetPosition(octant_bv->center_ * octant_bv->parent->GetScale() + octant_bv->parent->GetPosition());
    // world space scale * object space scale
    octant_bv->bv_object->SetScale(octant_bv->parent->GetScale() * octant_bv->size_);

    // update vertices for each octant bounding volume
    std::vector<glm::vec3> vertices;
    for (auto& v : newNode->vertices_)
    {
      if (octant_bv->containsPoint(v))
        vertices.push_back(v);
    }
    // recursively create bounding volume for each octant
    newNode->children_[i] = BuildRec(vertices, octant_bv, level + 1);
  }
  //std::cout << "Tree Level: " << level << std::endl;
  return newNode;
}

void Octree::MarkLeafNode(TreeNode* node)
{
  if (!node)
    return;

  for (int i = 0; i < MAX_CHILDREN; ++i)
  {
    if (node->children_[i])
    {
      node->type_ = TreeNodeType::TNT_INTERNAL_NODE;
      MarkLeafNode(node->children_[i]);
    }
  }
}

BoundingVolume* Octree::calculateBounds(Octant octant, BoundingVolume* parentRegion, const glm::vec3& diffuse)
{
  glm::vec3 center = parentRegion->center_;
  if (octant == Octant::O1) {
    return new BV_AABB(center, parentRegion->max_, parentRegion->parent, diffuse);
  }
  if (octant == Octant::O2) {
    return new BV_AABB(glm::vec3(parentRegion->min_.x, center.y, center.z), glm::vec3(center.x, parentRegion->max_.y, parentRegion->max_.z), parentRegion->parent, diffuse);
  }
  if (octant == Octant::O3) {
    return new BV_AABB(glm::vec3(parentRegion->min_.x, parentRegion->min_.y, center.z), glm::vec3(center.x, center.y, parentRegion->max_.z), parentRegion->parent, diffuse);
  }
  if (octant == Octant::O4) {
    return new BV_AABB(glm::vec3(center.x, parentRegion->min_.y, center.z), glm::vec3(parentRegion->max_.x, center.y, parentRegion->max_.z), parentRegion->parent, diffuse);
  }
  if (octant == Octant::O5) {
    return new BV_AABB(glm::vec3(center.x, center.y, parentRegion->min_.z), glm::vec3(parentRegion->max_.x, parentRegion->max_.y, center.z), parentRegion->parent, diffuse);
  }
  if (octant == Octant::O6) {
    return new BV_AABB(glm::vec3(parentRegion->min_.x, center.y, parentRegion->min_.z), glm::vec3(center.x, parentRegion->max_.y, center.z), parentRegion->parent, diffuse);
  }
  if (octant == Octant::O7) {
    return new BV_AABB(parentRegion->min_, center, parentRegion->parent, diffuse);
  }
  if (octant == Octant::O8) {
    return new BV_AABB(glm::vec3(center.x, parentRegion->min_.y, parentRegion->min_.z), glm::vec3(parentRegion->max_.x, center.y, center.z), parentRegion->parent, diffuse);
  }
}

Octree::TreeNode::TreeNode(const std::vector<glm::vec3>& vertices, BoundingVolume* bv) : vertices_(vertices), bv_(bv)
{
  for (int i = 0; i < MAX_CHILDREN; ++i)
  {
    children_[i] = nullptr;
  }
}


