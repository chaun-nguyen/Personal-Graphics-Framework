#include "GJK.h"
#include "Octree.h"
#include "Engine.h"
#include "Shader.h"

static const glm::vec3 ORIGIN = { 0.f,0.f,0.f };

bool GJK::DetectCollision_BroadPhase(Object* S, Octree::TreeNode* node)
{
  // aabb-aabb intersection
  if (!S->bv->intersect(node->bv_))
    return false;

  // intersection check with internal nodes
  for (int i = 0; i < MAX_CHILDREN; ++i)
  {
    // validate children
    if (node->children_[i])
    {
      if (DetectCollision_MidPhase(S, node->children_[i]))
      {
        return true;
      }
    }
  }

  // no intersection at all
  return false;
}

bool GJK::DetectCollision_MidPhase(Object* S, Octree::TreeNode* node)
{
  if (node->type_ == Octree::TreeNodeType::TNT_LEAF_NODE)
  {
    if (DetectCollision_NarrowPhase(S, node))
    {
      // render polygons of tree node & sphere in RED color
      auto* om = Engine::managers_.GetManager<ObjectManager*>();
      om->AddBoundingVolumeGJK(node->bv_);
      return true;
    }
    return false;
  }

  // aabb-aabb intersection
  if (!S->bv->intersect(node->bv_))
    return false;

  // recursively through children node
  for (int i = 0; i < MAX_CHILDREN; ++i)
  {
    // validate children
    if (node->children_[i])
    {
      if (DetectCollision_MidPhase(S, node->children_[i]))
        return true;
    }
  }

  // no intersection at all
  return false;
}

// perform GJK algorithm
bool GJK::DetectCollision_NarrowPhase(Object* S, Octree::TreeNode* node)
{
  // hit the leaf node
  if (S->bv->intersect(node->bv_))
  {
    std::vector<glm::vec4> S_Pnt;
    S_Pnt.resize(S->bv->bv_object->shape->Pnt.size());
    std::vector<glm::vec4> node_Pnt;
    node_Pnt.resize(node->bv_->bv_object->shape->Pnt.size());

    // transform vertices to world space coordinates
    for (int i = 0; i < S->bv->bv_object->shape->Pnt.size(); ++i)
    {
      S_Pnt[i] = S->bv->bv_object->modelTr * S->bv->bv_object->shape->Pnt[i];
    }
    for (int i = 0; i < node->bv_->bv_object->shape->Pnt.size(); ++i)
    {
      node_Pnt[i] = node->bv_->bv_object->modelTr * node->bv_->bv_object->shape->Pnt[i];
    }


    // perform GJK algorithm
    return Run(
      S_Pnt,
      S->bv->bv_object->shape->Tri,
      S->bv->center_,
      node_Pnt,
      node->bv_->bv_object->shape->Tri,
      node->bv_->center_
    );
  }
    
  return false;
}

// closet point to X is P
glm::vec3 GJK::ClosestPointOnPoint(const glm::vec3& X, const glm::vec3& P)
{
  return P;
}

glm::vec3 GJK::ClosestPointOnLineSegment(const glm::vec3& X, const glm::vec3& P0, const glm::vec3& P1)
{
  if (glm::dot(X - P0, P1 - P0) <= 0)
    return ClosestPointOnPoint(X, P0); //return P0

  if (glm::dot(X - P1, P0 - P1) <= 0)
    return ClosestPointOnPoint(X, P1); //return P1;

  glm::vec3 a = P1 - P0;
  glm::vec3 b = X - P0;
  float L = glm::length(a);
  float dotProduct = glm::dot(a, b) / (L*L);

  return P0 + a * dotProduct;
}

glm::vec3 GJK::ClosestPointOnTriangle(const glm::vec3& X, const glm::vec3& P0, const glm::vec3& P1, const glm::vec3& P2)
{
  // 1. test for region of P0
  if (glm::dot(X - P0, P1 - P0) <= 0 && glm::dot(X - P0, P2 - P0) <= 0)
    return ClosestPointOnPoint(X, P0);

  // 2. test for region of P1
  if (glm::dot(X - P1, P0 - P1) <= 0 && glm::dot(X - P1, P2 - P1) <= 0)
    return ClosestPointOnPoint(X, P1);

  // 3. test for edge P0-P1
  glm::vec3 N = glm::cross(P1 - P0, P2 - P0);
  glm::vec3 A = glm::cross(P1 - P0, N);
  if (glm::dot(X - P0, P1 - P0) >= 0 && glm::dot(X - P0, A) >= 0 && glm::dot(X - P1, P0 - P1))
  {
    return ClosestPointOnLineSegment(X, P0, P1);
  }

  // 4. test for region of P2
  if (glm::dot(X - P2, P0 - P2) <= 0 && glm::dot(X - P2, P1 - P2) <= 0)
    return ClosestPointOnPoint(X, P2);

  // 5. test for edge P1-P2
  N = glm::cross(P2 - P1, P0 - P1);
  A = glm::cross(P2 - P1, N);
  if (glm::dot(X - P1, P2 - P1) >= 0 && glm::dot(X - P1, A) >= 0 && glm::dot(X - P2, P1 - P2))
  {
    return ClosestPointOnLineSegment(X, P1, P2);
  }

  // 6. test for edge P2-P0
  N = glm::cross(P1 - P0, P2 - P0);
  A = glm::cross(N, P2 - P0);
  if (glm::dot(X - P0, P2 - P0) >= 0 && glm::dot(X - P0, A) >= 0 && glm::dot(X - P2, P0 - P2))
  {
    return ClosestPointOnLineSegment(X, P2, P0);
  }

  // 7. test for triangle
  // X is inside triangle
  Triangle tri = { P0, P1, P2 };
  // calculate barycentric coordinates
  glm::vec3 E1 = tri.v2_ - tri.v1_;
  glm::vec3 E2 = tri.v3_ - tri.v1_;

  float S = tri.AreaTriangle(E1, E2);
  float S1 = tri.AreaTriangle(X - tri.v1_, X - tri.v2_);
  float S2 = tri.AreaTriangle(X - tri.v2_, X - tri.v3_);
  float S3 = tri.AreaTriangle(X - tri.v3_, X - tri.v1_);

  // barycentric coordinates
  float u = S1 / S;
  float v = S2 / S;
  float w = S3 / S;

  if (v == 1.0f)
  {
    return ClosestPointOnPoint(X, P0);
  }
  if (w == 1.0f)
  {
    return ClosestPointOnPoint(X, P1);
  }
  if (u == 1.0f)
  {
    return ClosestPointOnPoint(X, P2);
  }
  if (v == 0.0f || v == std::min(std::min(u,v),w))
  {
    return ClosestPointOnLineSegment(X, P1, P2);
  }
  if (w == 0.0f || w == std::min(std::min(u, v), w))
  {
    return ClosestPointOnLineSegment(X, P2, P0);
  }
  if (u == 0.0f || u == std::min(std::min(u, v), w))
  {
    return ClosestPointOnLineSegment(X, P0, P1);
  }
}

glm::vec3 GJK::ClosestPointOnTetraHedron(const glm::vec3& X, const glm::vec3& P0, const glm::vec3& P1,
  const glm::vec3& P2, const glm::vec3& P3)
{
  // assume X is the closest point
  glm::vec3 closestPoint = X;

  // test for region of triangles
  // P0, P1, P2
  if (S_Plane(glm::cross(P1 - P0, P2 - P0), P0).PlanePoint(X) == S_Plane::CLASSIFY_POINT_PLANE::FRONT)
    closestPoint = ClosestPointOnTriangle(X, P0, P1, P2);

  // P0, P2, P3
  if (S_Plane(glm::cross(P2 - P0, P3 - P0), P0).PlanePoint(X) == S_Plane::CLASSIFY_POINT_PLANE::FRONT)
    closestPoint = ClosestPointOnTriangle(X, P0, P2, P3);

  // P0, P3, P1
  if (S_Plane(glm::cross(P3 - P0, P1 - P0), P0).PlanePoint(X) == S_Plane::CLASSIFY_POINT_PLANE::FRONT)
    closestPoint = ClosestPointOnTriangle(X, P0, P3, P1);

  // P1, P2, P3
  if (S_Plane(glm::cross(P2 - P1, P3 - P1), P1).PlanePoint(X) == S_Plane::CLASSIFY_POINT_PLANE::FRONT)
    closestPoint = ClosestPointOnTriangle(X, P1, P2, P3);

  return closestPoint;
}

bool GJK::Run(
  const std::vector<glm::vec4>& objectVertices,
  const std::vector<glm::ivec3>& objectIndices,
  const glm::vec3& objectCenter,
  const std::vector<glm::vec4>& modelVertices,
  const std::vector<glm::ivec3>& modelIndices,
  const glm::vec3& modelCenter)
{
  auto* om = Engine::managers_.GetManager<ObjectManager*>();

  // first choose a direction
  om->gjkController.simplex->dir_ = glm::normalize(objectCenter - modelCenter);

  glm::vec3 simplexPoint = supportFunction(om->gjkController.simplex->dir_,
    objectVertices, objectIndices, modelVertices, modelIndices);

  om->gjkController.simplex->Add(simplexPoint);

  // next direction is toward the origin
  om->gjkController.simplex->dir_ = glm::normalize(ORIGIN - om->gjkController.simplex->Get(0));

  while (true)
  {
    // get new support point
    glm::vec3 newSupportPoint = supportFunction(om->gjkController.simplex->dir_,
      objectVertices, objectIndices, modelVertices, modelIndices);

    float dotProduct = glm::dot(newSupportPoint, om->gjkController.simplex->dir_);

    // two shapes dont intersect
    if (dotProduct < 0)
    {
      om->gjkController.simplex->vertices_.clear();
      om->gjkController.simplex->indices_.clear();
      return false;
    }

    om->gjkController.simplex->Add(newSupportPoint);

    if (handleSimplex(om->gjkController.simplex, om->gjkController.simplex->dir_))
      return true;
  }
}

glm::vec3 GJK::supportFunction(glm::vec3 dir, 
  const std::vector<glm::vec4>& objectVertices,
  const std::vector<glm::ivec3>& objectIndices,
  const std::vector<glm::vec4>& modelVertices,
  const std::vector<glm::ivec3>& modelIndices)
{
  return getFurthestPoint(dir, objectVertices, objectIndices) - getFurthestPoint(-dir, modelVertices, modelIndices);
}

glm::vec3 GJK::getFurthestPoint(glm::vec3 dir, 
                                const std::vector<glm::vec4>& vertices,
                                const std::vector<glm::ivec3>& indices)
{
  unsigned saved_index;
  unsigned saved_comp;
  float max = -100000.f;

  for (unsigned i = 0; i < indices.size(); ++i)
  {
    for (unsigned c = 0; c < 3; ++c)
    {
      glm::vec3 vertex = glm::vec3(vertices[indices[i][c]]);
      float dotProduct = glm::dot(vertex, dir);
      if (max < dotProduct)
      {
        max = dotProduct;
        saved_index = i;
        saved_comp = c;
      }
    }
  }

  return glm::vec3(vertices[indices[saved_index][saved_comp]]);
}

bool GJK::handleSimplex(Simplex* simplex, glm::vec3& dir)
{
  // line case
  if (simplex->vertices_.size() == 2)
  {
    glm::vec3 B = simplex->Get(0);
    glm::vec3 A = simplex->Get(1);
    glm::vec3 resultP = ClosestPointOnLineSegment(ORIGIN, B, A);

    // line segment contains origin
    if (resultP == ORIGIN)
      return true;

    // update new direction
    dir = glm::normalize(ORIGIN - resultP);

    return false;
  }

  // triangle case
  if (simplex->vertices_.size() == 3)
  {
    glm::vec3 C = simplex->Get(0);
    glm::vec3 B = simplex->Get(1);
    glm::vec3 A = simplex->Get(2);
    glm::vec3 resultP = ClosestPointOnTriangle(ORIGIN, C, B, A);

    if (resultP == ORIGIN)
      return true;

    // update new direction
    dir = glm::normalize(ORIGIN - resultP);

    return false;
  }

  // tetrahedron case
  if (simplex->vertices_.size() == 4)
  {
    glm::vec3 D = simplex->Get(0);
    glm::vec3 C = simplex->Get(1);
    glm::vec3 B = simplex->Get(2);
    glm::vec3 A = simplex->Get(3);
    glm::vec3 resultP = ClosestPointOnTetraHedron(ORIGIN, D, C, B, A);

    if (resultP == ORIGIN)
      return true;

    return true;
  }

  return false;
}

void Simplex::Add(glm::vec3 P)
{
  vertices_.push_back(P);
  indices_.push_back(vertices_.size() - 1);
}

glm::vec3 Simplex::Get(unsigned index)
{
  return vertices_[indices_[index]];
}

void Simplex::CreateVAOs()
{
  Triangulate();
  unsigned VBO, EBO;
  CHECKERROR;
  glCreateVertexArrays(1, &VAOs_);
  glCreateBuffers(1, &VBO);
  glNamedBufferStorage(VBO, vertices_.size() * sizeof(glm::vec3), vertices_.data(), GL_DYNAMIC_STORAGE_BIT);
  CHECKERROR;
  glEnableVertexArrayAttrib(VAOs_, 0);
  glVertexArrayVertexBuffer(VAOs_, 0, VBO, 0, sizeof(glm::vec3));
  glVertexArrayAttribFormat(VAOs_, 0, 3, GL_FLOAT, GL_FALSE, 0);
  glVertexArrayAttribBinding(VAOs_, 0, 0);
  CHECKERROR;
  glCreateBuffers(1, &EBO);
  glNamedBufferStorage(EBO, indices_.size() * sizeof(unsigned int), indices_.data(), GL_DYNAMIC_STORAGE_BIT);
  CHECKERROR;
  glVertexArrayElementBuffer(VAOs_, EBO);
  CHECKERROR;
  vaoFlag_ = true;
}

void Simplex::Triangulate()
{
  indices_.push_back(2);
  indices_.push_back(1);
  indices_.push_back(2);
  indices_.push_back(3);
  indices_.push_back(0);
  indices_.push_back(3);
  indices_.push_back(1);
  indices_.push_back(0);

}

void Simplex::Draw(ShaderProgram* shaderProgram)
{
  glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
  CHECKERROR;
  glBindVertexArray(VAOs_);
  CHECKERROR;
  glDrawElements(GL_TRIANGLES, indices_.size(), GL_UNSIGNED_INT, 0);
  CHECKERROR;
  glBindVertexArray(0);
  CHECKERROR;
}
