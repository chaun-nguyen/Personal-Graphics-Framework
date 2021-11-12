#include "FrameRateManager.h"
#include "LibHeader.h"
#include "Engine.h"
#include <iostream>
#include <sstream>
#include <iomanip>

void FrameRateManager::Setup()
{
}

void FrameRateManager::Update()
{
  // get elapsed time (in seconds) between previous and current frames
  static double prev_time = glfwGetTime();
  curr_time = glfwGetTime();
  delta_time = curr_time - prev_time;
  prev_time = curr_time;

  // fps calculations
  static double game_loop_count = 0.0; // number of game loop iterations
  static double start_time = glfwGetTime();
  // get elapsed time since very beginning (in seconds) ...
  elapsed_time = curr_time - start_time;
  ++game_loop_count;
  // update fps at least every 10 seconds ...
  fps_calc_interval = (fps_calc_interval < 0.0) ? 0.0 : fps_calc_interval;
  fps_calc_interval = (fps_calc_interval > 10.0) ? 10.0 : fps_calc_interval;
  if (elapsed_time > fps_calc_interval)
  {
    fps = game_loop_count / elapsed_time;
    start_time = curr_time;
    game_loop_count = 0.0;
  }

  if (click)
    UpdateWindowTitle();

  Timer();
}

void FrameRateManager::UpdateWindowTitle()
{
  std::stringstream ss;
  ss << "Graphics Framework - fps: " << static_cast<int>(fps);
  Engine::managers_.GetManager<WindowManager*>()->SetTitle(ss.str().c_str());
  click = !click;
}

void FrameRateManager::Timer()
{
  static int count = 0;
  if (count < 5)
  {
    ++count;
  }
  else
  {
    click = !click;
    count = 0;
  }
}
