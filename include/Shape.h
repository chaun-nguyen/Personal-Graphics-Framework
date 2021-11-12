#pragma once
#include "LibHeader.h"
#include "BoundingVolume.h"
#include <vector>

constexpr float EPSILON = 0.00000001f;

class Shape;
class Ray;
struct Slab;

class Intersection
{
public:
  Intersection();

  float t;
  Shape* object = nullptr;
  glm::vec3 P = {};
  glm::vec3 N = {};
};

struct Slab
{
  Slab() = default;
  Slab(glm::vec3 _N, float _d0, float _d1) : N(_N), d0(_d0), d1(_d1) {}
  glm::vec3 N = {}; // normal
  float d0 = 0.f;
  float d1 = 0.f;
};

class Interval
{
public:
  float t0; // begin point along a ray
  glm::vec3 N0; // normal at t0
  float t1; // end point along a ray
  glm::vec3 N1; // normal at t1

  Interval();
  Interval(float _t0, glm::vec3 _N0, float _t1, glm::vec3 _N1);

  void empty();
  bool isEmpty();

  // intersect(ray and slab)
  Interval intersect(const Ray& ray, const Slab& slab);

  // intersect(other)
  void intersect(const Interval& other);
};

class Triangle
{
public:
  Triangle() = default;
  Triangle(glm::vec3 v1, glm::vec3 v2, glm::vec3 v3) : v1_(v1), v2_(v2), v3_(v3)
  {}

  float AreaTriangle(const glm::vec3& e1, const glm::vec3& e2);
  bool TrianglePoint(const glm::vec3& P);

  glm::vec3 v1_ = {};
  glm::vec3 v2_ = {};
  glm::vec3 v3_ = {};
};

enum class CLASSIFY_TRIANGLE_PLANE : int
{
  CTP_FRONT,
  CTP_BEHIND,
  CTP_COPLANAR,
  CTP_STRADDLING,

  Total
};

class S_Plane
{
public:
  enum class CLASSIFY_POINT_PLANE
  {
    Invalid = -1,
    FRONT,
    BEHIND,
    COPLANAR,

    Total
  };
  S_Plane() = default;
  // normal = (a,b,c)
  // d = -(a*x0 + b*y0 + c*z0)
  // (n.x, n.y, n.z, d)
  S_Plane(glm::vec3 normal, glm::vec3 P) : normal_(normal), p_(P)
  {}

  S_Plane(glm::vec3 normal, float d) : normal_(normal), d_(d)
  {}

  CLASSIFY_POINT_PLANE PlanePoint(glm::vec3 P);

  glm::vec3 normal_ = {};
  glm::vec3 p_ = {};
  float d_ = 0.f;
};

class Ray
{
public:
  Ray(glm::vec3 e, glm::vec3 dir) : Q(e), D(dir) {}

  Ray() = default;
  ~Ray() = default;

  glm::vec3 eval(float t) const { return Q + D * t; }

  float RayPlane(S_Plane* plane);

  glm::vec3 Q = {};
  glm::vec3 D = {};
};

class Shape
{
public:
  // Constructor and destructor
  Shape() = default;
  virtual ~Shape() = default;

  // spatial data structure, bounding volume hierarchy
  virtual bool intersect(const Ray& ray, Intersection& intersection) = 0;
  virtual BoundingVolume* bbox() = 0;
  

  virtual void ComputeSize();
  virtual void MakeVAO();
  virtual void DrawVAO();

  virtual void pushquad(std::vector<glm::ivec3>& Tri, int i, int j, int k, int l);
  virtual unsigned int VaoFromTris(std::vector<glm::vec4> Pnt,
    std::vector<glm::vec3> Nrm,
    std::vector<glm::vec2> Tex,
    std::vector<glm::vec3> Tan,
    std::vector<glm::ivec3> Tri);

  // The OpenGL identifier of this VAO
  unsigned int vaoID;

  // Data arrays
  std::vector<glm::vec4> Pnt;
  std::vector<glm::vec3> Nrm;
  std::vector<glm::vec2> Tex;
  std::vector<glm::vec3> Tan;

  // Lighting information
  glm::vec3 diffuseColor, specularColor;
  float shininess;

  // Geometry defined by indices into data arrays
  std::vector<glm::ivec3> Tri;
  unsigned int count;

  // Defined by SetTransform by scanning data arrays
  glm::vec3 minP, maxP;
  glm::vec3 center;
  float size;
  glm::mat4 modelTr;

  Object* parent = nullptr;
private:
};
