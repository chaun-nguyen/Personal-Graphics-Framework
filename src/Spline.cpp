#include "Spline.h"

#include <numeric>

#include "LibHeader.h"
#include "Transform.h"
#include "Shader.h"

Spline::Spline(const std::vector<glm::vec3>& pts) : curveVAO(0), curveVBO(0), curveEBO(0), controlPoints(pts),
controlPointVAO(0), controlPointVBO(0), controlPointEBO(0)
{
}

void Spline::Draw(ShaderProgram* shader)
{
  CHECKERROR;
  int loc = glGetUniformLocation(shader->programID, "color");
  glUniform3fv(loc, 1, glm::value_ptr(glm::vec3(0.f, 1.f, 0.f)));

  glm::mat4 modelTr = Translate(0.f, 50.f, 0.f) * Scale(500.f, 500.f, 500.f);

  loc = glGetUniformLocation(shader->programID, "ModelTr");
  glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(modelTr));
  CHECKERROR;

  // draw interpolated spline curve
  CHECKERROR;
  glBindVertexArray(curveVAO);
  CHECKERROR;

  glDrawElements(GL_LINE_STRIP, static_cast<GLsizei>(curveIndices.size()), GL_UNSIGNED_INT, 0);
  CHECKERROR;

  glBindVertexArray(0);
  CHECKERROR;

  loc = glGetUniformLocation(shader->programID, "color");
  glUniform3fv(loc, 1, glm::value_ptr(glm::vec3(1.f, 0.f, 0.f)));
  // draw control points
  glBindVertexArray(controlPointVAO);
  CHECKERROR;
  glPointSize(10.f);
  glDrawElements(GL_POINTS, static_cast<GLsizei>(controlPointIndices.size()), GL_UNSIGNED_INT, 0);
  CHECKERROR;

  glBindVertexArray(0);
  CHECKERROR;
}

void Spline::Construct()
{
  CreateLinearSystem();
  PopulateBxByAndSolveLinearSystem();

  alpha = 1.f / (controlPoints.size() * 6.f);

  glm::vec3 left;
  glm::vec3 right = SplineInterpolate(0.f,controlPoints.size() - 1);
  for (float t = 0.f; t < controlPoints.size() - 1.f; t += alpha)
  {
    left = right;
    right = SplineInterpolate(t, controlPoints.size() - 1);
    interpolatePoints.emplace_back(left);
    interpolatePoints.emplace_back(right);
  }
  interpolatePoints.emplace_back(SplineInterpolate(controlPoints.size() - 1, controlPoints.size() - 1));

  // draw spline curve
  SetUpCurveVAO();

  //draw spline control points
  SetUpControlPointsVAO();

  // set up arc length function
  method = to_integral(ArcLengthFunctionType::AdaptiveApproach);
  SetUpArcLengthFunction();
}

void Spline::Update()
{
  UpdateSpline();
}

void Spline::UpdateVBO()
{
  // update spline curve
  CHECKERROR;
  glInvalidateBufferData(curveVBO);
  CHECKERROR;
  glNamedBufferSubData(curveVBO, 0, interpolatePoints.size() * sizeof(glm::vec3), interpolatePoints.data());
  CHECKERROR;
  // update control points
  glInvalidateBufferData(controlPointVBO);
  CHECKERROR;
  glNamedBufferSubData(controlPointVBO, 0, controlPoints.size() * sizeof(glm::vec3), controlPoints.data());
  CHECKERROR;
}

glm::vec3 Spline::DeCastlejau(float t)
{
  std::vector<std::vector<glm::vec3>> table;
  table.emplace_back(controlPoints);

  for (int i = 0; i < table.size(); ++i)
  {
    std::vector<glm::vec3> row;
    for (int j = 0; j < table[i].size() - 1; ++j)
    {
      glm::vec3 a_0_0 = table[i][j];
      glm::vec3 a_0_1 = table[i][j + 1];
      glm::vec3 a_1_0 = (1.f - t) * a_0_0 + t * a_0_1;
      row.emplace_back(a_1_0);
    }
    if (row.empty())
      break;
    table.emplace_back(row);
  }
  glm::vec3 result = table[table.size() - 1][0];

  return result;
}

glm::vec3 Spline::SplineInterpolate(float t, int degree)
{
  // evaluate spline function
  float final_x = 0.0;
  float final_y = 0.0;
  int k_counter = 0;

  for (int i = 0; i < degree + 1 + 2; ++i)
  {
    if (i < 4)
    {
      final_x += powf(t, (float)i) * resX[i];
      final_y += powf(t, (float)i) * resY[i];
    }
    else
    {
      final_x += EvalTruncatedFunction(t, k[k_counter], 3) * resX[i];
      final_y += EvalTruncatedFunction(t, k[k_counter], 3) * resY[i];
      ++k_counter;
    }
  }

  return glm::vec3(final_x, 0.f, final_y);
}

void Spline::Add(glm::vec3 point)
{
  controlPoints.push_back(point);
}

std::vector<glm::vec3>& Spline::GetControlPoints()
{
  return controlPoints;
}

std::vector<glm::vec3>& Spline::GetInterpolatePoints()
{
  return interpolatePoints;
}

void Spline::SetUpArcLengthFunction()
{
  switch (method)
  {
  case to_integral(ArcLengthFunctionType::ForwardDifferencing):
    CreateTableForwardDifferencing();
    break;
  case to_integral(ArcLengthFunctionType::AdaptiveApproach):
    CreateSortedSegmentList();
    break;
  }
}

void Spline::SetUpCurveVAO()
{
  CHECKERROR;
  glCreateVertexArrays(1, &curveVAO);
  glCreateBuffers(1, &curveVBO);
  glCreateBuffers(1, &curveEBO);

  // populate VBO data
  CHECKERROR;
  glNamedBufferStorage(curveVBO, interpolatePoints.size() * sizeof(glm::vec3), nullptr, GL_DYNAMIC_STORAGE_BIT);
  CHECKERROR;

  glNamedBufferSubData(curveVBO,
    0,
    interpolatePoints.size() * sizeof(glm::vec3),
    interpolatePoints.data());
  CHECKERROR;

  // bind VBO to VAO
  glEnableVertexArrayAttrib(curveVAO, 0);
  glVertexArrayVertexBuffer(curveVAO, 0, curveVBO, 0, sizeof(glm::vec3));
  glVertexArrayAttribFormat(curveVAO, 0, 3, GL_FLOAT, GL_FALSE, 0);
  glVertexArrayAttribBinding(curveVAO, 0, 0);

  CHECKERROR;

  // setting up indices (GL_LINE_STRIP mode)
  curveIndices.resize(interpolatePoints.size());
  std::iota(curveIndices.begin(), curveIndices.begin() + interpolatePoints.size(), 0);
  glNamedBufferStorage(curveEBO, curveIndices.size() * sizeof(unsigned int), curveIndices.data(), GL_DYNAMIC_STORAGE_BIT);
  glVertexArrayElementBuffer(curveVAO, curveEBO);
  CHECKERROR;
}

void Spline::SetUpControlPointsVAO()
{
  CHECKERROR;
  glCreateVertexArrays(1, &controlPointVAO);
  glCreateBuffers(1, &controlPointVBO);
  glCreateBuffers(1, &controlPointEBO);

  // populate VBO data
  CHECKERROR;
  glNamedBufferStorage(controlPointVBO, controlPoints.size() * sizeof(glm::vec3), nullptr, GL_DYNAMIC_STORAGE_BIT);
  CHECKERROR;

  glNamedBufferSubData(controlPointVBO,
    0,
    controlPoints.size() * sizeof(glm::vec3),
    controlPoints.data());
  CHECKERROR;

  // bind VBO to VAO
  glEnableVertexArrayAttrib(controlPointVAO, 0);
  glVertexArrayVertexBuffer(controlPointVAO, 0, controlPointVBO, 0, sizeof(glm::vec3));
  glVertexArrayAttribFormat(controlPointVAO, 0, 3, GL_FLOAT, GL_FALSE, 0);
  glVertexArrayAttribBinding(controlPointVAO, 0, 0);

  CHECKERROR;

  // setting up indices (GL_LINE_STRIP mode)
  controlPointIndices.resize(controlPoints.size());
  std::iota(controlPointIndices.begin(), controlPointIndices.begin() + controlPoints.size(), 0);
  glNamedBufferStorage(controlPointEBO, controlPointIndices.size() * sizeof(unsigned int), controlPointIndices.data(), GL_DYNAMIC_STORAGE_BIT);
  glVertexArrayElementBuffer(controlPointVAO, controlPointEBO);
  CHECKERROR;
}

void Spline::CreateLinearSystem()
{
  // populate k
  int degree = controlPoints.size() - 1;
  for (float i = 1.f; i < (float)degree; ++i)
    k.emplace_back(i);

  linearSystem = Eigen::MatrixXf::Zero(degree + 1 + 2, degree + 1 + 2);
  int k_counter_truncatedFunc = 0;
  int k_counter_truncatedFuncDerivative = 0;
  int secondDerivative_counter = 0;
  int secondDerivativeTruncatedFunc_counter = 0;
  bool enter_secondDerivative = false;
  bool enter_secondDerivativeTruncatedFunc = false;

  // f(t) = a0 + a1t + a2t^2 + a3t^3 + b1(t-1)^3 + b2(t-2)^3 + ... + b(k-1)(t-(k-1))^3
  for (int row = 0; row < degree + 1 + 2; ++row)
  {
    for (int col = 0; col < degree + 1 + 2; ++col)
    {
      // we have 4 fixed row and cols
      if (row <= degree)
      {
        // populate first row
        if (row == 0)
        {
          if (col == 0)
            linearSystem(row, col) = 1.0f;
          else
            linearSystem(row, col) = 0.0f;
          continue;
        }
        if (col < 4)
        {
          // not first row
          linearSystem(row, col) = powf((float)row, (float)col);
        }
        else
        {
          linearSystem(row, col) = EvalTruncatedFunction((float)row, k[k_counter_truncatedFunc], 3);
          ++k_counter_truncatedFunc;
        }
      }
      else // second derivative for 2 free coefficients
      {
        if (col < 4)
        {
          linearSystem(row, col) = TakeSecondDerivative((float)secondDerivative_counter, col);
          enter_secondDerivative = true;
        }
        else
        {
          linearSystem(row, col) = TakeSecondDerivativeTruncatedFunc((float)secondDerivativeTruncatedFunc_counter, k[k_counter_truncatedFuncDerivative], 3);
          enter_secondDerivativeTruncatedFunc = true;
          ++k_counter_truncatedFuncDerivative;
        }
      }
    }
    k_counter_truncatedFunc = 0;
    k_counter_truncatedFuncDerivative = 0;
    if (enter_secondDerivative)
      secondDerivative_counter += degree;
    if (enter_secondDerivativeTruncatedFunc)
      secondDerivativeTruncatedFunc_counter += degree;
  }
}

float Spline::EvalTruncatedFunction(float t, float c, int d)
{
  if (t < c)
    return 0.f;
  return std::pow(t - c, (float)d);
}

float Spline::TakeSecondDerivative(float t, int pow)
{
  if (t == 0.f || pow == 0)
    return 0.f;

  // function x^pow, and evaluate at t
  float coeff = pow * (pow - 1.f);
  return coeff * std::pow(t, pow - 2.f);
}

float Spline::TakeFirstDerivative(float t, int pow)
{
  if (t == 0.f || pow == 0)
    return 0.f;

  return pow * std::pow(t, pow - 1.f);
}

float Spline::TakeSecondDerivativeTruncatedFunc(float t, float c, int d)
{
  if (t < c)
    return 0.f;

  return TakeSecondDerivative(t - c, d);
}

void Spline::PopulateBxByAndSolveLinearSystem()
{
  int degree = controlPoints.size() - 1;
  // create Bx and By for later use for solving linear system
  Bx = Eigen::VectorXf::Zero(degree + 1 + 2);
  By = Eigen::VectorXf::Zero(degree + 1 + 2);

  for (int i = 0; i < degree + 1 + 2; ++i)
  {
    if (i <= degree)
    {
      Bx[i] = controlPoints[i].x;
      By[i] = controlPoints[i].z;
    }
    else
    {
      Bx[i] = 0.0;
      By[i] = 0.0;
    }
  }

  resX = linearSystem.colPivHouseholderQr().solve(Bx);
  resY = linearSystem.colPivHouseholderQr().solve(By);
}

void Spline::UpdateSpline()
{
  if (dirtyFlag)
  {
    // update Bx and By as control points moves
    PopulateBxByAndSolveLinearSystem();

    int index = 0;

    glm::vec3 left;
    glm::vec3 right = SplineInterpolate(0.f, controlPoints.size() - 1);
    for (float t = 0.f; t < controlPoints.size() - 1.f; t += alpha)
    {
      left = right;
      right = SplineInterpolate(t, controlPoints.size() - 1);
      interpolatePoints[index++] = left;
      interpolatePoints[index++] = right;
    }
    interpolatePoints[index] = SplineInterpolate(controlPoints.size() - 1, controlPoints.size() - 1);

    UpdateVBO();

    switch (method)
    {
    case to_integral(ArcLengthFunctionType::ForwardDifferencing):
      CreateTableForwardDifferencing();
      break;
    case to_integral(ArcLengthFunctionType::AdaptiveApproach):
      CreateSortedSegmentList();
      break;
    }
    
    dirtyFlag = false;
  }
}

// forward differencing approach
void Spline::CreateTableForwardDifferencing()
{
  table.clear();
  int degree = controlPoints.size() - 1;

  // first element is 0
  TableData data;
  data.Pu = SplineInterpolate(0, degree);
  table.push_back(data);

  deltaU = 0.001f;
  // populate table
  for (float u = deltaU; u <= 1.f; u += deltaU)
  {
    TableData d;
    // map [0,1] range back to [0,controlPoints.size() - 1]
    d.u = u;

    // calculate a point on the curve
    d.Pu = SplineInterpolate(d.u * degree, degree);

    // calculate the arc length
    d.s = glm::length(d.Pu - table.back().Pu) + table.back().s; // G(ui)

    table.push_back(d);
  }

  table.back().u = 1.f; // hard snap last one to 1
  
  // normalize value inside table
  for (auto it = table.begin(); it != table.end(); ++it)
  {
    it->s /= table.back().s;
  }
}

// binary search for inverse arc length function
float Spline::BinarySearchForLowestIndex(float s)
{
  int leftIndex = 0;
  int rightIndex = table.size() - 1;

  // s always in range [0,1]
  if (s > 1.0f)
    return rightIndex - 1;

  // binary search
  while (leftIndex <= rightIndex)
  {
    int midIndex = (leftIndex + rightIndex) / 2;

    if (table[midIndex].s == s)
      return midIndex;

    if (table[leftIndex].s <= s && table[leftIndex + 1].s >= s)
      return leftIndex;

    if (table[midIndex].s < s)
    {
      leftIndex = midIndex;
    }
    else if (table[midIndex].s > s)
    {
      rightIndex = midIndex;
    }
  }
  // return lowest index
  return rightIndex;
}

float Spline::getUFromTable(float index, float s)
{
  // interpolating u from table to get final u
  float denom = table[index + 1].s - table[index].s;
  float deltaS = s - table[index].s;
  float k = deltaS / denom;
  //float u = table[index].u + k * (table[index + 1].u - table[index].u);
  float u = (1 - k) * table[index].u + k * table[index + 1].u; // glm::mix

  return u;
}

glm::vec3 Spline::getInterpolatedPositionOnSpaceCurve(float s)
{
  // interpolating spline technique from mat300 goes from [0,degree]
  int degree = controlPoints.size() - 1;

  // get the lowest index
  float lowestIndex = BinarySearchForLowestIndex(s);

  // get final u from table
  float u = getUFromTable(lowestIndex, s);
  // get position on the curve
  // map [0,1] range back to [0,controlPoints.size() - 1]
  glm::vec3 Position = SplineInterpolate(u * degree, degree);

  return Position;
}

// adaptive approach
void Spline::CreateSortedSegmentList()
{
  table.clear();
  int degree = controlPoints.size() - 1;

  // first element is 0
  TableData data;
  data.Pu = SplineInterpolate(0, degree);
  table.push_back(data);

  segmentList.push({ 0.f,1.f }); // whole curve

  while (!segmentList.empty())
  {
    SegmentData segment = segmentList.top();
    segmentList.pop();

    float um = (segment.ua + segment.ub) / 2.f;

    // map [0,1] range back to [0,controlPoints.size() - 1]
    glm::vec3 Pua = SplineInterpolate(segment.ua * degree, degree);
    glm::vec3 Pub = SplineInterpolate(segment.ub * degree, degree);
    glm::vec3 Pum = SplineInterpolate(um * degree, degree);

    float A = glm::length(Pua - Pum);
    float B = glm::length(Pum - Pub);
    float C = glm::length(Pua - Pub);
    float d = A + B - C;

    if (d > epsilon || glm::length(segment.ua - segment.ub) > gamma)
    {
      segmentList.push({ um,segment.ub });
      segmentList.push({ segment.ua,um });
    }
    else
    {
      // sa = G(ua)
      float sa = table.back().s + glm::length(Pua - table.back().Pu);
      
      // sm = sa + A = G(um)
      float sm = sa + A;

      // sb = sm + B
      float sb = sm + B;
      
      table.push_back({ Pum,sm,um });
      table.push_back({ Pub,sb,segment.ub });
    }

  }

  // normalize value inside table
  for (auto it = table.begin(); it != table.end(); ++it)
  {
    it->s /= table.back().s;
  }
}