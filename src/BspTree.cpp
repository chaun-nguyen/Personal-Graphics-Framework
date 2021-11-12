#include "BspTree.h"
#include "Shape.h"
#include "AllManagers.h"
#include "Shader.h"
#include <algorithm>
#include <iostream>
#include <numeric>

#include <random>
std::random_device device1;
std::mt19937_64 RNGen1(device1());
std::uniform_real_distribution<> myrandom1(0.0, 1.0);

void BspTree::ClearLeafNodes()
{
  leaf_nodes_.clear();
}


void BspTree::Destroy(TreeNode** ppRoot)
{
  if (*ppRoot == nullptr)
    return;

  Destroy(&(*ppRoot)->l_node);
  Destroy(&(*ppRoot)->r_node);

  (*ppRoot)->parent = nullptr;
  delete *ppRoot;
  *ppRoot = nullptr;
}

glm::vec3 BspTree::GetCenter(const std::vector<glm::vec3>& vertices)
{
  glm::vec3 min_ = glm::vec3(std::numeric_limits<float>::max());
  glm::vec3 max_ = glm::vec3(std::numeric_limits<float>::min());

  for (unsigned i = 0; i < vertices.size(); ++i)
  {
    // 3 axes
    for (unsigned c = 0; c < 3; ++c)
    {
      min_[c] = std::min(min_[c], vertices[i][c]);
      max_[c] = std::max(max_[c], vertices[i][c]);
    }
  }

  glm::vec3 center = (max_ + min_) / 2.0f;

  return center;
}

// choose standard planes Y aligned
S_Plane* BspTree::ChooseSplitPlane(glm::vec3 center, glm::vec3 normal)
{
  return new S_Plane(normal, center);
}

BspTree::TreeNode::TreeNode(S_Plane* plane, const std::vector<glm::vec3>& vertices, const std::vector<unsigned int>& indices) : plane_(plane), vertices_(vertices), indices_(indices)
{
}

BspTree::~BspTree()
{
  TreeNode* current = root_;
  Destroy(&current);
}

void BspTree::CreateVAOs()
{
  CHECKERROR;
  for (int i = 0; i < leaf_nodes_.size(); ++i)
  {
    unsigned VAO, VBO, EBO;
    glCreateVertexArrays(1, &VAO);
    glCreateBuffers(1, &VBO);
    glNamedBufferStorage(VBO, leaf_nodes_[i]->vertices_.size()*sizeof(glm::vec3), leaf_nodes_[i]->vertices_.data(), GL_DYNAMIC_STORAGE_BIT);
    CHECKERROR;
    glEnableVertexArrayAttrib(VAO, 0);
    glVertexArrayVertexBuffer(VAO, 0, VBO, 0, sizeof(glm::vec3));
    glVertexArrayAttribFormat(VAO, 0, 3, GL_FLOAT, GL_FALSE, 0);
    glVertexArrayAttribBinding(VAO, 0, 0);
    CHECKERROR;
    glCreateBuffers(1, &EBO);
    glNamedBufferStorage(EBO, leaf_nodes_[i]->indices_.size() * sizeof(unsigned int), leaf_nodes_[i]->indices_.data(), GL_DYNAMIC_STORAGE_BIT);
    CHECKERROR;
    glVertexArrayElementBuffer(VAO, EBO);
    VAOs_.push_back(VAO);
    CHECKERROR;
  }
}

void BspTree::Draw(ShaderProgram* shaderProgam)
{
  glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
  for (int i = 0; i < leaf_nodes_.size(); ++i)
  {
    CHECKERROR;
    int loc = glGetUniformLocation(shaderProgam->programID, "color");
    glUniform3fv(loc, 1, glm::value_ptr(leaf_nodes_[i]->color_));
    CHECKERROR;
    glBindVertexArray(VAOs_[i]);
    CHECKERROR;
    glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(leaf_nodes_[i]->indices_.size()), GL_UNSIGNED_INT, 0);
    CHECKERROR;
    glBindVertexArray(0);
    CHECKERROR;
  }
}

CLASSIFY_TRIANGLE_PLANE BspTree::ClassifyPolygon(Triangle tri, S_Plane p)
{
  //float d = glm::dot(p.normal_, p.p_);
  //float d1 = glm::dot(glm::vec4(tri.v1_, 1), glm::vec4(p.normal_, -d));
  //float d2 = glm::dot(glm::vec4(tri.v2_, 1), glm::vec4(p.normal_, -d));
  //float d3 = glm::dot(glm::vec4(tri.v3_, 1), glm::vec4(p.normal_, -d));

  glm::vec3 relative_vector1 = tri.v1_ - p.p_;
  glm::vec3 relative_vector2 = tri.v2_ - p.p_;
  glm::vec3 relative_vector3 = tri.v3_ - p.p_;

  float d1 = glm::dot(p.normal_, relative_vector1);
  float d2 = glm::dot(p.normal_, relative_vector2);
  float d3 = glm::dot(p.normal_, relative_vector3);

  if (d1 < -EPSILON && d2 < -EPSILON && d3 < -EPSILON)
  {
    return CLASSIFY_TRIANGLE_PLANE::CTP_BEHIND;
  }
  if (d1 > EPSILON && d2 > EPSILON && d3 > EPSILON)
  {
    return CLASSIFY_TRIANGLE_PLANE::CTP_FRONT;
  }
  if (d1 <= EPSILON && d1 >= -EPSILON &&
    d2 <= EPSILON && d2 >= -EPSILON &&
    d3 <= EPSILON && d3 >= -EPSILON)
  {
    return CLASSIFY_TRIANGLE_PLANE::CTP_COPLANAR;
  }
  return CLASSIFY_TRIANGLE_PLANE::CTP_STRADDLING;
}

void BspTree::SplitTriangle(
  glm::vec3 v1, S_Plane::CLASSIFY_POINT_PLANE v1_flag, 
  glm::vec3 v2, S_Plane::CLASSIFY_POINT_PLANE v2_flag, 
  std::vector<glm::vec3>& out_frontList, std::vector<glm::vec3>& out_backList,
  S_Plane* plane)
{
  if (v1_flag == S_Plane::CLASSIFY_POINT_PLANE::FRONT && v2_flag == S_Plane::CLASSIFY_POINT_PLANE::FRONT)
  {
    out_frontList.push_back(v2);
  }
  else if (v1_flag == S_Plane::CLASSIFY_POINT_PLANE::COPLANAR && v2_flag == S_Plane::CLASSIFY_POINT_PLANE::FRONT)
  {
    out_frontList.push_back(v2);
  }
  else if (v1_flag == S_Plane::CLASSIFY_POINT_PLANE::BEHIND && v2_flag == S_Plane::CLASSIFY_POINT_PLANE::FRONT)
  {
    Ray ray = Ray(v1, glm::normalize(v2 - v1));
    float t = ray.RayPlane(plane);
    glm::vec3 I = ray.eval(t);

    out_frontList.push_back(I);

    out_frontList.push_back(v2);

    out_backList.push_back(I);
  }
  else if (v1_flag == S_Plane::CLASSIFY_POINT_PLANE::FRONT && v2_flag == S_Plane::CLASSIFY_POINT_PLANE::COPLANAR)
  {
    out_frontList.push_back(v2);
  }
  else if (v1_flag == S_Plane::CLASSIFY_POINT_PLANE::COPLANAR && v2_flag == S_Plane::CLASSIFY_POINT_PLANE::COPLANAR)
  {
    out_frontList.push_back(v2);
  }
  else if (v1_flag == S_Plane::CLASSIFY_POINT_PLANE::BEHIND && v2_flag == S_Plane::CLASSIFY_POINT_PLANE::COPLANAR)
  {
    out_frontList.push_back(v2);
    out_backList.push_back(v2);
  }
  else if (v1_flag == S_Plane::CLASSIFY_POINT_PLANE::FRONT && v2_flag == S_Plane::CLASSIFY_POINT_PLANE::BEHIND)
  {
    Ray ray = Ray(v1, glm::normalize(v2 - v1));
    float t = ray.RayPlane(plane);
    glm::vec3 I = ray.eval(t);
    out_frontList.push_back(I);

    out_backList.push_back(I);

    out_backList.push_back(v2);
  }
  else if (v1_flag == S_Plane::CLASSIFY_POINT_PLANE::COPLANAR && v2_flag == S_Plane::CLASSIFY_POINT_PLANE::BEHIND)
  {
    out_backList.push_back(v1);
    out_backList.push_back(v2);
  }
  else if (v1_flag == S_Plane::CLASSIFY_POINT_PLANE::BEHIND && v2_flag == S_Plane::CLASSIFY_POINT_PLANE::BEHIND)
  {
    out_backList.push_back(v2);
  }

}

void BspTree::ClassifyGeometry(
  const std::vector<glm::vec3>& vertices, const std::vector<unsigned int>& indices,
  std::vector<glm::vec3>& out_frontList, std::vector<glm::vec3>& out_backList,
  std::vector<unsigned int>& out_frontIndices, std::vector<unsigned int>& out_backIndices,
  S_Plane* plane)
{
  for (unsigned i = 0; i < indices.size(); i+=3)
  {
    Triangle tri = { vertices[indices[i]],vertices[indices[i + 1]],vertices[indices[i + 2]] };

    CLASSIFY_TRIANGLE_PLANE flag = ClassifyPolygon(tri, *plane);

    switch (flag)
    {
    case CLASSIFY_TRIANGLE_PLANE::CTP_COPLANAR:
    case CLASSIFY_TRIANGLE_PLANE::CTP_FRONT:
      out_frontList.push_back(tri.v1_);
      out_frontIndices.push_back(out_frontList.size() - 1);

      out_frontList.push_back(tri.v2_);
      out_frontIndices.push_back(out_frontList.size() - 1);

      out_frontList.push_back(tri.v3_);
      out_frontIndices.push_back(out_frontList.size() - 1);
      break;
    case CLASSIFY_TRIANGLE_PLANE::CTP_BEHIND:
      out_backList.push_back(tri.v1_);
      out_backIndices.push_back(out_backList.size() - 1);

      out_backList.push_back(tri.v2_);
      out_backIndices.push_back(out_backList.size() - 1);

      out_backList.push_back(tri.v3_);
      out_backIndices.push_back(out_backList.size() - 1);
      break;
    case CLASSIFY_TRIANGLE_PLANE::CTP_STRADDLING:
      unsigned frontListSize = out_frontList.size();
      unsigned backListSize = out_backList.size();
      std::vector<glm::vec3> triList;
      triList.push_back(tri.v1_);
      triList.push_back(tri.v2_);
      triList.push_back(tri.v3_);
      for (int i = 0; i < 3; ++i)
      {
        glm::vec3 v1;
        glm::vec3 v2;
        if (i == 2)
        {
          v1 = triList[i];
          v2 = triList[0];
        }
        else
        {
          v1 = triList[i];
          v2 = triList[i + 1];
        }
        DirectedEdge edge = { v2 - v1,v1,v2 };
        SplitTriangle(
          edge.start_, plane->PlanePoint(edge.start_),
          edge.end_, plane->PlanePoint(edge.end_),
          out_frontList, out_backList,
          plane);
      }
      if (out_frontList.size() == frontListSize + 3)
      {
        out_frontIndices.push_back(frontListSize);
        out_frontIndices.push_back(frontListSize + 1);
        out_frontIndices.push_back(frontListSize + 2);
      }
      else if (out_frontList.size() == frontListSize + 4)
      {
        out_frontIndices.push_back(frontListSize);
        out_frontIndices.push_back(frontListSize + 1);
        out_frontIndices.push_back(frontListSize + 2);

        out_frontIndices.push_back(frontListSize);
        out_frontIndices.push_back(frontListSize + 2);
        out_frontIndices.push_back(frontListSize + 3);
      }
      if (out_backList.size() == backListSize + 3)
      {
        out_backIndices.push_back(backListSize);
        out_backIndices.push_back(backListSize + 1);
        out_backIndices.push_back(backListSize + 2);
      }
      else if (out_backList.size() == backListSize + 4)
      {
        out_backIndices.push_back(backListSize);
        out_backIndices.push_back(backListSize + 1);
        out_backIndices.push_back(backListSize + 2);

        out_backIndices.push_back(backListSize);
        out_backIndices.push_back(backListSize + 2);
        out_backIndices.push_back(backListSize + 3);
      }
      break;
    }
  }

  //for (auto& vertex : vertices)
  //{
  //  S_Plane::CLASSIFY_POINT_PLANE flag = plane->PlanePoint(vertex);
  //  switch (flag)
  //  {
  //  case S_Plane::CLASSIFY_POINT_PLANE::COPLANAR:
  //  case S_Plane::CLASSIFY_POINT_PLANE::FRONT:
  //    out_frontList.push_back(vertex);
  //    break;
  //  case S_Plane::CLASSIFY_POINT_PLANE::BEHIND:
  //    out_backList.push_back(vertex);
  //    break;
  //  }
  //}
}

void BspTree::BuildRec(TreeNode** ppRoot, TreeNode* pParent, 
  const std::vector<unsigned int>& indices, const std::vector<glm::vec3>& vertices, 
  int level, int start_index)
{
  // terminating condition
  if (vertices.size() < max_triangles_ * 3)
    return;

  // height of tree
  if (level > 10)
    return;

  // a good dividing plane cannot be found
  if (pParent)
  {
    if (pParent->vertices_.size() < vertices.size())
      return;
  }

  glm::vec3 center = GetCenter(vertices);

  *ppRoot = new TreeNode(new S_Plane(), vertices, indices);
  (*ppRoot)->plane_->p_ = center;
  (*ppRoot)->parent = pParent;

  std::vector<glm::vec3> frontList;
  std::vector<glm::vec3> backList;
  std::vector<unsigned int> frontIndices;
  std::vector<unsigned int> backIndices;

  for (unsigned i = start_index; i < aligned_axes_plane_normal_.size(); ++i)
  {
    // choose plane
    (*ppRoot)->plane_->normal_ = aligned_axes_plane_normal_[i % to_integral(PLANE_TYPE::Total)];
    ClassifyGeometry(vertices, indices,
      frontList, backList, 
      frontIndices, backIndices, 
      (*ppRoot)->plane_);

    // make sure front and back are even
    float ratio1 = static_cast<float>(frontList.size()) / vertices.size();
    float ratio2 = static_cast<float>(backList.size()) / vertices.size();
    if (std::abs(ratio1 - ratio2) > 0.7)
    {
      (*ppRoot)->plane_->p_ = frontList.size() > backList.size() ? GetCenter(frontList) : GetCenter(backList);
      // try again
      frontList.clear();
      backList.clear();
      frontIndices.clear();
      backIndices.clear();
      continue;
    }

    break;
  }
  BuildRec(&(*ppRoot)->l_node, *ppRoot, frontIndices, frontList, level + 1, start_index + 1);
  BuildRec(&(*ppRoot)->r_node, *ppRoot, backIndices, backList, level + 1, start_index + 1);

  if ((*ppRoot)->l_node == nullptr && (*ppRoot)->r_node == nullptr)
  {
    (*ppRoot)->color_ = glm::vec3(myrandom1(RNGen1), myrandom1(RNGen1), myrandom1(RNGen1));
    leaf_nodes_.push_back(*ppRoot);
  }
  else if ((*ppRoot)->l_node == nullptr)
  {
    (*ppRoot)->color_ = glm::vec3(myrandom1(RNGen1), myrandom1(RNGen1), myrandom1(RNGen1));
    leaf_nodes_.push_back((*ppRoot));
  }
  else if ((*ppRoot)->r_node == nullptr)
  {
    (*ppRoot)->color_ = glm::vec3(myrandom1(RNGen1), myrandom1(RNGen1), myrandom1(RNGen1));
    leaf_nodes_.push_back((*ppRoot));
  }

}

BspTree::BspTree(const std::vector<unsigned int>& indices, const std::vector<glm::vec3>& vertices, int max_triangles) : max_triangles_(max_triangles), aligned_axes_plane_normal_(std::vector<glm::vec3>(to_integral(PLANE_TYPE::Total)))
{
  aligned_axes_plane_normal_[to_integral(PLANE_TYPE::YZ_PLANE)] = { 1.f,0.f,0.f };
  aligned_axes_plane_normal_[to_integral(PLANE_TYPE::XY_PLANE)] = { 0.f,0.f,1.f };
  aligned_axes_plane_normal_[to_integral(PLANE_TYPE::ZX_PLANE)] = { 0.f,1.f,0.f };
  aligned_axes_plane_normal_[to_integral(PLANE_TYPE::FLIP_YZ_PLANE)] = { -1.f,0.f,0.f };
  aligned_axes_plane_normal_[to_integral(PLANE_TYPE::FLIP_XY_PLANE)] = { 0.f,0.f,-1.f };
  aligned_axes_plane_normal_[to_integral(PLANE_TYPE::FLIP_ZX_PLANE)] = { 0.f,-1.f,0.f };
  aligned_axes_plane_normal_[to_integral(PLANE_TYPE::PLANE1)] = { 1.f,1.f,1.f };
  aligned_axes_plane_normal_[to_integral(PLANE_TYPE::PLANE2)] = { 0.f,1.f,1.f };
  aligned_axes_plane_normal_[to_integral(PLANE_TYPE::PLANE3)] = { 1.f,0.f,1.f };
  aligned_axes_plane_normal_[to_integral(PLANE_TYPE::PLANE4)] = { 1.f,1.f,0.f };
  aligned_axes_plane_normal_[to_integral(PLANE_TYPE::FLIP_PLANE1)] = { -1.f,-1.f,-1.f };
  aligned_axes_plane_normal_[to_integral(PLANE_TYPE::FLIP_PLANE2)] = { 0.f,-1.f,-1.f };
  aligned_axes_plane_normal_[to_integral(PLANE_TYPE::FLIP_PLANE3)] = { -1.f,0.f,-1.f };
  aligned_axes_plane_normal_[to_integral(PLANE_TYPE::FLIP_PLANE4)] = { -1.f,-1.f,0.f };
  BuildRec(&root_, nullptr, indices, vertices, level, 0);

  int result = 0;
  size_t max = 0;
  for (int i = 0; i < leaf_nodes_.size(); ++i)
  {
    result += leaf_nodes_[i]->vertices_.size();
    if (max < leaf_nodes_[i]->vertices_.size())
    max = std::max(max, leaf_nodes_[i]->vertices_.size());
  }
  std::cout << "Total vertices in leaf node: " << result << std::endl;

  CreateVAOs();

}


