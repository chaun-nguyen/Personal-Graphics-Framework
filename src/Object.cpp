#include "Object.h"
#include "Shape.h"
#include "Engine.h"
#include "Shader.h"
#include "Transform.h"
#include "Model.h"

Object::Object(Shape* _shape,
  const glm::vec3 _diffuseColor, const glm::vec3 _specularColor, const float _shininess)
  : diffuseColor(_diffuseColor), specularColor(_specularColor), shininess(_shininess),
  shape(_shape)
{
  shape->parent = this;
}

Object::Object(std::string const& path, bool gamma)
{
  model = new Model(path, gamma);
  model->parent = this;
}

void Object::Draw()
{ 
  // Draw this object
  CHECKERROR;
  if (shape)
  {
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    shape->DrawVAO();
  }
  CHECKERROR;
}

void Object::SetPosition(glm::vec3 pos)
{
  position = pos;
  dirtyFlag = true;
}

void Object::SetRotation(float theta)
{
  angle = theta;
  dirtyFlag = true;
}

void Object::SetScale(glm::vec3 s)
{
  scale = s;
  dirtyFlag = true;
}

glm::vec3& Object::GetPosition()
{
  return position;
}

glm::vec3& Object::GetScale()
{
  return scale;
}

float& Object::GetAngle()
{
  return angle;
}

bool& Object::GetDirtyFlag()
{
  return dirtyFlag;
}

void Object::BuildModelMatrix()
{
  if (dirtyFlag)
  {
    modelTr = Translate(position.x, position.y, position.z) * Rotate(1, angle) * Scale(scale.x, scale.y, scale.z);
    dirtyFlag = false;
  }
}

void Object::ApplyOrientationMatrix(glm::mat4& mat)
{
  modelTr *= mat;
}
