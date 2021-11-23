#include "CameraManager.h"
#include "Transform.h"
#include "Engine.h"

void CameraManager::Setup()
{
  nav = false;
  eye = glm::vec3(0.0f, 0.0f, 0.0f);
  tr = glm::vec3(0.0f, 500.0f, 15500.0f);
  yaw = -90.0f;
  pitch = -30.0f;
  speed = 3000.0f;
  ry = 0.4f;
  front = 1.f;
  back = 80000.0f;
  
  auto* wm = Engine::managers_.GetManager<WindowManager*>();
  //glfwGetFramebufferSize(wm->GetHandle(), &width, &height);

  aspectRatio = wm->resolution_.x / wm->resolution_.y;
  relativeUpVector = { 0.0f,1.0f,0.0f };
  fov = 45.f;

  lookAtVector = { cosf(glm::radians(yaw)) * cosf(glm::radians(pitch)),
                   sinf(glm::radians(pitch)),
                   sinf(glm::radians(yaw)) * cosf(glm::radians(pitch)) };
  backVector = glm::normalize(glm::vec3(-1.f) * lookAtVector);
  rightVector = glm::normalize(glm::cross(relativeUpVector, backVector));
  upVector = glm::cross(backVector, rightVector);

  WorldView = Translate(-tr[0], -tr[1], -tr[2]) * LookAt(eye, eye + lookAtVector, upVector);
  WorldInverse = glm::inverse(WorldView);
  //WorldProj = Perspective(ry * aspectRatio, ry, front, back);
  WorldProj = glm::perspective(fov, aspectRatio, front, back);
}

void CameraManager::Update()
{
  BuildMatrix();

  if (key == GLFW_KEY_A || key == GLFW_KEY_S || key == GLFW_KEY_D || key == GLFW_KEY_W)
  {
    if (nav)
      Movement();
  }
}

void CameraManager::BuildMatrix()
{
  static bool saved_nav = false;

  if (saved_nav != nav)
  {
    if (nav)
    {
      yaw = -90.0f;
      pitch = 0.0f;
    }
    else
    {
      yaw = -90.0f;
      pitch = -30.0f;
    }
    eye = glm::vec3(0.0f, 0.0f, 0.0f);
    tr = glm::vec3(0.0f, 500.0f, 15500.0f);
    saved_nav = nav;
  }

  //WorldProj = PerspectiveFov(45.f,static_cast<float>(width / height), front, back);
  //WorldProj = Perspective(ry * aspectRatio, ry, front, back);
  //if (nav)
  //  WorldView = Rotate(0, tilt - 90) * Rotate(2, spin) * Translate(-eye[0], -eye[1], -eye[2]);
  //else
  //  WorldView = Translate(tr[0], -tr[1], tr[2]) * Rotate(0, tilt - 90) * Rotate(1, spin);

  lookAtVector = { cosf(glm::radians(yaw)) * cosf(glm::radians(pitch)),
                 sinf(glm::radians(pitch)),
                 sinf(glm::radians(yaw)) * cosf(glm::radians(pitch)) };
  backVector = glm::normalize(glm::vec3(-1.f) * lookAtVector);
  rightVector = glm::normalize(glm::cross(relativeUpVector, backVector));
  upVector = glm::cross(backVector, rightVector);

  if (nav)
  {
    WorldView = LookAt(eye, eye + lookAtVector, upVector) * Translate(-tr[0], -tr[1], -tr[2]);
  }
  else
  {
    WorldView = Translate(-tr[0], -tr[1], -tr[2]) * LookAt(eye, eye + lookAtVector, upVector);
  }
  
  WorldInverse = glm::inverse(WorldView);
}

void CameraManager::Movement()
{
  auto* fr = Engine::managers_.GetManager<FrameRateManager*>();
  float dist = speed * static_cast<float>(fr->delta_time);
  if (key == GLFW_KEY_W)
  {
    eye += dist * lookAtVector;
  }
  else if (key == GLFW_KEY_S)
  {
    eye -= dist * lookAtVector;
  }
  else if (key == GLFW_KEY_D)
  {
    eye += dist * rightVector;
  }
  else if (key == GLFW_KEY_A)
  {
    eye -= dist * rightVector;
  }
    
}
