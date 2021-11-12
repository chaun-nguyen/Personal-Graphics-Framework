#include "Shader.h"
#include <fstream>
#include <iostream>

char* ShaderProgram::ReadFile(const char* name)
{
  std::ifstream f;
  f.open(name, std::ios_base::binary); // Open
  f.seekg(0, std::ios_base::end);      // Position at end
  int length = static_cast<int>(f.tellg());              //   to get the length

  char* content = new char[length + 1]; // Create buffer of needed length
  f.seekg(0, std::ios_base::beg);     // Position at beginning
  f.read(content, length);            //   to read complete file
  f.close();                           // Close

  content[length] = char(0);           // Finish with a NULL
  return content;
}

ShaderProgram::ShaderProgram()
{
  programID = glCreateProgram();
}

ShaderProgram::~ShaderProgram()
{
  glDeleteProgram(programID);
}

void ShaderProgram::AddShader(const char* fileName, const GLenum type)
{
  // Read the source from the named file
  char* src = ReadFile(fileName);
  const char* psrc[1] = { src };

  // Create a shader and attach, hand it the source, and compile it.
  int shader = glCreateShader(type);
  glAttachShader(programID, shader);
  glShaderSource(shader, 1, psrc, NULL);
  glCompileShader(shader);
  delete src;

  // Get the compilation status
  int status;
  glGetShaderiv(shader, GL_COMPILE_STATUS, &status);

  // If compilation status is not OK, get and print the log message.
  if (status != 1) {
    int length;
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &length);
    char* buffer = new char[length];
    glGetShaderInfoLog(shader, length, NULL, buffer);
    std::cout << "Compile log for " << fileName << ":\n" << buffer << std::endl;
    delete buffer;
  }
}

void ShaderProgram::LinkProgram()
{
  // Link program and check the status
  glLinkProgram(programID);
  int status;
  glGetProgramiv(programID, GL_LINK_STATUS, &status);

  // If link failed, get and print log
  if (status != 1) {
    int length;
    glGetProgramiv(programID, GL_INFO_LOG_LENGTH, &length);
    char* buffer = new char[length];
    glGetProgramInfoLog(programID, length, NULL, buffer);
    std::cout << "Link log" << ":\n" << buffer << std::endl;
    printf("Link log:\n%s\n", buffer);
    delete buffer;
  }
}

void ShaderProgram::Use()
{
  glUseProgram(programID);
}

void ShaderProgram::UnUse()
{
  glUseProgram(0);
}


