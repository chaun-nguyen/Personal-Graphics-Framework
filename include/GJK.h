#pragma once
#include "Shape.h"
#include "Octree.h"
#include <vector>

class ShaderProgram;

class Simplex
{
public:
  Simplex() = default;

  void Draw(ShaderProgram* shaderProgram);

  void Add(glm::vec3 P);
  glm::vec3 Get(unsigned index);
  void CreateVAOs();
  void Triangulate();

  std::vector<glm::vec3> vertices_;
  std::vector<unsigned int> indices_;
  glm::vec3 dir_;
  bool vaoFlag_ = false;
private:
  // draw tree
  unsigned VAOs_;
};

namespace GJK
{
  bool Run(
    const std::vector<glm::vec4>& objectVertices,
    const std::vector<glm::ivec3>& objectIndices,
    const glm::vec3& objectCenter,
    const std::vector<glm::vec4>& modelVertices,
    const std::vector<glm::ivec3>& modelIndices,
    const glm::vec3& modelCenter
  );
  glm::vec3 supportFunction(
    glm::vec3 dir,
    const std::vector<glm::vec4>& objectVertices,
    const std::vector<glm::ivec3>& objectIndices,
    const std::vector<glm::vec4>& modelVertices,
    const std::vector<glm::ivec3>& modelIndices
  );
  glm::vec3 getFurthestPoint(glm::vec3 dir, 
    const std::vector<glm::vec4>& vertices,
    const std::vector<glm::ivec3>& indices
  );
  bool handleSimplex(
    Simplex* simplex, glm::vec3& dir
  );
  bool DetectCollision_BroadPhase(Object* S, Octree::TreeNode* node);
  bool DetectCollision_MidPhase(Object* S, Octree::TreeNode* node);
  bool DetectCollision_NarrowPhase(Object* S, Octree::TreeNode* node);
  glm::vec3 ClosestPointOnPoint(const glm::vec3& X, const glm::vec3& P);
  glm::vec3 ClosestPointOnLineSegment(const glm::vec3& X, const glm::vec3& P0, const glm::vec3& P1);
  glm::vec3 ClosestPointOnTriangle(const glm::vec3& X, const glm::vec3& P0, const glm::vec3& P1, const glm::vec3& P2);
  glm::vec3 ClosestPointOnTetraHedron(const glm::vec3& X, const glm::vec3& P0, const glm::vec3& P1, const glm::vec3& P2, const glm::vec3& P3);
}
