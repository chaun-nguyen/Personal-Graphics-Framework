#pragma once
#include "ManagerBase.h"
#include "Spline.h"

class SplineManager : public ManagerBase<SplineManager>
{
public:
  SplineManager() = default;
  ~SplineManager() override = default;

  void Setup() override;
  void Update() override;

  void Draw(ShaderProgram* shader);

  // activated index
  int index = 0; // only one space curve for now

  Spline& GetCurve(int index);

  // speed control distance-time function (parabolic ease in/out approach)
  float t1;
  float t2;
  float Vc; // easy for normalizing (s3(1) == 1)
private:
  float t = 0.f;
  float s = 0.f;
  std::vector<Spline> spaceCurves;

  // speed control distance-time function (parabolic ease in/out approach)
  void SetT1(float t);
  void SetT2(float t);

  // distance-time function
  float EvalS1(float t);
  float EvalS2(float t);
  float EvalS3(float t);
  float GetS(float t);

  // velocity-time function
  float EvalV1(float t);
  float EvalV2(float t);
  float EvalV3(float t);
  float GetV(float t);
};