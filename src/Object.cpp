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
    if (isWireFrame)
      glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    else
      glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    shape->DrawVAO();
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); // enforce draw fill after all
  }
  CHECKERROR;
}

void Object::SetPosition(glm::vec3 pos)
{
  position = pos;
  dirtyFlag = true;
}

void Object::SetRotation(Quaternion q)
{
  orientation = q;
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

Quaternion Object::GetOrientation()
{
  return orientation;
}

bool& Object::GetDirtyFlag()
{
  return dirtyFlag;
}

void Object::BuildModelMatrix()
{
  if (dirtyFlag)
  {
    modelTr = Translate(position.x, position.y, position.z) * orientation.toMat4() * Scale(scale.x, scale.y, scale.z);
    dirtyFlag = false;
  }
}

void Object::ApplyOrientationMatrix(glm::mat4& mat)
{
  modelTr *= mat;
}
