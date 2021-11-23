#version 460

layout (location = 0) in vec3 bonePosition;

uniform mat4 WorldView;
//uniform mat4 WorldInverse;
uniform mat4 WorldProj;
uniform mat4 ModelTr;

void main()
{
  gl_Position = WorldProj*WorldView*ModelTr*vec4(bonePosition,1.0);
}