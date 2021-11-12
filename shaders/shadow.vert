#version 460

layout (location = 0) in vec3 vertex;
layout (location = 1) in vec3 vertexNormal;
layout (location = 2) in vec2 vertexTexture;
layout (location = 3) in vec3 vertexTangent;
layout (location = 4) in vec3 vertexBiTangent;

// animation data
layout (location = 5) in ivec4 boneIds;
layout (location = 6) in vec4 weights;

uniform mat4 ModelTr;
uniform mat4 ViewMatrix;
uniform mat4 ProjectionMatrix;

// animation data
const int MAX_BONES = 100;
const int MAX_BONE_INFLUENCE = 4;
uniform mat4 finalBonesMatrices[MAX_BONES];

void main()
{
  vec4 totalPosition = vec4(0.0);
  for (int i = 0; i < MAX_BONE_INFLUENCE; ++i)
  {
    if (boneIds[i] == -1)
    {
      continue;
    }
    if (boneIds[i] >= MAX_BONES)
    {
      totalPosition = vec4(vertex,1.0);
      break;
    }
    vec4 localPosition = finalBonesMatrices[boneIds[i]] * vec4(vertex,1.0);
    totalPosition += localPosition * weights[i];
  }

  gl_Position = ProjectionMatrix*ViewMatrix*ModelTr*totalPosition;
}