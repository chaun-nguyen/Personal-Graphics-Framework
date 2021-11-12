#pragma once
#include <map>

#include "Mesh.h"
#include "Shader.h"
#include "BoundingVolume.h"
#include "stb_image.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <string>
#include <sstream>
#include <vector>

namespace AssimpHelper
{
  glm::mat4 ConvertRowMajorToColumnMajor(aiMatrix4x4& assimpMatrix);
}

struct BoneInfo
{
  int id;
  glm::mat4 offset;
};

class Model
{
public:
  // model data 
  std::vector<Texture> textures_loaded;	// stores all the textures loaded so far, optimization to make sure textures aren't loaded more than once.
  std::vector<Mesh>    meshes;
  std::string directory;
  bool gammaCorrection;
  Object* parent = nullptr;

  // constructor, expects a filepath to a 3D model.
  Model(std::string const& path, bool gamma = false);

  // draws the model, and thus all its meshes
  void Draw(ShaderProgram* shader);

  void DrawVertexNormals();
  void DrawFaceNormals();

  // animation
  std::map<std::string, BoneInfo>& getBoneInfoMap();
  int& getBoneCount();
private:
  // loads a model with supported ASSIMP extensions from file and stores the resulting meshes in the meshes vector.
  void loadModel(std::string const& path);

  // processes a node in a recursive fashion. Processes each individual mesh located at the node and repeats this process on its children nodes (if any).
  void processNode(aiNode* node, const aiScene* scene);

  Mesh processMesh(aiMesh* mesh, const aiScene* scene);

  // checks all material textures of a given type and loads the textures if they're not loaded yet.
  // the required info is returned as a Texture struct.
  std::vector<Texture> loadMaterialTextures(aiMaterial* mat, aiTextureType type, std::string typeName);

  // animation
  std::map<std::string, BoneInfo> m_BoneInfoMap;
  int m_BoneCounter = 0;

  void SetVertexBoneDataToDefault(glm::ivec4& m_BoneIDs, glm::vec4& m_Weights);
  void SetVertexBoneData(glm::ivec4& m_BoneIDs, glm::vec4& m_Weights, int boneID, float weight);
  void ExtractBoneWeightForVertices(
    std::vector<glm::ivec4>& m_BoneIDs,
    std::vector<glm::vec4>& m_Weights,
    aiMesh* mesh, const aiScene* scene);
};

