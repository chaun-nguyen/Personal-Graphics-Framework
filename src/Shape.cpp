#include "Shape.h"
#include "transform.h"
#include <algorithm>
#include <iostream>

void Shape::pushquad(std::vector<glm::ivec3>& Tri, int i, int j, int k, int l)
{
  Tri.push_back(glm::ivec3(i, j, k));
  Tri.push_back(glm::ivec3(i, k, l));
}

// Batch up all the data defining a shape to be drawn (example: the
// teapot) as a Vertex Array object (VAO) and send it to the graphics
// card.  Return an OpenGL identifier for the created VAO.
unsigned int Shape::VaoFromTris(std::vector<glm::vec4> Pnt,
  std::vector<glm::vec3> Nrm,
  std::vector<glm::vec2> Tex,
  std::vector<glm::vec3> Tan,
  std::vector<glm::ivec3> Tri)
{
  //std::cout << "VaoFromTrix " << Pnt.size() << " " << Tri.size() << std::endl;
  unsigned int vaoID;
  glCreateVertexArrays(1, &vaoID);
  glBindVertexArray(vaoID);

  GLuint Pbuff;
  glCreateBuffers(1, &Pbuff);
  glBindBuffer(GL_ARRAY_BUFFER, Pbuff);
  glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 4 * Pnt.size(),
    Pnt.data(), GL_STATIC_DRAW);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, 0);
  glBindBuffer(GL_ARRAY_BUFFER, 0);

  if (Nrm.size() > 0) {
    GLuint Nbuff;
    glCreateBuffers(1, &Nbuff);
    glBindBuffer(GL_ARRAY_BUFFER, Nbuff);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 3 * Nrm.size(),
      Nrm.data(), GL_STATIC_DRAW);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
  }

  if (Tex.size() > 0) {
    GLuint Tbuff;
    glCreateBuffers(1, &Tbuff);
    glBindBuffer(GL_ARRAY_BUFFER, Tbuff);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 2 * Tex.size(),
      Tex.data(), GL_STATIC_DRAW);
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
  }

  if (Tan.size() > 0) {
    GLuint Dbuff;
    glCreateBuffers(1, &Dbuff);
    glBindBuffer(GL_ARRAY_BUFFER, Dbuff);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 3 * Tan.size(),
      Tan.data(), GL_STATIC_DRAW);
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
  }

  GLuint Ibuff;
  glCreateBuffers(1, &Ibuff);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, Ibuff);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(int) * 3 * Tri.size(),
    Tri.data(), GL_STATIC_DRAW);

  glBindVertexArray(0);

  return vaoID;
}

void Shape::ComputeSize()
{
  // Compute min/max
  minP = (Pnt[0]).xyz();
  maxP = (Pnt[0]).xyz();
  for (std::vector<glm::vec4>::iterator p = Pnt.begin(); p < Pnt.end(); p++)
    for (int c = 0; c < 3; c++) {
      minP[c] = std::min(minP[c], (*p)[c]);
      maxP[c] = std::max(maxP[c], (*p)[c]);
    }

  center = (maxP + minP) / 2.0f;
  size = 0.0;
  for (int c = 0; c < 3; c++)
    size = std::max(size, (maxP[c] - minP[c]) / 2.0f);

  float s = 1.0f / size;
  modelTr = Scale(s, s, s) * Translate(-center[0], -center[1], -center[2]);
}

void Shape::MakeVAO()
{
  vaoID = VaoFromTris(Pnt, Nrm, Tex, Tan, Tri);
  count = static_cast<unsigned>(Tri.size());
}

void Shape::DrawVAO()
{
  CHECKERROR;
  glBindVertexArray(vaoID);
  CHECKERROR;
  glDrawElements(GL_TRIANGLES, 3 * count, GL_UNSIGNED_INT, 0);
  CHECKERROR;
  glBindVertexArray(0);
}

// spatial data structure, intersection check
Intersection::Intersection() : t(std::numeric_limits<float>::max())
{
}

Interval::Interval() : t0(0.f), N0(glm::vec3(0.f, 0.f, 0.f)), t1(std::numeric_limits<float>::max()), N1(glm::vec3(0.f, 0.f, 0.f))
{
}

Interval::Interval(float _t0, glm::vec3 _N0, float _t1, glm::vec3 _N1)
{
  if (_t0 <= _t1)
  {
    t0 = _t0;
    t1 = _t1;
    N0 = _N0;
    N1 = _N1;
  }
  else
  {
    t0 = _t1;
    t1 = _t0;
    N0 = _N1;
    N1 = _N0;
  }
}

void Interval::empty()
{
  t0 = 0.f;
  t1 = -1.f;
}

bool Interval::isEmpty()
{
  if (t0 > t1)
    return true;
  return false;
}

Interval Interval::intersect(const Ray& ray, const Slab& slab)
{
  Interval interval;
  // ray not parallel with slab planes
  if (glm::dot(slab.N, ray.D) != 0)
  {
    float N_dot_Q = glm::dot(slab.N, ray.Q);
    float N_dot_D = glm::dot(slab.N, ray.D);
    float t0 = -(slab.d0 + N_dot_Q) / N_dot_D;
    float t1 = -(slab.d1 + N_dot_Q) / N_dot_D;
    interval = Interval(t0, -slab.N, t1, slab.N);
  }
  else // ray is parallel to slab planes
  {
    // test if ray is inside slab or not
    float N_dot_Q = glm::dot(slab.N, ray.Q);
    float s0 = N_dot_Q + slab.d0;
    float s1 = N_dot_Q + slab.d1;

    if (s0 * s1 > 0)
    {
      // same sign (Q is outside planes)
      interval.empty();
    }
    else
    {
      // opposite sign (Q is between planes)
      interval = Interval();
    }
  }

  return interval;
}

void Interval::intersect(const Interval& other)
{
  if (t0 < other.t0)
  {
    t0 = other.t0;
    N0 = other.N0;
  }
  if (t1 > other.t1)
  {
    t1 = other.t1;
    N1 = other.N1;
  }
}

float Triangle::AreaTriangle(const glm::vec3& e1, const glm::vec3& e2)
{
  return 0.5f * glm::length(glm::cross(e1, e2));
}

bool Triangle::TrianglePoint(const glm::vec3& P)
{
  glm::vec3 E1 = v2_ - v1_;
  glm::vec3 E2 = v3_ - v1_;

  float S = AreaTriangle(E1, E2);

  float S1 = AreaTriangle(P - v1_, P - v2_);
  float S2 = AreaTriangle(P - v2_, P - v3_);
  float S3 = AreaTriangle(P - v3_, P - v1_);

  // barycentric coordinates
  float u = S1 / S;
  float v = S2 / S;
  float w = S3 / S;

  // inside triangle
  if (u >= 0.f && u <= 1.f &&
    v >= 0.f && v <= 1.f &&
    w >= 0.f && w <= 1.f)
    return true;

  return false;
}

S_Plane::CLASSIFY_POINT_PLANE S_Plane::PlanePoint(glm::vec3 P)
{
  glm::vec3 relative_vector = P - p_;
  float w = glm::dot(normal_, relative_vector);

  if (w > EPSILON) // point is in front of plane
  {
    return CLASSIFY_POINT_PLANE::FRONT;
  }
  if (w < -EPSILON) // point is behind of plane
  {
    return CLASSIFY_POINT_PLANE::BEHIND;
  }

  //if (w <= EPSILON && w >= -EPSILON) // point lies on the plane
  return CLASSIFY_POINT_PLANE::COPLANAR;
}

float Ray::RayPlane(S_Plane* plane)
{
  return glm::dot(-plane->normal_, Q - plane->p_) / glm::dot(plane->normal_, D);
}
