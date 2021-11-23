#pragma once
#include <GL/glew.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <type_traits>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_SWIZZLE
#include <glm/glm/glm.hpp>
#include <glm/glm/ext.hpp>
#include <glm/glm/gtc/type_ptr.hpp>

#define CHECKERROR {GLenum err = glGetError(); if (err != GL_NO_ERROR) { fprintf(stderr, "OpenGL error (at line %s:%d): %d\n", __FILE__, __LINE__, err); exit(-1);} }

constexpr float PI = 3.14159f;
constexpr float rad = PI / 180.0f;

template<typename E>
constexpr auto to_integral(E e)
{
  return static_cast<typename std::underlying_type<E>::type>(e);
}

template <class SysType, typename MemberCallback, MemberCallback MemberCallbackFn, typename ReturnType, typename ...Args>
class CCallbackHelper
{
public:

	CCallbackHelper(SysType* to_bind)
	{
		bound = to_bind;
	}

	static ReturnType CCallback(Args... args)
	{
		return (bound->*MemberCallbackFn)(args...);
	}

private:
	inline static SysType* bound{ nullptr };
};