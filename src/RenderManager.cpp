#include "RenderManager.h"
#include "Engine.h"
#include "Shader.h"
#include "Transform.h"

void RenderManager::Setup()
{
  auto* wm = Engine::managers_.GetManager<WindowManager*>();
  //glfwGetFramebufferSize(wm->GetHandle(), &width, &height);
  width = static_cast<int>(wm->resolution_.x);
  height = static_cast<int>(wm->resolution_.y);
  gbuffer_.SetUp(width, height);
  lightFbo_.SetUp(width, height, 5);
  fsqFbo_.SetUp(width, height, 6);
  shadowFBO_.SetUp(width, height, 7);
  simplexFbo_.SetUp(width, height, 6);

  MRT_Program = new ShaderProgram();
  MRT_Program->AddShader("./shaders/MRT.vert", GL_VERTEX_SHADER);
  MRT_Program->AddShader("./shaders/MRT.frag", GL_FRAGMENT_SHADER);
  MRT_Program->LinkProgram();

  BRDF_Program = new ShaderProgram();
  BRDF_Program->AddShader("./shaders/BRDF.vert", GL_VERTEX_SHADER);
  BRDF_Program->AddShader("./shaders/BRDF.frag", GL_FRAGMENT_SHADER);
  BRDF_Program->LinkProgram();

  FSQ_Program = new ShaderProgram();
  FSQ_Program->AddShader("./shaders/FSQ.vert", GL_VERTEX_SHADER);
  FSQ_Program->AddShader("./shaders/FSQ.frag", GL_FRAGMENT_SHADER);
  FSQ_Program->LinkProgram();

  Shadow_Program = new ShaderProgram();
  Shadow_Program->AddShader("./shaders/shadow.vert", GL_VERTEX_SHADER);
  Shadow_Program->AddShader("./shaders/shadow.frag", GL_FRAGMENT_SHADER);
  Shadow_Program->LinkProgram();

  Debug_Program = new ShaderProgram();
  Debug_Program->AddShader("./shaders/DEBUG.vert", GL_VERTEX_SHADER);
  Debug_Program->AddShader("./shaders/DEBUG.frag", GL_FRAGMENT_SHADER);
  Debug_Program->LinkProgram();

  Bone_Program = new ShaderProgram();
  Bone_Program->AddShader("./shaders/bone.vert", GL_VERTEX_SHADER);
  Bone_Program->AddShader("./shaders/bone.frag", GL_FRAGMENT_SHADER);
  Bone_Program->LinkProgram();

  Spline_Program = new ShaderProgram();
  Spline_Program->AddShader("./shaders/spline.vert", GL_VERTEX_SHADER);
  Spline_Program->AddShader("./shaders/spline.frag", GL_FRAGMENT_SHADER);
  Spline_Program->LinkProgram();

  IK_Program = new ShaderProgram();
  IK_Program->AddShader("./shaders/ik.vert", GL_VERTEX_SHADER);
  IK_Program->AddShader("./shaders/ik.frag", GL_FRAGMENT_SHADER);
  IK_Program->LinkProgram();

  Spring_Program = new ShaderProgram();
  Spring_Program->AddShader("./shaders/spring.vert", GL_VERTEX_SHADER);
  Spring_Program->AddShader("./shaders/spring.frag", GL_FRAGMENT_SHADER);
  Spring_Program->LinkProgram();

  clearColor_ = { 0.22f,0.22f,0.22f };
  // create empty vao
  glCreateVertexArrays(1, &emptyVAOid_);
  sun.SunIntensity = 5.0f;
  sun.SunColor = { 1.0f,136.f / 255.f,53.f / 255.f };
  ambientLight = 0.15f;
}

void RenderManager::Update()
{
  MRT_Pass();
  ShadowPass();
  LightingPass();
  if (debugDraw)
    DebugPass(static_cast<DebugDrawType>(debugDrawType));
  if (simplexDraw)
    SimplexPass(DebugDrawType::Simplex);
  if (boneDraw)
    BonePass();
  if (splineDraw)
    SplinePass();
  if (IKChainDraw)
    IKChainPass();
  SpringPass();
  FSQ();
}

void RenderManager::FSQ()
{
  glViewport(0, 0, width, height);
  CHECKERROR;
  glDisable(GL_DEPTH_TEST);
  fsqFbo_.Bind();
  glClearColor(clearColor_.r, clearColor_.g, clearColor_.b, 1);
  glClear(GL_COLOR_BUFFER_BIT);
  FSQ_Program->Use();

  glBindTextureUnit(lightFbo_.textureUnit, lightFbo_.textureID);
  int loc = glGetUniformLocation(FSQ_Program->programID, "screenTexture");
  glUniform1i(loc, static_cast<int>(lightFbo_.textureUnit));

  loc = glGetUniformLocation(FSQ_Program->programID, "exposure");
  glUniform1f(loc, exposure);

  loc = glGetUniformLocation(FSQ_Program->programID, "gammaCorrection");
  glUniform1i(loc, static_cast<int>(gammaCorrection));

  glBindVertexArray(emptyVAOid_);
  CHECKERROR;
  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
  glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
  CHECKERROR;

  glBindVertexArray(0);
  FSQ_Program->UnUse();
  fsqFbo_.Unbind();
  CHECKERROR;
  fsqFbo_.generateMipMap();
}

void RenderManager::DebugPass(DebugDrawType type)
{
  glViewport(0, 0, width, height);
  CHECKERROR;
  glEnable(GL_DEPTH_TEST);
  CHECKERROR;
  lightFbo_.Bind();
  Debug_Program->Use();

  if (depthCopy)
  {
    CHECKERROR;
    glBlitNamedFramebuffer(gbuffer_.GetGBufferID(), lightFbo_.fboID, 0, 0, width, height, 0, 0, width, height, GL_DEPTH_BUFFER_BIT, GL_NEAREST);
    CHECKERROR;
  }

  int loc = glGetUniformLocation(Debug_Program->programID, "WorldView");
  glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(Engine::managers_.GetManager<CameraManager*>()->WorldView));
  CHECKERROR;

  loc = glGetUniformLocation(Debug_Program->programID, "WorldProj");
  glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(Engine::managers_.GetManager<CameraManager*>()->WorldProj));
  CHECKERROR;

  Engine::managers_.GetManager<ObjectManager*>()->DebugDraw(Debug_Program, type);
  CHECKERROR;
  Debug_Program->UnUse();
  lightFbo_.Unbind();
}

void RenderManager::SimplexPass(DebugDrawType type)
{
  glViewport(0, 0, simplexFbo_.width, simplexFbo_.height);
  CHECKERROR;
  simplexFbo_.Bind();
  glEnable(GL_DEPTH_TEST);
  glClearColor(clearColor_.r, clearColor_.g, clearColor_.b, 1);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  CHECKERROR;
  Debug_Program->Use();

  int loc = glGetUniformLocation(Debug_Program->programID, "WorldView");
  glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(Engine::managers_.GetManager<CameraManager*>()->WorldView));
  CHECKERROR;

  loc = glGetUniformLocation(Debug_Program->programID, "WorldProj");
  glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(Engine::managers_.GetManager<CameraManager*>()->WorldProj));
  CHECKERROR;

  Engine::managers_.GetManager<ObjectManager*>()->DebugDraw(Debug_Program, type);
  CHECKERROR;
  Debug_Program->UnUse();
  simplexFbo_.Unbind();
  simplexFbo_.generateMipMap(); // need to generate mip map to display correctly using imgui image
}

void RenderManager::BonePass()
{
  glViewport(0, 0, width, height);
  CHECKERROR;
  //glDisable(GL_DEPTH_TEST); // so bone always in front of model
  glEnable(GL_DEPTH_TEST);
  CHECKERROR;
  lightFbo_.Bind();
  Bone_Program->Use();

  // depth copy
  CHECKERROR;
  glBlitNamedFramebuffer(gbuffer_.GetGBufferID(), lightFbo_.fboID, 0, 0, width, height, 0, 0, width, height, GL_DEPTH_BUFFER_BIT, GL_NEAREST);
  CHECKERROR;

  int loc = glGetUniformLocation(Bone_Program->programID, "WorldView");
  glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(Engine::managers_.GetManager<CameraManager*>()->WorldView));
  CHECKERROR;

  loc = glGetUniformLocation(Bone_Program->programID, "WorldProj");
  glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(Engine::managers_.GetManager<CameraManager*>()->WorldProj));
  CHECKERROR;

  Engine::managers_.GetManager<AnimationManager*>()->DrawBone(Bone_Program);

  CHECKERROR;
  Bone_Program->UnUse();
  lightFbo_.Unbind();
}

void RenderManager::SplinePass()
{
  glViewport(0, 0, width, height);
  CHECKERROR;
  glEnable(GL_DEPTH_TEST);
  CHECKERROR;
  lightFbo_.Bind();
  Spline_Program->Use();

  // depth copy
  CHECKERROR;
  glBlitNamedFramebuffer(gbuffer_.GetGBufferID(), lightFbo_.fboID, 0, 0, width, height, 0, 0, width, height, GL_DEPTH_BUFFER_BIT, GL_NEAREST);
  CHECKERROR;

  int loc = glGetUniformLocation(Spline_Program->programID, "WorldView");
  glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(Engine::managers_.GetManager<CameraManager*>()->WorldView));
  CHECKERROR;

  loc = glGetUniformLocation(Spline_Program->programID, "WorldProj");
  glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(Engine::managers_.GetManager<CameraManager*>()->WorldProj));
  CHECKERROR;

  Engine::managers_.GetManager<SplineManager*>()->Draw(Spline_Program);

  CHECKERROR;
  Spline_Program->UnUse();
  lightFbo_.Unbind();
}

void RenderManager::IKChainPass()
{
  glViewport(0, 0, width, height);
  CHECKERROR;
  //glDisable(GL_DEPTH_TEST); // so bone always in front of model
  glEnable(GL_DEPTH_TEST);
  CHECKERROR;
  lightFbo_.Bind();
  IK_Program->Use();

  // depth copy
  CHECKERROR;
  glBlitNamedFramebuffer(gbuffer_.GetGBufferID(), lightFbo_.fboID, 0, 0, width, height, 0, 0, width, height, GL_DEPTH_BUFFER_BIT, GL_NEAREST);
  CHECKERROR;

  int loc = glGetUniformLocation(IK_Program->programID, "WorldView");
  glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(Engine::managers_.GetManager<CameraManager*>()->WorldView));
  CHECKERROR;

  loc = glGetUniformLocation(IK_Program->programID, "WorldProj");
  glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(Engine::managers_.GetManager<CameraManager*>()->WorldProj));
  CHECKERROR;

  Engine::managers_.GetManager<InverseKinematicManager*>()->DrawIKChain(IK_Program);

  CHECKERROR;
  IK_Program->UnUse();
  lightFbo_.Unbind();
}

void RenderManager::SpringPass()
{
  glViewport(0, 0, width, height);
  CHECKERROR;
  glEnable(GL_DEPTH_TEST);
  CHECKERROR;
  lightFbo_.Bind();
  Spring_Program->Use();

  // depth copy
  CHECKERROR;
  glBlitNamedFramebuffer(gbuffer_.GetGBufferID(), lightFbo_.fboID, 0, 0, width, height, 0, 0, width, height, GL_DEPTH_BUFFER_BIT, GL_NEAREST);
  CHECKERROR;

  int loc = glGetUniformLocation(Spring_Program->programID, "WorldView");
  glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(Engine::managers_.GetManager<CameraManager*>()->WorldView));
  CHECKERROR;

  loc = glGetUniformLocation(Spring_Program->programID, "WorldProj");
  glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(Engine::managers_.GetManager<CameraManager*>()->WorldProj));
  CHECKERROR;

  Engine::managers_.GetManager<PhysicsManager*>()->Draw(Spring_Program);

  CHECKERROR;
  Spring_Program->UnUse();
  lightFbo_.Unbind();
}

void RenderManager::BeginFrame()
{
  glViewport(0, 0, width, height);

  // blend
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  // depth
  glEnable(GL_DEPTH_TEST);

  glEnable(GL_LINE_SMOOTH);
  CHECKERROR;
  glEnable(GL_POLYGON_SMOOTH);
  CHECKERROR;
  glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
  CHECKERROR;
  glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);
  CHECKERROR;

  // clear screen
  glClearColor(clearColor_.r, clearColor_.g, clearColor_.b, 1);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // msaa
  glEnable(GL_MULTISAMPLE);
}

void RenderManager::EndFrame()
{
  glfwSwapBuffers(Engine::managers_.GetManager<WindowManager*>()->GetHandle());
}

void RenderManager::MRT_Pass()
{
  glViewport(0, 0, width, height);
  CHECKERROR;
  // MRT pass
  gbuffer_.Bind();
  // depth
  glEnable(GL_DEPTH_TEST);
  // clear screen
  glClearColor(clearColor_.r, clearColor_.g, clearColor_.b, 1);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  CHECKERROR;
  MRT_Program->Use();

  int loc = glGetUniformLocation(MRT_Program->programID, "WorldView");
  glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(Engine::managers_.GetManager<CameraManager*>()->WorldView));

  loc = glGetUniformLocation(MRT_Program->programID, "WorldProj");
  glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(Engine::managers_.GetManager<CameraManager*>()->WorldProj));

  loc = glGetUniformLocation(MRT_Program->programID, "WorldInverse");
  glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(Engine::managers_.GetManager<CameraManager*>()->WorldInverse));

  // send animation data to shader
  CHECKERROR;
  auto* am = Engine::managers_.GetManager<AnimationManager*>();
  if (am->animator)
  {
    auto transforms = am->animator->GetFinalBoneMatrices();
    for (int i = 0; i < transforms.size(); ++i)
    {
      std::string name = "finalBonesMatrices[" + std::to_string(i) + "]";
      loc = glGetUniformLocation(MRT_Program->programID, name.c_str());
      glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(transforms[i]));
    }
  }


  CHECKERROR;
  Engine::managers_.GetManager<ObjectManager*>()->Draw(MRT_Program);

  CHECKERROR;
  MRT_Program->UnUse();
  CHECKERROR;
  gbuffer_.UnBind();
  CHECKERROR;
  gbuffer_.GenerateMipmap();
}

void RenderManager::ShadowPass()
{
  glViewport(0, 0, width, height);
  CHECKERROR;
  shadowFBO_.Bind();
  glEnable(GL_DEPTH_TEST);
  glClear(GL_DEPTH_BUFFER_BIT);

  Shadow_Program->Use();

  sun.SunProj = glm::ortho(-5000.0f, 5000.0f, -5000.0f, 5000.0f, 1.f, 20000.0f);
  glm::vec3& lightPos = Engine::managers_.GetManager<ObjectManager*>()->sun->GetPosition();
  glm::vec3 center_scene = { 0.f,0.f,0.f };
  sun.SunDir = glm::normalize(center_scene - lightPos);
  glm::vec3 relativeUp = { 0.f,1.f,0.f };
  glm::vec3 rightVector = glm::cross(relativeUp, -sun.SunDir);
  glm::vec3 upVector = glm::cross(-sun.SunDir, rightVector);
  sun.SunView = LookAt(lightPos, sun.SunDir, upVector);

  int loc = glGetUniformLocation(Shadow_Program->programID, "ViewMatrix");
  glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(sun.SunView));

  loc = glGetUniformLocation(Shadow_Program->programID, "ProjectionMatrix");
  glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(sun.SunProj));

  // send animation data to shader
  CHECKERROR;
  auto* am = Engine::managers_.GetManager<AnimationManager*>();
  if (am->animator)
  {
    auto transforms = am->animator->GetFinalBoneMatrices();
    for (int i = 0; i < transforms.size(); ++i)
    {
      std::string name = "finalBonesMatrices[" + std::to_string(i) + "]";
      loc = glGetUniformLocation(Shadow_Program->programID, name.c_str());
      glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(transforms[i]));
    }
  }

  CHECKERROR;
  Engine::managers_.GetManager<ObjectManager*>()->Draw(Shadow_Program);
  CHECKERROR;

  Shadow_Program->UnUse();
  shadowFBO_.UnBind();
  CHECKERROR;
}

void RenderManager::LightingPass()
{
  glViewport(0, 0, width, height);
  // lighting pass
  glDisable(GL_DEPTH_TEST);
  lightFbo_.Bind();
  // clear screen
  glClearColor(clearColor_.r, clearColor_.g, clearColor_.b, 1);
  glClear(GL_COLOR_BUFFER_BIT);
  CHECKERROR;
  BRDF_Program->Use();
  CHECKERROR;

  int loc = glGetUniformLocation(BRDF_Program->programID, "WorldView");
  glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(Engine::managers_.GetManager<CameraManager*>()->WorldView));
  CHECKERROR;

  loc = glGetUniformLocation(BRDF_Program->programID, "WorldInverse");
  glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(Engine::managers_.GetManager<CameraManager*>()->WorldInverse));
  CHECKERROR;

  loc = glGetUniformLocation(BRDF_Program->programID, "lightPos");
  glUniform3fv(loc, 1, glm::value_ptr(Engine::managers_.GetManager<ObjectManager*>()->sun->GetPosition()));
  CHECKERROR;

  glm::vec3 finalSunColor = sun.SunColor * sun.SunIntensity;
  loc = glGetUniformLocation(BRDF_Program->programID, "Light");
  glUniform3fv(loc, 1, glm::value_ptr(finalSunColor));
  CHECKERROR;

  glm::vec3 ambient = { ambientLight,ambientLight,ambientLight };
  loc = glGetUniformLocation(BRDF_Program->programID, "Ambient");
  glUniform3fv(loc, 1, glm::value_ptr(ambient));
  CHECKERROR;

  glBindTextureUnit(gbuffer_.GetTextureUnit(Layout::Position), gbuffer_.GetTextureID(Layout::Position));
  loc = glGetUniformLocation(BRDF_Program->programID, "gPosition");
  glUniform1i(loc, static_cast<int>(gbuffer_.GetTextureUnit(Layout::Position)));
  CHECKERROR;

  glBindTextureUnit(gbuffer_.GetTextureUnit(Layout::Albedo), gbuffer_.GetTextureID(Layout::Albedo));
  loc = glGetUniformLocation(BRDF_Program->programID, "gAlbedo");
  glUniform1i(loc, static_cast<int>(gbuffer_.GetTextureUnit(Layout::Albedo)));
  CHECKERROR;

  glBindTextureUnit(gbuffer_.GetTextureUnit(Layout::Normal), gbuffer_.GetTextureID(Layout::Normal));
  loc = glGetUniformLocation(BRDF_Program->programID, "gNormal");
  glUniform1i(loc, static_cast<int>(gbuffer_.GetTextureUnit(Layout::Normal)));
  CHECKERROR;

  glBindTextureUnit(gbuffer_.GetTextureUnit(Layout::Specular), gbuffer_.GetTextureID(Layout::Specular));
  loc = glGetUniformLocation(BRDF_Program->programID, "gSpecular");
  glUniform1i(loc, static_cast<int>(gbuffer_.GetTextureUnit(Layout::Specular)));
  CHECKERROR;

  loc = glGetUniformLocation(BRDF_Program->programID, "shininess");
  float Shininess = 50.f;
  glUniform1f(loc, Shininess);
  CHECKERROR;

  loc = glGetUniformLocation(BRDF_Program->programID, "resolution");
  glUniform2fv(loc, 1, glm::value_ptr(Engine::managers_.GetManager<WindowManager*>()->resolution_));

  loc = glGetUniformLocation(BRDF_Program->programID, "gammaCorrection");
  glUniform1i(loc, static_cast<int>(gammaCorrection));

  loc = glGetUniformLocation(BRDF_Program->programID, "castShadow");
  glUniform1i(loc, static_cast<int>(castShadow));

  loc = glGetUniformLocation(BRDF_Program->programID, "PCF");
  glUniform1i(loc, static_cast<int>(PCF));

  loc = glGetUniformLocation(BRDF_Program->programID, "bias");
  glUniform1f(loc, shadowBias);

  // light space (shadow mapping)
  loc = glGetUniformLocation(BRDF_Program->programID, "lightSpaceMatrix");
  glm::mat4 lightSpaceMatrix = sun.SunProj * sun.SunView;
  glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(lightSpaceMatrix));

  glBindTextureUnit(shadowFBO_.textureUnit, shadowFBO_.textureID);
  loc = glGetUniformLocation(BRDF_Program->programID, "shadowMap");
  glUniform1i(loc, static_cast<int>(shadowFBO_.textureUnit));

  glBindVertexArray(emptyVAOid_);
  CHECKERROR;
  glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
  CHECKERROR;

  glBindVertexArray(0);

  BRDF_Program->UnUse();
  CHECKERROR;

  lightFbo_.Unbind();
  CHECKERROR;
  lightFbo_.generateMipMap();
}


