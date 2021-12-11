#include "ImGuiUIManager.h"
#include "Engine.h"
#include "Texture.h"
#include <iostream>

// Our state (make them static = more or less global) as a convenience to keep the example terse.
static bool show_demo_window = false;
static bool albedo_window = true;
static bool normal_window = true;
static bool position_window = true;
static bool specular_window = true;
static bool depth_window = true;
static bool shadowMap_window = false;
static bool octree_window = true;
static bool bsp_window = true;
static bool model_window = false;
static bool gjk_window = true;
static bool animation_window = false;
static bool path_window = false;
static bool physic_window = true;

// utility structure for realtime plot
struct ScrollingBuffer {
  int MaxSize;
  int Offset;
  ImVector<ImVec2> Data;
  ScrollingBuffer(int max_size = 2000) {
    MaxSize = max_size;
    Offset = 0;
    Data.reserve(MaxSize);
  }
  void AddPoint(float x, float y) {
    if (Data.size() < MaxSize)
      Data.push_back(ImVec2(x, y));
    else {
      Data[Offset] = ImVec2(x, y);
      Offset = (Offset + 1) % MaxSize;
    }
  }
  void Erase() {
    if (Data.size() > 0) {
      Data.shrink(0);
      Offset = 0;
    }
  }
};

// utility structure for realtime plot
struct RollingBuffer {
  float Span;
  ImVector<ImVec2> Data;
  RollingBuffer() {
    Span = 10.0f;
    Data.reserve(2000);
  }
  void AddPoint(float x, float y) {
    float xmod = fmodf(x, Span);
    if (!Data.empty() && xmod < Data.back().x)
      Data.shrink(0);
    Data.push_back(ImVec2(xmod, y));
  }
};

ImGuiUIManager::~ImGuiUIManager()
{
  // for using implot
  ImPlot::DestroyContext();

  // shutdown imgui
  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext();
}

void ImGuiUIManager::Setup()
{
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();

  // for using implot
  ImPlot::CreateContext();

  ImGuiIO& io = ImGui::GetIO();
  (void)io;
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
  //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
  io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;        // Enable Docking
  io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

  ImGui_ImplGlfw_InitForOpenGL(Engine::managers_.GetManager<WindowManager*>()->GetHandle(), true);
  const char* glsl_version = "#version 460";
  ImGui_ImplOpenGL3_Init(glsl_version);

  // Setup Dear ImGui style
  SetStyle();

  // set up section path in UI
  auto* dm = Engine::managers_.GetManager<DeserializeManager*>();
  
  for (int i = 0; i < 2; ++i)
  {
    items[i] = dm->SectionPaths[i].c_str();
  }

  springs[0] = "Spring1";
  springs[1] = "Spring2";
  springs[2] = "Spring3";
  springs[3] = "Spring4";
  springs[4] = "Spring5";
  springs[5] = "Spring6";
  springs[6] = "Spring7";
  springs[7] = "Spring8";

  sticks[0] = "Sticks1";
  sticks[1] = "Sticks2";
  sticks[2] = "Sticks3";
  sticks[3] = "Sticks4";
  sticks[4] = "Sticks5";
  sticks[5] = "Sticks6";
  sticks[6] = "Sticks7";
}

void ImGuiUIManager::Update()
{
  bool show = true;
  auto* rm = Engine::managers_.GetManager<RenderManager*>();
  auto* am = Engine::managers_.GetManager<AnimationManager*>();
  // render imgui to default framebufer
  glBindFramebuffer(GL_FRAMEBUFFER, 0);

  NewFrame();

  ShowDockSpace(&show);

  // 1. Show the big demo window (Most of the sample code is in ImGui::ShowDemoWindow()! You can browse its code to learn more about Dear ImGui!).
  if (show_demo_window)
    ImGui::ShowDemoWindow(&show_demo_window);

#pragma region LOADMODEL_WINDOW
  //ImGui::Begin("Load Model");
  //ImGui::Text("Model Loading Drop Down:");
  //
  //static const char* current_items = "NULL";
  //if (ImGui::BeginCombo("##Models", *items))
  //{
  //  for (int n = 0; n < IM_ARRAYSIZE(items); ++n)
  //  {
  //    bool is_selected = (current_items == items[n]);
  //    if (ImGui::Selectable(items[n], is_selected))
  //    {
  //      current_items = items[n];
  //      loadedItems.push_back(std::string(current_items));
  //      Engine::managers_.GetManager<ObjectManager*>()->SectionLoader(current_items);
  //    }
  //    if (is_selected)
  //    {
  //      ImGui::SetItemDefaultFocus();
  //    }
  //  }
  //  ImGui::EndCombo();
  //}
  auto* om = Engine::managers_.GetManager<ObjectManager*>();

  //ImGui::Text("Selected section:");
  //ImGui::Text(current_items);
  //
  //ImGui::Text("Loaded section:");
  //for (auto& s : loadedItems)
  //{
  //  ImGui::Text(s.c_str());
  //}
  //
  //if (ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows) && ImGui::IsWindowHovered(ImGuiHoveredFlags_RootAndChildWindows))
  //  Engine::managers_.GetManager<InputManager*>()->glfw_used_flag = false;
  //ImGui::End();
#pragma endregion
#pragma region SETTING_WINDOW
  // setting window
  ImGui::Begin("Settings");

  ImGui::ColorEdit3("clear color", (float*)&Engine::managers_.GetManager<RenderManager*>()->clearColor_); // Edit 3 floats representing a color

  ImGui::Text("FBOs:");
  ImGui::Checkbox("Albedo FBO", &albedo_window);
  ImGui::Checkbox("Normal FBO", &normal_window);
  ImGui::Checkbox("Position FBO", &position_window);
  ImGui::Checkbox("Depth FBO", &depth_window);

  ImGui::Checkbox("Debug Draw", &rm->debugDraw);

  if (rm->debugDraw)
  {
    ImGui::Checkbox("Depth Copy", &rm->depthCopy);
    ImGui::RadioButton("Vertex Normal", &rm->debugDrawType, to_integral(RenderManager::DebugDrawType::VertexNormal));
    ImGui::SameLine();
    ImGui::RadioButton("Face Normal", &rm->debugDrawType, to_integral(RenderManager::DebugDrawType::FaceNormal));
    ImGui::RadioButton("Bounding Volume (Octree)", &rm->debugDrawType, to_integral(RenderManager::DebugDrawType::BoundingVolume));
    ImGui::RadioButton("Bsp Tree", &rm->debugDrawType, to_integral(RenderManager::DebugDrawType::BspTree));
    ImGui::RadioButton("GJK", &rm->debugDrawType, to_integral(RenderManager::DebugDrawType::GJK));
  }
  else
  {
    rm->debugDrawType = to_integral(RenderManager::DebugDrawType::Invalid);
  }

  ImGui::Text("Shadow:");
  ImGui::Checkbox("Cast Shadow", &rm->castShadow);

  if (rm->castShadow)
  {
    ImGui::Checkbox("PCF", &rm->PCF);
    ImGui::SliderFloat("Shadow Bias", &rm->shadowBias, 0.001f, 0.01f, "%.4f", ImGuiSliderFlags_AlwaysClamp);
  }

  ImGui::Text("Tone Mapping:");
  ImGui::Checkbox("Gamma Correction", &rm->gammaCorrection);
  if (rm->gammaCorrection)
    ImGui::SliderFloat("Exposure", &rm->exposure, 0.1f, 5.0f, "%.1f", ImGuiSliderFlags_AlwaysClamp);


  auto sunPos = om->sun->GetPosition();
  ImGui::Text("Directional Light:");
  ImGui::SliderFloat3("Position", glm::value_ptr(sunPos), -6000.f, 6000.f, "%1.f", ImGuiSliderFlags_AlwaysClamp);
  om->sun->SetPosition(sunPos);

  auto sunIntensity = rm->sun.SunIntensity;
  ImGui::SliderFloat("Intensity", &sunIntensity, 1.0f, 10.0f, "%.1f", ImGuiSliderFlags_AlwaysClamp);
  rm->sun.SunIntensity = sunIntensity;

  auto sunColor = rm->sun.SunColor;
  ImGui::ColorEdit3("Color", glm::value_ptr(sunColor));
  rm->sun.SunColor = sunColor;

  ImGui::Text("Global Ambient Light:");
  float ambientIntensity = rm->ambientLight;
  ImGui::SliderFloat("Ambient", &ambientIntensity, 0.1f, 0.5f, "%.2f", ImGuiSliderFlags_AlwaysClamp);
  rm->ambientLight = ambientIntensity;

  if (ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows) && ImGui::IsWindowHovered(ImGuiHoveredFlags_RootAndChildWindows))
    Engine::managers_.GetManager<InputManager*>()->glfw_used_flag = false;

  ImGui::End();
#pragma endregion

#pragma region PHYSICS_WINDOW
  if (physic_window)
  {
    auto* pm = Engine::managers_.GetManager<PhysicsManager*>();

    ImGui::Begin("Spring-damper Settings", &physic_window);

    ImGui::Checkbox("Start Simulation", &Engine::managers_.GetManager<PhysicsManager*>()->simulateFlag);

    ImGui::TextColored(ImVec4(0.f, 1.f, 0.f, 1.f), "Adjust position by using mouse to drag number");
    glm::vec3 leftAnchorPosition = pm->getLeftAnchorPointPosition();
    ImGui::DragFloat3("Left Anchor Position", glm::value_ptr(leftAnchorPosition), 50.f, -10000.f, 10000.f, "%.3f");
    pm->setLeftAnchorPointPosition(leftAnchorPosition);

    glm::vec3 rightAnchorPosition = pm->getRightAnchorPointPosition();
    ImGui::DragFloat3("Right Anchor Position", glm::value_ptr(rightAnchorPosition), 50.f, -10000.f, 10000.f, "%.3f");
    pm->setRightAnchorPointPosition(rightAnchorPosition);

    ImGui::TextColored(ImVec4(0.f, 1.f, 0.f, 1.f), "Global Coefficient");
    float g = pm->getGravity();
    ImGui::SetNextItemWidth(100.f);
    ImGui::DragFloat("Gravity", &g, 10.f, 10.f, 1000.f);
    pm->setGravity(g);

    ImGui::TextColored(ImVec4(0.f, 1.f, 0.f, 1.f), "Big spring constant will result in less stretching");
    ImGui::TextColored(ImVec4(0.f, 1.f, 0.f, 1.f), "Small spring constant will result in more stretching");
    ImGui::TextColored(ImVec4(0.f, 1.f, 0.f, 1.f), "Damping constants usage:");
    ImGui::TextColored(ImVec4(0.f, 1.f, 0.f, 1.f), "Equal 0 means un-damped");
    ImGui::TextColored(ImVec4(0.f, 1.f, 0.f, 1.f), "Less than 1 means under-damped");
    ImGui::TextColored(ImVec4(0.f, 1.f, 0.f, 1.f), "Equal 1 means critically damped");
    ImGui::TextColored(ImVec4(0.f, 1.f, 0.f, 1.f), "Greater than 1 means over-damped");


    ImGui::Begin("Springs Settings", &physic_window);
    ImGui::TextColored(ImVec4(0.f, 1.f, 0.f, 1.f), "Adjust value by using mouse to drag number");
    ImGui::Text("Choose which spring to edit:");
    static const char* current_spring = "NULL";
    static bool chosen = false;
    static int index = -1;
    ImGui::SetNextItemWidth(100.f);
    if (ImGui::BeginCombo(current_spring, *springs))
    {
      chosen = false;
      for (int n = 0; n < IM_ARRAYSIZE(springs); ++n)
      {
        bool is_selected = (current_spring == springs[n]);
        if (ImGui::Selectable(springs[n], is_selected))
        {
          chosen = true;
          current_spring = springs[n];
          std::string str = current_spring;
          char last = str.back();
          index = atoi(&last);
          index -= 1;
        }
      }
      ImGui::EndCombo();
    }
    
    if (chosen)
    {
      ImGui::TextColored(ImVec4(1.f, 0.f, 0.f, 1.f), "Editing");
      ImGui::SameLine();
      ImGui::TextColored(ImVec4(1.f, 0.f, 0.f, 1.f), current_spring);
      float k = pm->getSpringConstants(index);
      float d = pm->getDampingConstants(index);

      ImGui::SetNextItemWidth(100.f);
      ImGui::DragFloat("Spring Constant", &k, 1.f, 10.f, 100.f);
      ImGui::SetNextItemWidth(100.f);
      ImGui::DragFloat("Damping Constant", &d, 0.01f, 0.f, 1.f);
    
      pm->setSpringConstants(index, k);
      pm->setDampingConstants(index, d);
    }
    // enable glfw input
    if (ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows) && ImGui::IsWindowHovered(ImGuiHoveredFlags_RootAndChildWindows))
      Engine::managers_.GetManager<InputManager*>()->glfw_used_flag = false;
    ImGui::End();


    ImGui::Begin("Sticks Settings", &physic_window);
    ImGui::TextColored(ImVec4(0.f, 1.f, 0.f, 1.f), "Adjust value by using mouse to drag number");
    ImGui::Text("Choose which stick to edit:");

    static const char* current_stick = "NULL";
    static bool chosenStick = false;
    static int indexStick = -1;
    ImGui::SetNextItemWidth(100.f);
    if (ImGui::BeginCombo(current_stick, *sticks))
    {
      chosenStick = false;
      for (int n = 0; n < IM_ARRAYSIZE(sticks); ++n)
      {
        bool is_selected = (current_stick == sticks[n]);
        if (ImGui::Selectable(sticks[n], is_selected))
        {
          chosenStick = true;
          current_stick = sticks[n];
          std::string str = current_stick;
          char last = str.back();
          indexStick = atoi(&last);
        }
      }
      ImGui::EndCombo();
    }

    if (chosenStick)
    {
      ImGui::TextColored(ImVec4(1.f, 0.f, 0.f, 1.f), "Editing");
      ImGui::SameLine();
      ImGui::TextColored(ImVec4(1.f, 0.f, 0.f, 1.f), current_stick);

      float m = pm->getTotalMass(indexStick);

      ImGui::SetNextItemWidth(100.f);
      ImGui::DragFloat("Mass", &m, 1.f, 5.f, 50.f);
      pm->setTotalMass(indexStick, m);
    }
    // enable glfw input
    if (ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows) && ImGui::IsWindowHovered(ImGuiHoveredFlags_RootAndChildWindows))
      Engine::managers_.GetManager<InputManager*>()->glfw_used_flag = false;
    ImGui::End();

    // enable glfw input
    if (ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows) && ImGui::IsWindowHovered(ImGuiHoveredFlags_RootAndChildWindows))
      Engine::managers_.GetManager<InputManager*>()->glfw_used_flag = false;
    ImGui::End();
  }
#pragma endregion

#pragma region ANIMATION_WINDOW
  if (animation_window)
  {
    ImGui::Begin("Animation Settings", &animation_window);

    ImGui::Checkbox("Bone Draw", &rm->boneDraw);

    ImGui::Checkbox("Play Animation", &am->PlayAnimation);

    ImGui::Checkbox("Path Draw", &rm->splineDraw);

    auto* ikm = Engine::managers_.GetManager<InverseKinematicManager*>();
    ImGui::Checkbox("Start IK", &ikm->runFlag);

    ImGui::Checkbox("IK Chain Draw", &rm->IKChainDraw);

    ImGui::Checkbox("Apply Constraint", &ikm->m_CCDSolver.applyConstraint);

    if (am->PlayAnimation)
    {
      ImGui::Text("Sliding & Skidding Control");

      ImGui::InputFloat("Value", &am->animator->SlidingSkiddingControl);
    }

    // enable glfw input
    if (ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows) && ImGui::IsWindowHovered(ImGuiHoveredFlags_RootAndChildWindows))
      Engine::managers_.GetManager<InputManager*>()->glfw_used_flag = false;
    ImGui::End();
  }
#pragma endregion
#pragma region PATH_WINDOW
  if (path_window)
  {
    static bool show_labels = true;
    ImGui::Begin("Path Control", &path_window);
    //ImPlot::ShowDemoWindow(&path_window);

    auto* sm = Engine::managers_.GetManager<SplineManager*>();
    Spline& curve = sm->GetCurve(0); // get the first curve to test first

    ImGui::Text("Parabolic Approach Speed Control (0 <= t1 <= t2 <= 1)");
    if (ImGui::DragFloatRange2("(t1, t2)", &sm->t1, &sm->t2, 0.01f, 0.f, 1.f))
    {
      sm->Vc = 2.f / (1.f - sm->t1 + sm->t2);
    }

    // plotting velocity-time function
    if (am->PlayAnimation)
    {
      static RollingBuffer rdata1;
      static float t = 0;
      t += ImGui::GetIO().DeltaTime;
      rdata1.AddPoint(t, am->animator->speed);

      static float history = 30.f;
      //ImGui::SliderFloat("History", &history, 1, 30, "%.1f s");
      rdata1.Span = history;

      //static ImPlotAxisFlags flags = ImPlotAxisFlags_NoTickLabels;
      ImPlot::SetNextPlotLimitsX(-1, history, ImGuiCond_Always);
      ImPlot::SetNextPlotLimitsY(-1, 3);
      if (ImPlot::BeginPlot("Velocity-Time Graph", "t", "v", ImVec2(-1, 200), ImPlotFlags_AntiAliased))
      {
        ImPlot::PlotLine("##Velocity", &rdata1.Data[0].x, &rdata1.Data[0].y, rdata1.Data.size(), 0, 2 * sizeof(float));
        ImPlot::EndPlot();
      }
    }

    if (ImGui::RadioButton("Adaptive Approach", &curve.method, to_integral(ArcLengthFunctionType::AdaptiveApproach)))
    {
      // update curve
      curve.dirtyFlag = true;
    }
    if (ImGui::RadioButton("Forward Differencing Approach", &curve.method, to_integral(ArcLengthFunctionType::ForwardDifferencing)))
    {
      // update curve
      curve.dirtyFlag = true;
    }

    // plotting space curve
    ImPlot::SetNextPlotLimitsX(-15, 15, ImGuiCond_Always);
    ImPlot::SetNextPlotLimitsY(-15, 15);
    if (ImPlot::BeginPlot("Graph", "x", "y",ImVec2(-1,250),ImPlotFlags_AntiAliased))
    {
      auto& controlPts = curve.GetControlPoints();
      auto& interpolatePts = curve.GetInterpolatePoints();

      // create proxy for implot point
      std::vector<ImPlotPoint> plotPts;
      plotPts.resize(controlPts.size());

      for (int i = 0; i < controlPts.size(); ++i)
      {
        // copy data over
        plotPts[i] = ImPlotPoint(controlPts[i].x, controlPts[i].z);

        std::string name = "P" + std::to_string(i);

        // draggable point
        if (ImPlot::DragPoint(name.c_str(), &plotPts[i].x, &plotPts[i].y, 
          show_labels, ImVec4(0.580f, 0.580f, 0.580f,1.f)))
        {
          // update data back
          controlPts[i].x = plotPts[i].x;
          controlPts[i].z = plotPts[i].y;
          // update curve
          curve.dirtyFlag = true;
        }
      }

      //ImPlot::SetNextMarkerStyle(ImPlotMarker_Circle);
      ImPlot::PlotLine("Path", &interpolatePts[0].x, &interpolatePts[0].z,
        interpolatePts.size(),
        0,
        sizeof(glm::vec3));

      ImPlot::EndPlot();
    }
    // enable glfw input
    if (ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows) && ImGui::IsWindowHovered(ImGuiHoveredFlags_RootAndChildWindows))
      Engine::managers_.GetManager<InputManager*>()->glfw_used_flag = false;
    ImGui::End();
  }
#pragma endregion
#pragma region ALBEDO_WINDOW
  // 3. Show another simple window.
  if (albedo_window)
  {
    ImGui::Begin("Albedo FBO", &albedo_window);

    ImGui::BeginChild("FBO");

    ImVec2 wsize = ImGui::GetWindowSize();
    ImVec2 center = ImVec2(wsize.x - (float)rm->gbuffer_.width / 4.0f, wsize.y - (float)rm->gbuffer_.height / 4.0f);
    center = ImVec2(center.x * 0.5f, center.y * 0.5f);
    ImGui::SetCursorPos(center);

    ImGui::Image((void*)(intptr_t)rm->gbuffer_.GetTextureID(Layout::Albedo),
      ImVec2((float)rm->gbuffer_.width / 4.0f, (float)rm->gbuffer_.height / 4.0f),
      ImVec2(0, 1), ImVec2(1, 0));

    // enable glfw input
    if (ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows) && ImGui::IsWindowHovered(ImGuiHoveredFlags_RootAndChildWindows))
      Engine::managers_.GetManager<InputManager*>()->glfw_used_flag = true;

    ImGui::EndChild();

    ImGui::End();
  }
#pragma endregion
#pragma region NORMAL_WINDOW
  if (normal_window)
  {
    ImGui::Begin("Normal FBO", &normal_window);

    ImGui::BeginChild("FBO");

    ImVec2 wsize = ImGui::GetWindowSize();
    ImVec2 center = ImVec2(wsize.x - (float)rm->gbuffer_.width / 4.0f, wsize.y - (float)rm->gbuffer_.height / 4.0f);
    center = ImVec2(center.x * 0.5f, center.y * 0.5f);
    ImGui::SetCursorPos(center);

    ImGui::Image((void*)(intptr_t)rm->gbuffer_.GetTextureID(Layout::Normal),
      ImVec2((float)rm->gbuffer_.width / 4.0f, (float)rm->gbuffer_.height / 4.0f),
      ImVec2(0, 1), ImVec2(1, 0));

    // enable glfw input
    if (ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows) && ImGui::IsWindowHovered())
      Engine::managers_.GetManager<InputManager*>()->glfw_used_flag = true;

    ImGui::EndChild();

    ImGui::End();
  }
#pragma endregion
#pragma region POSITION_WINDOW
  if (position_window)
  {
    ImGui::Begin("Position FBO", &position_window);

    ImGui::BeginChild("FBO");

    ImVec2 wsize = ImGui::GetWindowSize();
    ImVec2 center = ImVec2(wsize.x - (float)rm->gbuffer_.width / 4.0f, wsize.y - (float)rm->gbuffer_.height / 4.0f);
    center = ImVec2(center.x * 0.5f, center.y * 0.5f);
    ImGui::SetCursorPos(center);

    ImGui::Image((void*)(intptr_t)rm->gbuffer_.GetTextureID(Layout::Position),
      ImVec2((float)rm->gbuffer_.width / 4.0f, (float)rm->gbuffer_.height / 4.0f),
      ImVec2(0, 1), ImVec2(1, 0));

    // enable glfw input
    if (ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows) && ImGui::IsWindowHovered())
      Engine::managers_.GetManager<InputManager*>()->glfw_used_flag = true;

    ImGui::EndChild();

    ImGui::End();
  }
#pragma endregion
#pragma region SPECULAR_WINDOW
  if (specular_window)
  {
    ImGui::Begin("Specular FBO", &specular_window);

    ImGui::BeginChild("FBO");

    ImVec2 wsize = ImGui::GetWindowSize();
    ImVec2 center = ImVec2(wsize.x - (float)rm->gbuffer_.width / 4.0f, wsize.y - (float)rm->gbuffer_.height / 4.0f);
    center = ImVec2(center.x * 0.5f, center.y * 0.5f);
    ImGui::SetCursorPos(center);

    ImGui::Image((void*)(intptr_t)rm->gbuffer_.GetTextureID(Layout::Specular),
      ImVec2((float)rm->gbuffer_.width / 4.0f, (float)rm->gbuffer_.height / 4.0f),
      ImVec2(0, 1), ImVec2(1, 0));

    // enable glfw input
    if (ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows) && ImGui::IsWindowHovered())
      Engine::managers_.GetManager<InputManager*>()->glfw_used_flag = true;

    ImGui::EndChild();

    ImGui::End();
  }
#pragma endregion
#pragma region DEPTH_WINDOW
  if (depth_window)
  {
    ImGui::Begin("Depth FBO", &depth_window);

    ImGui::BeginChild("FBO");

    ImVec2 wsize = ImGui::GetWindowSize();
    ImVec2 center = ImVec2(wsize.x - (float)rm->gbuffer_.width / 4.0f, wsize.y - (float)rm->gbuffer_.height / 4.0f);
    center = ImVec2(center.x * 0.5f, center.y * 0.5f);
    ImGui::SetCursorPos(center);

    ImGui::Image((void*)(intptr_t)rm->gbuffer_.GetTextureID(Layout::Depth),
      ImVec2((float)rm->gbuffer_.width / 4.0f, (float)rm->gbuffer_.height / 4.0f),
      ImVec2(0, 1), ImVec2(1, 0));

    // enable glfw input
    if (ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows) && ImGui::IsWindowHovered())
      Engine::managers_.GetManager<InputManager*>()->glfw_used_flag = true;

    ImGui::EndChild();

    ImGui::End();
  }
#pragma endregion
#pragma region SHADOWMAP_WINDOW
  if (shadowMap_window)
  {
    ImGui::Begin("Shadow Map", &shadowMap_window);

    ImGui::BeginChild("FBO");

    ImVec2 wsize = ImGui::GetWindowSize();
    ImVec2 center = ImVec2(wsize.x - (float)rm->shadowFBO_.width / 4.0f, wsize.y - (float)rm->shadowFBO_.height / 4.0f);
    center = ImVec2(center.x * 0.5f, center.y * 0.5f);
    ImGui::SetCursorPos(center);

    ImGui::Image((void*)(intptr_t)rm->shadowFBO_.textureID,
      ImVec2((float)rm->shadowFBO_.width / 4.0f, (float)rm->shadowFBO_.height / 4.0f),
      ImVec2(0, 1), ImVec2(1, 0));

    // enable glfw input
    if (ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows) && ImGui::IsWindowHovered())
      Engine::managers_.GetManager<InputManager*>()->glfw_used_flag = true;

    ImGui::EndChild();

    ImGui::End();
  }
#pragma endregion
#pragma region OCTREE_WINDOW
  if (octree_window)
  {
    ImGui::Begin("Octree Settings", &octree_window);

    if (om->octreeController.treeEmpty)
    {
      ImGui::Text("Terminating condition:");
      ImGui::DragInt("Triangles", &om->octreeController.max_triangles);
      ImGui::Checkbox("Build", &om->octreeController.buildFlag);
      ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "TREE EMPTY!");
    }
      
    if (om->octreeController.treeReady)
    {
      ImGui::Checkbox("Delete Tree", &om->octreeController.deleteFlag);
      ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "TREE READY!\nMAKE SURE TO TURN ON DEBUG DRAW TO SEE");
    }

    // enable glfw input
    if (ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows) && ImGui::IsWindowHovered(ImGuiHoveredFlags_RootAndChildWindows))
      Engine::managers_.GetManager<InputManager*>()->glfw_used_flag = false;

    ImGui::End();
  }
#pragma endregion
#pragma region BSP_WINDOW
  if (bsp_window)
  {
    ImGui::Begin("BSP Tree Settings", &bsp_window);

    if (om->bsptreeConroller.treeEmpty)
    {
      //ImGui::Text("Terminating condition:");
      //ImGui::DragInt("Triangles", &om->bsptreeConroller.max_triangles);
      ImGui::Checkbox("Build", &om->bsptreeConroller.buildFlag);
      ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "TREE EMPTY!");
    }

    if (om->bsptreeConroller.treeReady)
    {
      ImGui::Checkbox("Delete Tree", &om->bsptreeConroller.deleteFlag);
      ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "TREE READY!\nMAKE SURE TO TURN ON DEBUG DRAW TO SEE");
    }

    // enable glfw input
    if (ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows) && ImGui::IsWindowHovered(ImGuiHoveredFlags_RootAndChildWindows))
      Engine::managers_.GetManager<InputManager*>()->glfw_used_flag = false;

    ImGui::End();
  }
#pragma endregion
#pragma region MODEL_WINDOW
  if (model_window)
  {
    ImGui::Begin("Model Settings", &model_window);

    ImGui::Checkbox("Show Models", &om->renderModel);
    //ImGui::Checkbox("Delete All Models", &om->deleteModel);

    // enable glfw input
    if (ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows) && ImGui::IsWindowHovered(ImGuiHoveredFlags_RootAndChildWindows))
      Engine::managers_.GetManager<InputManager*>()->glfw_used_flag = false;

    ImGui::End();
  }
#pragma endregion
#pragma region GJK_WINDOW
  if (gjk_window)
  {
    ImGui::Begin("GJK Settings", &gjk_window);

    if (!om->gjkController.startFlag)
    {
      ImGui::Checkbox("Start", &om->gjkController.startFlag);
      ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "MAKE SURE TO BUILD OCTREE FIRST!!!\nCLICK START TO BEGIN GJK-ALGORITHM");
    }

    if (om->gjkController.stopFlag)
    {
      ImGui::Checkbox("Reset", &om->gjkController.resetFlag);
      ImGui::Checkbox("Draw Simplex", &rm->simplexDraw);
      ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "COLLISION DETECTED");
    }

    // enable glfw input
    if (ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows) && ImGui::IsWindowHovered(ImGuiHoveredFlags_RootAndChildWindows))
      Engine::managers_.GetManager<InputManager*>()->glfw_used_flag = false;

    ImGui::End();
  }

  if (rm->simplexDraw)
  {
    ImGui::Begin("Simplex Viewport");
    ImGui::BeginChild("Scene");

    ImVec2 wsize = ImGui::GetWindowSize();
    ImVec2 center = ImVec2(wsize.x - (float)rm->simplexFbo_.width / 4.0f, wsize.y - (float)rm->simplexFbo_.height / 4.0f);
    center = ImVec2(center.x * 0.5f, center.y * 0.5f);
    ImGui::SetCursorPos(center);

    ImGui::Image((void*)(intptr_t)rm->simplexFbo_.textureID,
      ImVec2((float)rm->simplexFbo_.width / 4.0f, (float)rm->simplexFbo_.height / 4.0f),
      ImVec2(0, 1), ImVec2(1, 0));

    // enable glfw input
    if (ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows) && ImGui::IsWindowHovered())
      Engine::managers_.GetManager<InputManager*>()->glfw_used_flag = true;

    ImGui::EndChild();
    ImGui::End();
  }
#pragma endregion
#pragma region VIEWPORT
  ImGui::Begin("Viewport");
  ImGui::BeginChild("Scene");

  ImVec2 wsize = ImGui::GetWindowSize();
  ImVec2 center = ImVec2(wsize.x - (float)rm->fsqFbo_.width, wsize.y - (float)rm->fsqFbo_.height);
  center = ImVec2(center.x * 0.5f, center.y * 0.5f);
  ImGui::SetCursorPos(center);

  ImGui::Image((void*)(intptr_t)rm->fsqFbo_.textureID,
    ImVec2((float)rm->fsqFbo_.width, (float)rm->fsqFbo_.height),
    ImVec2(0, 1), ImVec2(1, 0));

  // enable glfw input
  if (ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows) && ImGui::IsWindowHovered())
    Engine::managers_.GetManager<InputManager*>()->glfw_used_flag = true;

  ImGui::EndChild();
  ImGui::End();
#pragma endregion
  // Rendering
  ImGui::Render();

  ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

  // Update and Render additional Platform Windows
  // (Platform functions may change the current OpenGL context, so we save/restore it to make it easier to paste this code elsewhere.
  //  For this specific demo app we could also call glfwMakeContextCurrent(window) directly)
  if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
  {
    GLFWwindow* backup_current_context = glfwGetCurrentContext();
    ImGui::UpdatePlatformWindows();
    ImGui::RenderPlatformWindowsDefault();
    glfwMakeContextCurrent(backup_current_context);
  }
}

void ImGuiUIManager::NewFrame()
{
  // Start the Dear ImGui frame
  ImGui_ImplOpenGL3_NewFrame();
  ImGui_ImplGlfw_NewFrame();
  ImGui::NewFrame();
}

void ImGuiUIManager::SetStyle()
{
  ImGuiStyle* style = &ImGui::GetStyle();
  ImVec4* colors = style->Colors;

  colors[ImGuiCol_Text] = ImVec4(1.000f, 1.000f, 1.000f, 1.000f);
  colors[ImGuiCol_TextDisabled] = ImVec4(0.500f, 0.500f, 0.500f, 1.000f);
  colors[ImGuiCol_WindowBg] = ImVec4(0.180f, 0.180f, 0.180f, 1.000f);
  colors[ImGuiCol_ChildBg] = ImVec4(0.280f, 0.280f, 0.280f, 0.000f);
  colors[ImGuiCol_PopupBg] = ImVec4(0.313f, 0.313f, 0.313f, 1.000f);
  colors[ImGuiCol_Border] = ImVec4(0.266f, 0.266f, 0.266f, 1.000f);
  colors[ImGuiCol_BorderShadow] = ImVec4(0.000f, 0.000f, 0.000f, 0.000f);
  colors[ImGuiCol_FrameBg] = ImVec4(0.160f, 0.160f, 0.160f, 1.000f);
  colors[ImGuiCol_FrameBgHovered] = ImVec4(0.200f, 0.200f, 0.200f, 1.000f);
  colors[ImGuiCol_FrameBgActive] = ImVec4(0.280f, 0.280f, 0.280f, 1.000f);
  colors[ImGuiCol_TitleBg] = ImVec4(0.148f, 0.148f, 0.148f, 1.000f);
  colors[ImGuiCol_TitleBgActive] = ImVec4(0.148f, 0.148f, 0.148f, 1.000f);
  colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.148f, 0.148f, 0.148f, 1.000f);
  colors[ImGuiCol_MenuBarBg] = ImVec4(0.195f, 0.195f, 0.195f, 1.000f);
  colors[ImGuiCol_ScrollbarBg] = ImVec4(0.160f, 0.160f, 0.160f, 1.000f);
  colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.277f, 0.277f, 0.277f, 1.000f);
  colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.300f, 0.300f, 0.300f, 1.000f);
  colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(1.000f, 0.391f, 0.000f, 1.000f);
  colors[ImGuiCol_CheckMark] = ImVec4(1.000f, 1.000f, 1.000f, 1.000f);
  colors[ImGuiCol_SliderGrab] = ImVec4(0.391f, 0.391f, 0.391f, 1.000f);
  colors[ImGuiCol_SliderGrabActive] = ImVec4(1.000f, 0.391f, 0.000f, 1.000f);
  colors[ImGuiCol_Button] = ImVec4(1.000f, 1.000f, 1.000f, 0.000f);
  colors[ImGuiCol_ButtonHovered] = ImVec4(1.000f, 1.000f, 1.000f, 0.156f);
  colors[ImGuiCol_ButtonActive] = ImVec4(1.000f, 1.000f, 1.000f, 0.391f);
  colors[ImGuiCol_Header] = ImVec4(0.313f, 0.313f, 0.313f, 1.000f);
  colors[ImGuiCol_HeaderHovered] = ImVec4(0.469f, 0.469f, 0.469f, 1.000f);
  colors[ImGuiCol_HeaderActive] = ImVec4(0.469f, 0.469f, 0.469f, 1.000f);
  colors[ImGuiCol_Separator] = colors[ImGuiCol_Border];
  colors[ImGuiCol_SeparatorHovered] = ImVec4(0.391f, 0.391f, 0.391f, 1.000f);
  colors[ImGuiCol_SeparatorActive] = ImVec4(1.000f, 0.391f, 0.000f, 1.000f);
  colors[ImGuiCol_ResizeGrip] = ImVec4(1.000f, 1.000f, 1.000f, 0.250f);
  colors[ImGuiCol_ResizeGripHovered] = ImVec4(1.000f, 1.000f, 1.000f, 0.670f);
  colors[ImGuiCol_ResizeGripActive] = ImVec4(1.000f, 0.391f, 0.000f, 1.000f);
  colors[ImGuiCol_Tab] = ImVec4(0.098f, 0.098f, 0.098f, 1.000f);
  colors[ImGuiCol_TabHovered] = ImVec4(0.352f, 0.352f, 0.352f, 1.000f);
  colors[ImGuiCol_TabActive] = ImVec4(0.195f, 0.195f, 0.195f, 1.000f);
  colors[ImGuiCol_TabUnfocused] = ImVec4(0.098f, 0.098f, 0.098f, 1.000f);
  colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.195f, 0.195f, 0.195f, 1.000f);
  colors[ImGuiCol_DockingPreview] = ImVec4(1.000f, 0.391f, 0.000f, 0.781f);
  colors[ImGuiCol_DockingEmptyBg] = ImVec4(0.180f, 0.180f, 0.180f, 1.000f);
  colors[ImGuiCol_PlotLines] = ImVec4(0.469f, 0.469f, 0.469f, 1.000f);
  colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.000f, 0.391f, 0.000f, 1.000f);
  colors[ImGuiCol_PlotHistogram] = ImVec4(0.586f, 0.586f, 0.586f, 1.000f);
  colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.000f, 0.391f, 0.000f, 1.000f);
  colors[ImGuiCol_TextSelectedBg] = ImVec4(1.000f, 1.000f, 1.000f, 0.156f);
  colors[ImGuiCol_DragDropTarget] = ImVec4(1.000f, 0.391f, 0.000f, 1.000f);
  colors[ImGuiCol_NavHighlight] = ImVec4(1.000f, 0.391f, 0.000f, 1.000f);
  colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.000f, 0.391f, 0.000f, 1.000f);
  colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.000f, 0.000f, 0.000f, 0.586f);
  colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.000f, 0.000f, 0.000f, 0.586f);

  style->ChildRounding = 4.0f;
  style->FrameBorderSize = 1.0f;
  style->FrameRounding = 2.0f;
  style->GrabMinSize = 7.0f;
  style->PopupRounding = 2.0f;
  style->ScrollbarRounding = 12.0f;
  style->ScrollbarSize = 13.0f;
  style->TabBorderSize = 1.0f;
  style->TabRounding = 0.0f;
  style->WindowRounding = 4.0f;
}

void ImGuiUIManager::ShowDockSpace(bool* p_open)
{
  static bool opt_fullscreen_persistant = true;
  bool opt_fullscreen = opt_fullscreen_persistant;
  static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;

  // We are using the ImGuiWindowFlags_NoDocking flag to make the parent window not dockable into,
  // because it would be confusing to have two docking targets within each others.
  ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
  if (opt_fullscreen)
  {
    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->Pos);
    ImGui::SetNextWindowSize(viewport->Size);
    ImGui::SetNextWindowViewport(viewport->ID);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
    window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
  }

  // When using ImGuiDockNodeFlags_PassthruCentralNode, DockSpace() will render our background 
  // and handle the pass-thru hole, so we ask Begin() to not render a background.
  if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
    window_flags |= ImGuiWindowFlags_NoBackground;

  // Important: note that we proceed even if Begin() returns false (aka window is collapsed).
  // This is because we want to keep our DockSpace() active. If a DockSpace() is inactive,
  // all active windows docked into it will lose their parent and become undocked.
  // We cannot preserve the docking relationship between an active window and an inactive docking, otherwise
  // any change of dockspace/settings would lead to windows being stuck in limbo and never being visible.
  ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
  ImGui::Begin("###DockSpace", p_open, window_flags);
  ImGui::PopStyleVar();

  if (opt_fullscreen)
    ImGui::PopStyleVar(2);

  // DockSpace
  ImGuiIO& io = ImGui::GetIO();
  if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
  {
    ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
    ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
  }

  ImGui::End();
}


