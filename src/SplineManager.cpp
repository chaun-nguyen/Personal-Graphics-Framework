#include "SplineManager.h"
#include "Engine.h"
#include "LibHeader.h"
#include "Shader.h"


void SplineManager::Setup()
{
  Spline curve;
  curve.Add({ 0.f,0.f,0.f });
  curve.Add({ -2.f,0.f,5.8f });
  curve.Add({ -0.7f,0.f,11.3f });
  curve.Add({ 2.5f,0.f,13.3f });
  curve.Add({ 5.1f,0.f,12.4f });
  curve.Add({ 7.2f,0.f,9.4f });
  curve.Add({ 8.f,0.f,2.5f });
  curve.Add({ 6.4f,0.f,-4.7f });
  curve.Add({ 3.3f,0.f,-10.f });
  curve.Add({ -1.1f,0.f,-9.7f });
  curve.Add({ -5.f,0.f,-3.6f });
  curve.Add({ -6.2f,0.f,4.6f });

  // construct spline curve
  curve.Construct();

  spaceCurves.push_back(curve);

  t1 = 0.44f;
  t2 = 0.84f;
  Vc = 2.f / (1.f - t1 + t2);
}

void SplineManager::Update()
{
  auto& models = Engine::managers_.GetManager<ObjectManager*>()->GetModels();
  auto* am = Engine::managers_.GetManager<AnimationManager*>();

  // reset s
  if (t > 1.f)
    t = 0.f;

  if (!models.empty() && am->PlayAnimation)
  {
    Object* player = models[0]; // player always the first index

    Spline& currCurve = spaceCurves[index];

    // sync the velocity from parabolic approach with the animation
    am->animator->speed = GetV(t);

    MoveAlongSpaceCurve(player, currCurve, t);

    // step size
    t += Engine::managers_.GetManager<FrameRateManager*>()->delta_time / 10.f;
  }

  for (auto& curve : spaceCurves)
  {
    curve.Update();
  }
}

void SplineManager::Draw(ShaderProgram* shader)
{
  for (int i = 0; i < spaceCurves.size(); ++i)
  {
    spaceCurves[i].Draw(shader, colors[i]);
  }
}

Spline& SplineManager::GetCurve(int index)
{
  return spaceCurves[index];
}

void SplineManager::AddCurve(Spline& curve)
{
  spaceCurves.emplace_back(curve);
}

int SplineManager::GetSize()
{
  return spaceCurves.size();
}

// speed control distance-time function (parabolic ease in/out approach)
void SplineManager::SetT1(float t)
{
  t1 = t;
  Vc = 2.f / (1.f - t1 + t2);
}

void SplineManager::SetT2(float t)
{
  t2 = t;
  Vc = 2.f / (1.f - t1 + t2);
}

// distance-time function
float SplineManager::EvalS1(float t)
{
  // avoid dividing by 0
  if (t1 == 0.f)
    return 0.f;

  return t * t * Vc / (2.f * t1);
}

float SplineManager::EvalS2(float t)
{
  return Vc * (t - t1 / 2.f);
}

float SplineManager::EvalS3(float t)
{
  return Vc * (t - t2) * (2.f - t - t2) / (2.f * (1.f - t2)) + Vc * (t2 - t1 / 2.f);
}

float SplineManager::GetS(float t)
{
  if (t <= t1)
    return EvalS1(t);

  if (t >= t2)
    return EvalS3(t);

  return EvalS2(t);
}

float SplineManager::EvalV1(float t)
{
  // avoid dividing by 0
  if (t1 == 0.f)
    return 0.f;

  return Vc * t / t1;
}

float SplineManager::EvalV2(float t)
{
  // constant velocity
  return Vc;
}

float SplineManager::EvalV3(float t)
{
  return Vc * (1 - t) / (1 - t2);
}

float SplineManager::GetV(float t)
{
  if (t <= t1)
    return EvalV1(t);

  if (t >= t2)
    return EvalV3(t);

  return EvalV2(t);
}

void SplineManager::MoveAlongSpaceCurve(Object* player, Spline& currCurve, float t_)
{
  s = GetS(t_);

  // calculate the next step on space curve
  glm::vec3 Position = currCurve.getInterpolatedPositionOnSpaceCurve(s);

  // orientation control (forward mode)
  glm::vec3 COI =
    currCurve.getInterpolatedPositionOnSpaceCurve(s + 0.001f) +
    currCurve.getInterpolatedPositionOnSpaceCurve(s + 0.001f * 2.f) +
    currCurve.getInterpolatedPositionOnSpaceCurve(s + 0.001f * 3.f);
  COI /= 3.f;

  glm::vec3 W = COI - Position;
  W = glm::normalize(W);
  glm::vec3 U = glm::normalize(glm::cross(glm::vec3(0.f, 1.f, 0.f), W));
  glm::vec3 V = glm::normalize(glm::cross(W, U));

  glm::mat4 M =
  {
    glm::vec4(U,0.f),
    glm::vec4(V,0.f),
    glm::vec4(W,0.f),
    glm::vec4(Position,1.f)
  };

  // scale up to the same size as the curve
  Position *= 500.f;
  player->SetPosition(Position);
  player->BuildModelMatrix();

  // set up player orientation
  player->ApplyOrientationMatrix(M);

  auto* am = Engine::managers_.GetManager<AnimationManager*>();
  // set up bone orientation
  am->animation->SetBoneWorldPosition(Position);
  am->animation->SetBoneOrientation(M);
}


