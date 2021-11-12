#pragma once
#include "Object.h"
#include <vector>

constexpr int MAX_CHILDREN = 8;

class BoundingVolume;

class Octree
{
public:
  enum class TreeNodeType : int
  {
    TNT_INTERNAL_NODE,
    TNT_LEAF_NODE,
  };
  struct TreeNode
  {
    TreeNode() = default;
    TreeNode(const std::vector<glm::vec3>& vertices, BoundingVolume* bv);

    TreeNodeType type_ = TreeNodeType::TNT_LEAF_NODE;
    BoundingVolume* bv_ = nullptr;
    TreeNode* children_[MAX_CHILDREN]{ nullptr };
    std::vector<glm::vec3> vertices_; // for power plant will be vertices
  };

public:
  enum class Octant : unsigned char
  {
    O1 = 0x01, // 0b00000001
    O2 = 0x02, // 0b00000010
    O3 = 0x04, // 0b00000100
    O4 = 0x08, // 0b00001000
    O5 = 0x10, // 0b00010000
    O6 = 0x20, // 0b00100000
    O7 = 0x40, // 0b01000000
    O8 = 0x80  // 0b10000000
  };
  Octree() = default;
  ~Octree();
  Octree(const std::vector<glm::vec3>& vertices, BoundingVolume* bv, int max_triangles);

  void Destroy(TreeNode** ppRoot);

  TreeNode* root_ = nullptr;
  std::vector<glm::vec3> colors_;
  int level = 0;
  int max_triangles_ = 0;
private:
  TreeNode* BuildRec(const std::vector<glm::vec3>& vertices, BoundingVolume* bv, int level);
  BoundingVolume* calculateBounds(Octant octant, BoundingVolume* parentRegion, const glm::vec3& diffuse);
  void MarkLeafNode(TreeNode* node);
};