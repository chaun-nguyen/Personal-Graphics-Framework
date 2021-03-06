#pragma once
#include "LibHeader.h"
#include <string>
#include "Quaternion.h"

class BoundingVolume;
class Model;
class TextureLoader;
class Shape;
class Physics;

class Object
{
public:
  // procedural shape (box, sphere, plane)
  Object(Shape* _shape,
    const glm::vec3 _d = glm::vec3(), const glm::vec3 _s = glm::vec3(), const float _n = 1);

  // loaded model
  Object(std::string const& path, bool gamma); 
  ~Object() = default;

  void Draw();

  void SetPosition(glm::vec3 pos);
  void SetRotation(Quaternion q);
  void SetScale(glm::vec3 s);

  glm::vec3& GetPosition();
  glm::vec3& GetScale();
  Quaternion GetOrientation();
  bool& GetDirtyFlag();

  void BuildModelMatrix();
  void ApplyOrientationMatrix(glm::mat4& mat);

  Shape* shape = nullptr;

  glm::vec3 diffuseColor;          // Diffuse color of object
  glm::vec3 specularColor;         // Specular color of object
  float shininess;            // Surface roughness value

  TextureLoader* diffuseTex = nullptr;
  TextureLoader* normalTex = nullptr;

  glm::mat4 modelTr;

  Model* model = nullptr;
  BoundingVolume* bv = nullptr;

  int isTextureSupported = 1;
  float tiling = 10.f;

  bool isWireFrame = false;

  Physics* physics = nullptr;
private:
  glm::vec3 position;
  glm::vec3 scale;
  Quaternion orientation; // identity quaternion
  bool dirtyFlag = false;
};
