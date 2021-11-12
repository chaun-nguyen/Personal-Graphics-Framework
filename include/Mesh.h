#pragma once
#include "LibHeader.h"
#include "Shader.h"
#include "BoundingVolume.h"
#include <string>
#include <vector>
#include <array>

constexpr int MAX_BONE_INFLUENCE = 4;

struct Texture {
  unsigned int id;
  std::string type;
  std::string path;
};

class Mesh {
public:
  // mesh Data
  std::vector<glm::vec3> Position;
  std::vector<glm::vec3> Normal;
  std::vector<glm::vec2> TexCoords;
  std::vector<glm::vec3> Tangent;
  std::vector<glm::vec3> Bitangent;

  // animation data
  std::vector<glm::ivec4> m_BoneIDs;
  std::vector<glm::vec4> m_Weights;

  std::vector<unsigned int> indices;
  std::vector<Texture>      textures;
  unsigned int VAO;

  // create bounding volume
  BoundingVolume* bbox(Object* parent);

  // constructor
  Mesh(std::vector<glm::vec3> position,
    std::vector<glm::vec3> normal,
    std::vector<glm::vec2> texcoords,
    std::vector<glm::vec3> tangent,
    std::vector<glm::vec3> bitangent,
    std::vector<glm::ivec4> boneIDs,
    std::vector<glm::vec4> weights,
    std::vector<unsigned int> indices, std::vector<Texture> textures);

  // render the mesh
  void Draw(ShaderProgram* shader);

  // render normals
  void DrawVertexNormals();
  void DrawFaceNormals();

  // vertex normal (DEBUG)
  unsigned int vertexNormalVAO;
  unsigned int vertexNormalVBO, vertexNormalEBO;
  std::vector<glm::vec3> vertexNormalLine;
  std::vector<unsigned int> vertexNormalLineIndices;

  // face normal (DEBUG)
  unsigned int faceNormalVAO;
  unsigned int faceNormalVBO, faceNormalEBO;
  std::vector<glm::vec3> faceNormalLine;
  std::vector<unsigned int> faceNormalLineIndices;

private:
  // render data 
  unsigned int VBO, EBO;

  // initializes all the buffer objects/arrays
  void setupMesh();

  void setupVertexNormalDebug();
  void setupFaceNormalDebug();
};