#pragma once
#include <stack>
#include <vector>
#include <Eigen/Dense>
#include <glm/glm.hpp>

class ShaderProgram;

enum class ArcLengthFunctionType
{
  ForwardDifferencing = 0,
  AdaptiveApproach = 1,
  Analytic = 2,

  Total
};

class Spline
{
public:
  Spline() = default;
  ~Spline() = default;

  Spline(const std::vector<glm::vec3>& pts);

  void Draw(ShaderProgram* shader);

  void Construct();

  void Update();
  void UpdateVBO();

  glm::vec3 DeCastlejau(float t);
  glm::vec3 SplineInterpolate(float t, int degree);

  void Add(glm::vec3 point);

  std::vector<glm::vec3>& GetControlPoints();
  std::vector<glm::vec3>& GetInterpolatePoints();

  bool dirtyFlag = false;

  // arc-length function
  // forward differencing & adaptive approach
  float BinarySearchForLowestIndex(float s);
  float getUFromTable(float index, float s);
  glm::vec3 getInterpolatedPositionOnSpaceCurve(float s);

  int method;
  float deltaU;

  // size of interpolating step for the space curve
  float alpha;
private:
  void SetUpCurveVAO();
  void SetUpControlPointsVAO();

  // rendering data
  unsigned curveVAO;
  unsigned curveVBO;
  unsigned curveEBO;
  std::vector<unsigned> curveIndices;

  unsigned controlPointVAO;
  unsigned controlPointVBO;
  unsigned controlPointEBO;
  std::vector<unsigned> controlPointIndices;

  // spline data
  std::vector<glm::vec3> controlPoints;
  std::vector<glm::vec3> interpolatePoints;

  // spline interpolating helper functions
  float EvalTruncatedFunction(float t, float c, int d);
  float TakeSecondDerivative(float t, int pow);
  float TakeFirstDerivative(float t, int pow);
  float TakeSecondDerivativeTruncatedFunc(float t, float c, int d);
  void CreateLinearSystem();
  void PopulateBxByAndSolveLinearSystem();
  void UpdateSpline();

  // spline interpolating data
  std::vector<float> k;
  Eigen::MatrixXf linearSystem;
  Eigen::VectorXf Bx;
  Eigen::VectorXf By;
  Eigen::VectorXf resX;
  Eigen::VectorXf resY;

  // arc-length function
  void SetUpArcLengthFunction();

  // forward differencing
  void CreateTableForwardDifferencing();
  struct TableData
  {
    glm::vec3 Pu;  // Point on spline curve
    float s = 0.f; // distance
    float u = 0.f; // interpolate factor
  };
  std::vector<TableData> table;

  // adaptive approach
  void CreateSortedSegmentList();
  float epsilon = 0.0001f;
  float gamma = 0.01f;
  struct SegmentData
  {
    float ua;
    float ub;
  };
  std::stack<SegmentData> segmentList;

    // bezier coefficent matrix
    glm::mat4 bezierCoeffs = 
  {
    {-1,3,-3,1},
    {3,-6,3,0},
    {-3,3,0,0},
    {1,0,0,0}
  };
};
