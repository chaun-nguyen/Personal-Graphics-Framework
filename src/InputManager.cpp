#include "InputManager.h"
#include "Engine.h"

void InputManager::Setup()
{
  auto* wm = Engine::managers_.GetManager<WindowManager*>();

  MouseButtonCallBackHelper bind_mouse_button(this);
  MouseMotionCallBackHelper bind_mouse_motion(this);
  KeyboardCallBackHelper bind_keyboard(this);

  glfwSetMouseButtonCallback(wm->GetHandle(), MouseButtonCallBackHelper::CCallback);
  glfwSetCursorPosCallback(wm->GetHandle(), MouseMotionCallBackHelper::CCallback);
  glfwSetKeyCallback(wm->GetHandle(), KeyboardCallBackHelper::CCallback);
}

void InputManager::Update()
{
  glfwGetCursorPos(Engine::managers_.GetManager<WindowManager*>()->GetHandle(), &mouseX, &mouseY);
}

void InputManager::MouseMotion(GLFWwindow* window, double x, double y)
{
  if (glfw_used_flag)
  {
    auto* cm = Engine::managers_.GetManager<CameraManager*>();

    double dx = x - mouseX;
    double dy = y - mouseY;

    if (leftDown)
    {
      cm->yaw += static_cast<float>(dx / 3.0f);
      cm->pitch += static_cast<float>(-dy / 3.0f);

      cm->pitch = cm->pitch > 89.0f ? 89.0f : cm->pitch;
      cm->pitch = cm->pitch < -89.0f ? -89.0f : cm->pitch;
    }

    if (rightDown)
    {
      cm->tr[0] += static_cast<float>(dx);
      cm->tr[1] += static_cast<float>(-dy);
    }

    mouseX = x;
    mouseY = y;
  }
}

void InputManager::MouseButton(GLFWwindow* window, int button, int action, int mods)
{
  if (glfw_used_flag)
  {
    if (button == GLFW_MOUSE_BUTTON_LEFT)
      leftDown = (action == GLFW_PRESS);

    else if (button == GLFW_MOUSE_BUTTON_MIDDLE)
      middleDown = (action == GLFW_PRESS);

    else if (button == GLFW_MOUSE_BUTTON_RIGHT)
      rightDown = (action == GLFW_PRESS);
  }
}

void InputManager::Keyboard(GLFWwindow* window, int key, int scancode, int action, int mods)
{
  if (glfw_used_flag)
  {
    if (action == GLFW_REPEAT) return;

    auto* cm = Engine::managers_.GetManager<CameraManager*>();

    if (action == GLFW_PRESS)
    {
      switch (key)
      {
      case GLFW_KEY_TAB:
        cm->nav = !cm->nav;
        cm->key = GLFW_KEY_TAB;
        break;
      case GLFW_KEY_W:
      case GLFW_KEY_S:
      case GLFW_KEY_A:
      case GLFW_KEY_D:
        cm->key = key;
        break;
      case GLFW_KEY_ESCAPE:
        exit(EXIT_SUCCESS);
      }
    }
    else if (action == GLFW_RELEASE)
    {
      cm->key = GLFW_KEY_UNKNOWN;
    }
  }
  else
  {
    Engine::managers_.GetManager<CameraManager*>()->key = GLFW_KEY_UNKNOWN;
  }
}
