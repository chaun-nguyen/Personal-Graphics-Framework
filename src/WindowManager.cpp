#include <cstdlib>
#include "WindowManager.h"
#include <iostream>
#include "ImGuiUIManager.h"
#include "Engine.h"

WindowManager::~WindowManager()
{
  glfwTerminate();
}

void WindowManager::SetSize(int width, int height)
{
  w_ = width;
  h_ = height;
}

void WindowManager::SetTitle(const char* title)
{
  title_ = const_cast<char*>(title);
  glfwSetWindowTitle(window_, title_);
}

int WindowManager::GetWidth()
{
  return w_;
}

int WindowManager::GetHeight()
{
  return h_;
}

void WindowManager::Setup()
{
  SetSize(1920, 1080);
  SetTitle("Graphics Framework");
  resolution_ = { 1000.f,588.235f };

  // Initialize glfw open a window
  if (!glfwInit())  exit(EXIT_FAILURE);

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // required on Mac
  glfwWindowHint(GLFW_DOUBLEBUFFER, GLFW_TRUE);
  glfwWindowHint(GLFW_RESIZABLE, 1);
  glfwWindowHint(GLFW_RED_BITS, 8);
  glfwWindowHint(GLFW_GREEN_BITS, 8);
  glfwWindowHint(GLFW_BLUE_BITS, 8);
  glfwWindowHint(GLFW_ALPHA_BITS, 8);
  glfwWindowHint(GLFW_SRGB_CAPABLE, GLFW_TRUE);
  glfwWindowHint(GLFW_DEPTH_BITS, 32);
  glfwWindowHint(GLFW_SAMPLES, 4); // 4x msaa
  window_ = glfwCreateWindow(w_, h_, title_, nullptr, nullptr);

  if (!window_)
  {
    glfwTerminate();
    exit(EXIT_FAILURE);
  }

  // make context before init glew
  glfwMakeContextCurrent(window_);

  SetUpGLEW();

  glfwSwapInterval(1); // enable vsync

  HardwareInfo();
}

void WindowManager::Update()
{
  glfwPollEvents();
}

int WindowManager::ShouldClose()
{
  return glfwWindowShouldClose(window_);
}

GLFWwindow* WindowManager::GetHandle()
{
  return window_;
}

void WindowManager::SetUpGLEW()
{
  const auto err = glewInit();
  if (err != GLEW_OK)
  {
    std::runtime_error("Unable to initialize GLEW");
  }

  if (GLEW_VERSION_4_6)
  {
    std::cout << "Using glew version: " << glewGetString(GLEW_VERSION) << std::endl;
    std::cout << "Driver support OpenGL 4.6" << std::endl;
  }
  else
  {
    std::cout << "Driver doesn't support OpenGL 4.6" << std::endl;
  }
}

void WindowManager::HardwareInfo()
{
  GLubyte const* vendor = glGetString(GL_VENDOR);
  std::cout << "GPU Vendor: " << vendor << std::endl;
  std::cout << "GL Renderer: " << glGetString(GL_RENDERER) << std::endl;
  std::cout << "GL Version: " << glGetString(GL_VERSION) << std::endl;
  std::cout << "GLSL Version: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;

  // print major version
  GLint major_num;
  glGetIntegerv(GL_MAJOR_VERSION, &major_num);
  std::cout << "GL Major Version: " << major_num << std::endl;

  // print minor version
  GLint minor_num;
  glGetIntegerv(GL_MINOR_VERSION, &minor_num);
  std::cout << "GL Minor Version: " << minor_num << std::endl;

  // print current OpenGL Context
  GLboolean is_double_buffer;
  glGetBooleanv(GL_DOUBLEBUFFER, &is_double_buffer);
  if (is_double_buffer)
    std::cout << "Current OpenGL Context is double buffered" << std::endl;
  else
    std::cout << "Current OpenGL Context is not double buffered" << std::endl;

  // print maximum number of vertex array vertices
  GLint max_vertex_count;
  glGetIntegerv(GL_MAX_ELEMENTS_VERTICES, &max_vertex_count);
  std::cout << "Maximum Vertex Count: " << max_vertex_count << std::endl;

  // print maximum number of vertex array indices
  GLint max_index_count;
  glGetIntegerv(GL_MAX_ELEMENTS_INDICES, &max_index_count);
  std::cout << "Maximum Indices Count: " << max_index_count << std::endl;

  // print max texture size
  GLint tex_size;
  glGetIntegerv(GL_MAX_TEXTURE_SIZE, &tex_size);
  std::cout << "GL Maximum texture size: " << tex_size << std::endl;

  // print maximum viewport dimensions
  GLint viewport_dims[2];
  glGetIntegerv(GL_MAX_VIEWPORT_DIMS, viewport_dims);
  std::cout << "Maximum Viewport Dimensions: " << viewport_dims[0] << " x " << viewport_dims[1] << std::endl;
}
