#include "Mesh.h"
#include <numeric>

BoundingVolume* Mesh::bbox(Object* parent)
{
  return new BV_AABB(Position, parent, { 1.0f,0.f,0.f });
}

Mesh::Mesh(std::vector<glm::vec3> position,
           std::vector<glm::vec3> normal,
           std::vector<glm::vec2> texcoords,
           std::vector<glm::vec3> tangent,
           std::vector<glm::vec3> bitangent,
           std::vector<glm::ivec4> boneIDs,
           std::vector<glm::vec4> weights,
           std::vector<unsigned> indices, std::vector<Texture> textures)
{
  Position = position;
  Normal = normal;
  TexCoords = texcoords;
  Tangent = tangent;
  Bitangent = bitangent;
  m_BoneIDs = boneIDs; // animation
  m_Weights = weights; // animation
  this->indices = indices;
  this->textures = textures;

  // now that we have all the required data, set the vertex buffers and its attribute pointers.
  setupMesh();
  setupVertexNormalDebug();
  setupFaceNormalDebug();
}

void Mesh::Draw(ShaderProgram* shader)
{
  // bind appropriate textures
  unsigned int diffuseNr = 1;
  unsigned int specularNr = 1;
  unsigned int normalNr = 1;
  unsigned int heightNr = 1;
  for (unsigned int i = 0; i < textures.size(); i++)
  {
    glActiveTexture(GL_TEXTURE10 + i); // active proper texture unit before binding
    // retrieve texture number (the N in diffuse_textureN)
    std::string number;
    std::string name = textures[i].type;
    if (name == "texture_diffuse")
      number = std::to_string(diffuseNr++);
    else if (name == "texture_specular")
      number = std::to_string(specularNr++); // transfer unsigned int to stream
    else if (name == "texture_normal")
      number = std::to_string(normalNr++); // transfer unsigned int to stream
    else if (name == "texture_height")
      number = std::to_string(heightNr++); // transfer unsigned int to stream

    // now set the sampler to the correct texture unit
    glUniform1i(glGetUniformLocation(shader->programID, (name + number).c_str()), i + 10);
    // and finally bind the texture
    glBindTexture(GL_TEXTURE_2D, textures[i].id);
  }

  // draw mesh
  glBindVertexArray(VAO);
  glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(indices.size()), GL_UNSIGNED_INT, 0);
  glBindVertexArray(0);

  // always good practice to set everything back to defaults once configured.
  glActiveTexture(GL_TEXTURE0);
}

void Mesh::DrawVertexNormals()
{
  // debug line draw
  CHECKERROR;
  glBindVertexArray(vertexNormalVAO);
  CHECKERROR;
  glDrawElements(GL_LINES, static_cast<GLsizei>(vertexNormalLineIndices.size()), GL_UNSIGNED_INT, 0);
  CHECKERROR;
  glBindVertexArray(0);
  CHECKERROR;
}

void Mesh::DrawFaceNormals()
{
  CHECKERROR;
  glBindVertexArray(faceNormalVAO);
  CHECKERROR;
  glDrawElements(GL_LINES, static_cast<GLsizei>(faceNormalLineIndices.size()), GL_UNSIGNED_INT, 0);
  CHECKERROR;
  glBindVertexArray(0);
  CHECKERROR;
}

void Mesh::setupMesh()
{
  glCreateVertexArrays(1, &VAO);

  glCreateBuffers(1, &VBO);
  glNamedBufferStorage(VBO,
    Position.size() * sizeof(glm::vec3) +
    Normal.size() * sizeof(glm::vec3) +
    TexCoords.size() * sizeof(glm::vec2) +
    Tangent.size() * sizeof(glm::vec3) +
    Bitangent.size() * sizeof(glm::vec3) +
    m_BoneIDs.size() * sizeof(glm::ivec4) +
    m_Weights.size() * sizeof(glm::vec4),
    nullptr, GL_DYNAMIC_STORAGE_BIT);

  glNamedBufferSubData(VBO, 
    0, 
    Position.size() * sizeof(glm::vec3), 
    Position.data());

  glNamedBufferSubData(VBO,
    Position.size() * sizeof(glm::vec3),
    Normal.size() * sizeof(glm::vec3),
    Normal.data());

  glNamedBufferSubData(VBO,
    Position.size() * sizeof(glm::vec3) + Normal.size() * sizeof(glm::vec3),
    TexCoords.size() * sizeof(glm::vec2),
    TexCoords.data());

  glNamedBufferSubData(VBO,
    Position.size() * sizeof(glm::vec3) + Normal.size() * sizeof(glm::vec3) + TexCoords.size() * sizeof(glm::vec2),
    Tangent.size() * sizeof(glm::vec3),
    Tangent.data());

  glNamedBufferSubData(VBO,
    Position.size() * sizeof(glm::vec3) + Normal.size() * sizeof(glm::vec3) + TexCoords.size() * sizeof(glm::vec2) + Tangent.size() * sizeof(glm::vec3),
    Bitangent.size() * sizeof(glm::vec3),
    Bitangent.data());

  // animation data
  glNamedBufferSubData(VBO,
    Position.size() * sizeof(glm::vec3) + Normal.size() * sizeof(glm::vec3) + TexCoords.size() * sizeof(glm::vec2) + Tangent.size() * sizeof(glm::vec3) + Bitangent.size() * sizeof(glm::vec3),
    m_BoneIDs.size() * sizeof(glm::ivec4),
    m_BoneIDs.data());

  glNamedBufferSubData(VBO,
    Position.size() * sizeof(glm::vec3) + Normal.size() * sizeof(glm::vec3) + TexCoords.size() * sizeof(glm::vec2) + Tangent.size() * sizeof(glm::vec3) + Bitangent.size() * sizeof(glm::vec3) + m_BoneIDs.size() * sizeof(glm::ivec4),
    m_Weights.size() * sizeof(glm::vec4),
    m_Weights.data());

  // position
  glEnableVertexArrayAttrib(VAO, 0);
  glVertexArrayVertexBuffer(VAO, 0, VBO, 0, sizeof(glm::vec3));
  glVertexArrayAttribFormat(VAO, 0, 3, GL_FLOAT, GL_FALSE, 0);
  glVertexArrayAttribBinding(VAO, 0, 0);

  // normal
  glEnableVertexArrayAttrib(VAO, 1);
  glVertexArrayVertexBuffer(VAO, 1, VBO, Position.size() * sizeof(glm::vec3), sizeof(glm::vec3));
  glVertexArrayAttribFormat(VAO, 1, 3, GL_FLOAT, GL_FALSE, 0);
  glVertexArrayAttribBinding(VAO, 1, 1);

  // texture
  glEnableVertexArrayAttrib(VAO, 2);
  glVertexArrayVertexBuffer(VAO, 2, VBO, Position.size() * sizeof(glm::vec3) + Normal.size() * sizeof(glm::vec3), sizeof(glm::vec2));
  glVertexArrayAttribFormat(VAO, 2, 2, GL_FLOAT, GL_FALSE, 0);
  glVertexArrayAttribBinding(VAO, 2, 2);

  // tangent
  glEnableVertexArrayAttrib(VAO, 3);
  glVertexArrayVertexBuffer(VAO, 3, VBO, Position.size() * sizeof(glm::vec3) + Normal.size() * sizeof(glm::vec3) + TexCoords.size() * sizeof(glm::vec2), sizeof(glm::vec3));
  glVertexArrayAttribFormat(VAO, 3, 3, GL_FLOAT, GL_FALSE, 0);
  glVertexArrayAttribBinding(VAO, 3, 3);

  // bi-tangent
  glEnableVertexArrayAttrib(VAO, 4);
  glVertexArrayVertexBuffer(VAO, 4, VBO, Position.size() * sizeof(glm::vec3) + Normal.size() * sizeof(glm::vec3) + TexCoords.size() * sizeof(glm::vec2) + Tangent.size() * sizeof(glm::vec3), sizeof(glm::vec3));
  glVertexArrayAttribFormat(VAO, 4, 3, GL_FLOAT, GL_FALSE, 0);
  glVertexArrayAttribBinding(VAO, 4, 4);

  // animation data
  // bone ids
  glEnableVertexArrayAttrib(VAO, 5);
  glVertexArrayVertexBuffer(VAO, 5, VBO, 
    Position.size() * sizeof(glm::vec3) + 
    Normal.size() * sizeof(glm::vec3) + 
    TexCoords.size() * sizeof(glm::vec2) + 
    Tangent.size() * sizeof(glm::vec3) +
    Bitangent.size() * sizeof(glm::vec3), sizeof(glm::ivec4));
  //glVertexArrayAttribFormat(VAO, 5, MAX_BONE_INFLUENCE, GL_INT, GL_FALSE, 0);
  glVertexArrayAttribIFormat(VAO, 5, MAX_BONE_INFLUENCE, GL_INT, 0);
  glVertexArrayAttribBinding(VAO, 5, 5);

  // bone weights
  glEnableVertexArrayAttrib(VAO, 6);
  glVertexArrayVertexBuffer(VAO, 6, VBO,
    Position.size() * sizeof(glm::vec3) +
    Normal.size() * sizeof(glm::vec3) +
    TexCoords.size() * sizeof(glm::vec2) +
    Tangent.size() * sizeof(glm::vec3) +
    Bitangent.size() * sizeof(glm::vec3) +
    m_BoneIDs.size() * sizeof(glm::ivec4), sizeof(glm::vec4));
  glVertexArrayAttribFormat(VAO, 6, MAX_BONE_INFLUENCE, GL_FLOAT, GL_FALSE, 0);
  glVertexArrayAttribBinding(VAO, 6, 6);

  glCreateBuffers(1, &EBO);
  glNamedBufferStorage(EBO, indices.size() * sizeof(unsigned int), indices.data(), GL_DYNAMIC_STORAGE_BIT);
  glVertexArrayElementBuffer(VAO, EBO);

  // create buffers/arrays
  //glGenVertexArrays(1, &VAO);
  //glGenBuffers(1, &VBO);
  //glGenBuffers(1, &EBO);
  //
  //glBindVertexArray(VAO);
  //// load data into vertex buffers
  //glBindBuffer(GL_ARRAY_BUFFER, VBO);
  //// A great thing about structs is that their memory layout is sequential for all its items.
  //// The effect is that we can simply pass a pointer to the struct and it translates perfectly to a glm::vec3/2 array which
  //// again translates to 3/2 floats which translates to a byte array.
  //glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);
  //
  //glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
  //glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);
  //
  //// set the vertex attribute pointers
  //// vertex Positions
  //glEnableVertexAttribArray(0);
  //glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
  //// vertex normals
  //glEnableVertexAttribArray(1);
  //glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Normal));
  //// vertex texture coords
  //glEnableVertexAttribArray(2);
  //glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, TexCoords));
  //// vertex tangent
  //glEnableVertexAttribArray(3);
  //glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Tangent));
  //// vertex bitangent
  //glEnableVertexAttribArray(4);
  //glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Bitangent));
  //
  //glBindVertexArray(0);
}

void Mesh::setupVertexNormalDebug()
{
  for (int i = 0; i < static_cast<int>(Position.size()); ++i)
  {
    glm::vec3 nVector = Position[i] + (Normal[i] * 5000.f);
    vertexNormalLine.push_back(Position[i]);
    vertexNormalLine.push_back(nVector);
  }
  vertexNormalLineIndices.resize(vertexNormalLine.size());
  std::iota(vertexNormalLineIndices.begin(), vertexNormalLineIndices.begin() + vertexNormalLine.size(), 0);

  glCreateVertexArrays(1, &vertexNormalVAO);
  glCreateBuffers(1, &vertexNormalVBO);
  glNamedBufferStorage(vertexNormalVBO, vertexNormalLine.size() * sizeof(glm::vec3), vertexNormalLine.data(), GL_DYNAMIC_STORAGE_BIT);

  glEnableVertexArrayAttrib(vertexNormalVAO, 0);
  glVertexArrayVertexBuffer(vertexNormalVAO, 0, vertexNormalVBO, 0, sizeof(glm::vec3));
  glVertexArrayAttribFormat(vertexNormalVAO, 0, 3, GL_FLOAT, GL_FALSE, 0);
  glVertexArrayAttribBinding(vertexNormalVAO, 0, 0);
  
  glCreateBuffers(1, &vertexNormalEBO);
  glNamedBufferStorage(vertexNormalEBO, vertexNormalLineIndices.size() * sizeof(unsigned int), vertexNormalLineIndices.data(), GL_DYNAMIC_STORAGE_BIT);
  glVertexArrayElementBuffer(vertexNormalVAO, vertexNormalEBO);
}

void Mesh::setupFaceNormalDebug()
{
  for (int i = 0; i < static_cast<int>(Position.size()); i+=6)
  {
    glm::vec3 faceNormalVector = Position[i] + (Normal[i] * 5000.f);
    faceNormalLine.push_back(Position[i]);
    faceNormalLine.push_back(faceNormalVector);
  }
  faceNormalLineIndices.resize(faceNormalLine.size());
  std::iota(faceNormalLineIndices.begin(), faceNormalLineIndices.begin() + faceNormalLine.size(), 0);

  glCreateVertexArrays(1, &faceNormalVAO);
  glCreateBuffers(1, &faceNormalVBO);
  glNamedBufferStorage(faceNormalVBO, faceNormalLine.size() * sizeof(glm::vec3), faceNormalLine.data(), GL_DYNAMIC_STORAGE_BIT);

  glEnableVertexArrayAttrib(faceNormalVAO, 0);
  glVertexArrayVertexBuffer(faceNormalVAO, 0, faceNormalVBO, 0, sizeof(glm::vec3));
  glVertexArrayAttribFormat(faceNormalVAO, 0, 3, GL_FLOAT, GL_FALSE, 0);
  glVertexArrayAttribBinding(faceNormalVAO, 0, 0);

  glCreateBuffers(1, &faceNormalEBO);
  glNamedBufferStorage(faceNormalEBO, faceNormalLineIndices.size() * sizeof(unsigned int), faceNormalLineIndices.data(), GL_DYNAMIC_STORAGE_BIT);
  glVertexArrayElementBuffer(faceNormalVAO, faceNormalEBO);
}
