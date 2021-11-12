#include "Model.h"
#include "Engine.h"
#include <iostream>

unsigned int TextureFromFile(const char* path, const std::string& directory, bool gamma = false);

glm::mat4 AssimpHelper::ConvertRowMajorToColumnMajor(aiMatrix4x4& assimpMatrix)
{
  glm::mat4 result;
  for (int i = 0; i < 4; ++i)
  {
    result[i][0] = assimpMatrix[0][i];
    result[i][1] = assimpMatrix[1][i];
    result[i][2] = assimpMatrix[2][i];
    result[i][3] = assimpMatrix[3][i];
  }

  return result;
}

Model::Model(std::string const& path, bool gamma) : gammaCorrection(gamma)
{
  loadModel(path);
}

void Model::Draw(ShaderProgram* shader)
{
  for (unsigned int i = 0; i < meshes.size(); i++)
    meshes[i].Draw(shader);
}

void Model::DrawVertexNormals()
{
  for (unsigned int i = 0; i < meshes.size(); i++)
    meshes[i].DrawVertexNormals();
}

void Model::DrawFaceNormals()
{
  for (unsigned int i = 0; i < meshes.size(); i++)
    meshes[i].DrawFaceNormals();
}

// create bounding volume
//void Model::CreateBoundingBox()
//{
//  for (auto& mesh : meshes)
//  {
//    BoundingVolume* bv = mesh.bbox(parent);
//    bv->bv_object->SetPosition(bv->center_ * bv->parent->GetScale() + bv->parent->GetPosition()); // need to add relative transform and multiply relative scale
//    bv->bv_object->SetScale(bv->parent->GetScale() * bv->size_); // world space scale * object space scale
//    bv->bv_object->BuildModelMatrix();
//    Engine::managers_.GetManager<ObjectManager*>()->AddBoundingVolume(bv);
//  }
//}

void Model::loadModel(std::string const& path)
{
  // read file via ASSIMP
  Assimp::Importer importer;
  const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_FlipUVs | aiProcess_CalcTangentSpace);// | aiProcess_JoinIdenticalVertices);
  // check for errors
  if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) // if is Not Zero
  {
    std::cout << "ERROR::ASSIMP:: " << importer.GetErrorString() << std::endl;
    throw std::runtime_error("Make Sure you have a right filepath convention such as:\n \"./model/Power_Plant_Files/Section1.txt\"");
  }
  // retrieve the directory path of the filepath
  directory = path.substr(0, path.find_last_of('/'));

  // process ASSIMP's root node recursively
  processNode(scene->mRootNode, scene);
}

void Model::processNode(aiNode* node, const aiScene* scene)
{
  // process each mesh located at the current node
  for (unsigned int i = 0; i < node->mNumMeshes; i++)
  {
    // the node object only contains indices to index the actual objects in the scene. 
    // the scene contains all the data, node is just to keep stuff organized (like relations between nodes).
    aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
    meshes.push_back(processMesh(mesh, scene));
  }
  // after we've processed all of the meshes (if any) we then recursively process each of the children nodes
  for (unsigned int i = 0; i < node->mNumChildren; i++)
  {
    processNode(node->mChildren[i], scene);
  }
}

Mesh Model::processMesh(aiMesh* mesh, const aiScene* scene)
{
  // data to fill
  //std::vector<Vertex> vertices;
  std::vector<glm::vec3> position;
  std::vector<glm::vec3> normal;
  std::vector<glm::vec2> texcoords;
  std::vector<glm::vec3> tangent;
  std::vector<glm::vec3> bitangent;

  // animation data to fill
  std::vector<glm::ivec4> BoneIDs;
  std::vector<glm::vec4> Weights;

  std::vector<unsigned int> indices;
  std::vector<Texture> textures;

  // walk through each of the mesh's vertices
  for (unsigned int i = 0; i < mesh->mNumVertices; i++)
  {
    // animation data
    glm::ivec4 boneID;
    glm::vec4 weight;
    SetVertexBoneDataToDefault(boneID, weight);
    BoneIDs.push_back(boneID);
    Weights.push_back(weight);

    //Vertex vertex;
    glm::vec3 vector; // we declare a placeholder vector since assimp uses its own vector class that doesn't directly convert to glm's vec3 class so we transfer the data to this placeholder glm::vec3 first.
    // positions
    vector.x = mesh->mVertices[i].x;
    vector.y = mesh->mVertices[i].y;
    vector.z = mesh->mVertices[i].z;
    position.push_back(vector);
    // normals
    if (mesh->HasNormals())
    {
      vector.x = mesh->mNormals[i].x;
      vector.y = mesh->mNormals[i].y;
      vector.z = mesh->mNormals[i].z;
      normal.push_back(vector);
    }
    // texture coordinates
    if (mesh->mTextureCoords[0]) // does the mesh contain texture coordinates?
    {
      glm::vec2 vec;
      // a vertex can contain up to 8 different texture coordinates. We thus make the assumption that we won't 
      // use models where a vertex can have multiple texture coordinates so we always take the first set (0).
      vec.x = mesh->mTextureCoords[0][i].x;
      vec.y = mesh->mTextureCoords[0][i].y;
      texcoords.push_back(vec);
      // tangent
      vector.x = mesh->mTangents[i].x;
      vector.y = mesh->mTangents[i].y;
      vector.z = mesh->mTangents[i].z;
      tangent.push_back(vector);
      // bitangent
      vector.x = mesh->mBitangents[i].x;
      vector.y = mesh->mBitangents[i].y;
      vector.z = mesh->mBitangents[i].z;
      bitangent.push_back(vector);
    }
    else
      texcoords.push_back(glm::vec2(0.0f, 0.0f));

    //vertices.push_back(vertex);
  }
  // now wak through each of the mesh's faces (a face is a mesh its triangle) and retrieve the corresponding vertex indices.
  for (unsigned int i = 0; i < mesh->mNumFaces; i++)
  {
    aiFace face = mesh->mFaces[i];
    // retrieve all indices of the face and store them in the indices vector
    for (unsigned int j = 0; j < face.mNumIndices; j++)
      indices.push_back(face.mIndices[j]);
  }
  // process materials
  aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
  // we assume a convention for sampler names in the shaders. Each diffuse texture should be named
  // as 'texture_diffuseN' where N is a sequential number ranging from 1 to MAX_SAMPLER_NUMBER. 
  // Same applies to other texture as the following list summarizes:
  // diffuse: texture_diffuseN
  // specular: texture_specularN
  // normal: texture_normalN

  // 1. diffuse maps
  std::vector<Texture> diffuseMaps = loadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse");
  textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
  // 2. specular maps
  std::vector<Texture> specularMaps = loadMaterialTextures(material, aiTextureType_SPECULAR, "texture_specular");
  textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
  // 3. normal maps
  std::vector<Texture> normalMaps = loadMaterialTextures(material, aiTextureType_NORMALS, "texture_normal");
  textures.insert(textures.end(), normalMaps.begin(), normalMaps.end());
  //// 4. height maps
  //std::vector<Texture> heightMaps = loadMaterialTextures(material, aiTextureType_AMBIENT, "texture_height");
  //textures.insert(textures.end(), heightMaps.begin(), heightMaps.end());

  // animation
  ExtractBoneWeightForVertices(BoneIDs, Weights, mesh, scene);

  // return a mesh object created from the extracted mesh data
  return Mesh(position, normal, texcoords, tangent, bitangent, BoneIDs, Weights, indices, textures);
}

std::vector<Texture> Model::loadMaterialTextures(aiMaterial* mat, aiTextureType type, std::string typeName)
{
  std::vector<Texture> textures;
  for (unsigned int i = 0; i < mat->GetTextureCount(type); i++)
  {
    aiString str;
    mat->GetTexture(type, i, &str);

    // correcting texture filepath (load fbx file)
    std::string path = std::string(str.C_Str());
    if (path.find("maria") != std::string::npos)
      path = path.substr(path.find("maria"));

    // check if texture was loaded before and if so, continue to next iteration: skip loading a new texture
    bool skip = false;
    for (unsigned int j = 0; j < textures_loaded.size(); j++)
    {
      if (std::strcmp(textures_loaded[j].path.data(), path.data()) == 0)
      {
        textures.push_back(textures_loaded[j]);
        skip = true; // a texture with the same filepath has already been loaded, continue to next one. (optimization)
        break;
      }
    }
    if (!skip)
    {   // if texture hasn't been loaded already, load it
      Texture texture;
      texture.id = TextureFromFile(path.c_str(), this->directory);
      texture.type = typeName;
      texture.path = path;
      textures.push_back(texture);
      textures_loaded.push_back(texture);  // store it as texture loaded for entire model, to ensure we won't unnecesery load duplicate textures.
    }
  }
  return textures;
}

// animation
std::map<std::string, BoneInfo>& Model::getBoneInfoMap()
{
  return m_BoneInfoMap;
}

int& Model::getBoneCount()
{
  return m_BoneCounter;
}

void Model::SetVertexBoneDataToDefault(glm::ivec4& m_BoneIDs, glm::vec4& m_Weights)
{
  for (int i = 0; i < MAX_BONE_INFLUENCE; ++i)
  {
    m_BoneIDs[i] = -1;
    m_Weights[i] = 0.0f;
  }
}

void Model::SetVertexBoneData(glm::ivec4& m_BoneIDs, glm::vec4& m_Weights, int boneID, float weight)
{
  for (int i = 0; i < MAX_BONE_INFLUENCE; ++i)
  {
    if (m_BoneIDs[i] < 0)
    {
      m_BoneIDs[i] = boneID;
      m_Weights[i] = weight;
      break;
    }
  }
}

void Model::ExtractBoneWeightForVertices(std::vector<glm::ivec4>& m_BoneIDs, std::vector<glm::vec4>& m_Weights, aiMesh* mesh, const aiScene* scene)
{
  for (int boneIndex = 0; boneIndex < mesh->mNumBones; ++boneIndex)
  {
    int boneID = -1;
    std::string boneName = mesh->mBones[boneIndex]->mName.C_Str();
    if (m_BoneInfoMap.find(boneName) == m_BoneInfoMap.end())
    {
      BoneInfo newBoneInfo;
      newBoneInfo.id = m_BoneCounter;
      newBoneInfo.offset = AssimpHelper::ConvertRowMajorToColumnMajor(mesh->mBones[boneIndex]->mOffsetMatrix);
      m_BoneInfoMap[boneName] = newBoneInfo;
      boneID = m_BoneCounter;
      m_BoneCounter++;
    }
    else
    {
      boneID = m_BoneInfoMap[boneName].id;
    }
    assert(boneID != -1);
    auto weights = mesh->mBones[boneIndex]->mWeights;
    int numWeights = mesh->mBones[boneIndex]->mNumWeights;

    for (int weightIndex = 0; weightIndex < numWeights; ++weightIndex)
    {
      int vertexId = weights[weightIndex].mVertexId;
      float weight = weights[weightIndex].mWeight;
      assert(vertexId <= m_BoneIDs.size());
      SetVertexBoneData(m_BoneIDs[vertexId], m_Weights[vertexId], boneID, weight);
    }
  }
}

unsigned int TextureFromFile(const char* path, const std::string& directory, bool gamma)
{
  stbi_set_flip_vertically_on_load(false);
  std::string filename = std::string(path);
  filename = directory + '/' + filename;

  unsigned int textureID;
  glGenTextures(1, &textureID);

  int width, height, nrComponents;
  unsigned char* data = stbi_load(filename.c_str(), &width, &height, &nrComponents, 0);
  if (data)
  {
    GLenum format;
    if (nrComponents == 1)
      format = GL_RED;
    else if (nrComponents == 3)
      format = GL_RGB;
    else if (nrComponents == 4)
      format = GL_RGBA;

    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    stbi_image_free(data);
  }
  else
  {
    std::cout << "Texture failed to load at path: " << path << std::endl;
    stbi_image_free(data);
  }

  return textureID;
}
