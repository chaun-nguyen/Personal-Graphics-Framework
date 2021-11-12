#pragma once
#include "Object.h"
#include <vector>
#include "Shape.h"

class ShaderProgram;

class BspTree
{
private:
  struct TreeNode;
public:
  enum class PLANE_TYPE
  {
    YZ_PLANE,
    XY_PLANE,
    ZX_PLANE,
    FLIP_YZ_PLANE,
    FLIP_XY_PLANE,
    FLIP_ZX_PLANE,
    PLANE1,
    PLANE2,
    PLANE3,
    PLANE4,
    FLIP_PLANE1,
    FLIP_PLANE2,
    FLIP_PLANE3,
    FLIP_PLANE4,

    Total
  };

  BspTree() = default;
  ~BspTree();
  BspTree(const std::vector<unsigned int>& indices, const std::vector<glm::vec3>& vertices, int max_triangles);

  void Destroy(TreeNode** ppRoot);
  void ClearLeafNodes();

  void Draw(ShaderProgram* shaderProgam);

  TreeNode* root_ = nullptr;
  int level = 0;
  int max_triangles_ = 0;
  std::vector<glm::vec3> aligned_axes_plane_normal_;
  std::vector<TreeNode*> leaf_nodes_;
private:
  // draw tree
  std::vector<unsigned> VAOs_;
  void CreateVAOs();

  void SplitTriangle(
    glm::vec3 v1, S_Plane::CLASSIFY_POINT_PLANE v1_flag,
    glm::vec3 v2, S_Plane::CLASSIFY_POINT_PLANE v2_flag,
    std::vector<glm::vec3>& out_frontList, std::vector<glm::vec3>& out_backList,
    S_Plane* plane);

  void ClassifyGeometry(
    const std::vector<glm::vec3>& vertices, const std::vector<unsigned int>& indices,
    std::vector<glm::vec3>& out_frontList, std::vector<glm::vec3>& out_backList,
    std::vector<unsigned int>& out_frontIndices, std::vector<unsigned int>& out_backIndices,
    S_Plane* plane);

  glm::vec3 GetCenter(const std::vector<glm::vec3>& vertices);



  S_Plane* ChooseSplitPlane(glm::vec3 center, glm::vec3 normal);

  void BuildRec(TreeNode** ppRoot, TreeNode* pParent,
    const std::vector<unsigned int>& indices, 
    const std::vector<glm::vec3>& vertices,
    int level, int start_index
  );

  CLASSIFY_TRIANGLE_PLANE ClassifyPolygon(Triangle tri, S_Plane p);

  struct DirectedEdge
  {
    glm::vec3 dir_;
    glm::vec3 start_;
    glm::vec3 end_;
  };

  struct TreeNode
  {
    TreeNode() = default;
    TreeNode(S_Plane* plane, const std::vector<glm::vec3>& vertices, const std::vector<unsigned int>& indices);
    S_Plane* plane_ = nullptr;
    std::vector<glm::vec3> vertices_;
    std::vector<unsigned int> indices_;
    glm::vec3 color_;
    TreeNode* l_node = nullptr;
    TreeNode* r_node = nullptr;
    TreeNode* parent = nullptr;
  };
};
