#pragma once
#include "LibHeader.h"

class ShaderProgram
{
public:
  ShaderProgram();
  ~ShaderProgram();

  void AddShader(const char* fileName, const GLenum type);
  void LinkProgram();
  void Use();
  void UnUse();

  int programID;

  char* ReadFile(const char* name);
};