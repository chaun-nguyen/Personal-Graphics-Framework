#pragma once
#include "LibHeader.h"
#include "ManagerBase.h"
#include "GBuffer.h"
#include "FBO.h"

class ShaderProgram;

class RenderManager : public ManagerBase<RenderManager>
{
public:
  enum class DebugDrawType : int
  {
    Invalid = -1,
    VertexNormal,
    FaceNormal,
    BoundingVolume,
    BspTree,
    GJK,
    Simplex,

    Total
  };
public:
  RenderManager() = default;
  ~RenderManager() override = default;
  
  void Setup() override;
  void Update() override;

  void BeginFrame();
  void EndFrame();

  void MRT_Pass();
  void ShadowPass();
  void LightingPass();
  void FSQ();
  void DebugPass(DebugDrawType type);
  void SimplexPass(DebugDrawType type);
  void BonePass();
  void SplinePass();
  void IKChainPass();
  void SpringPass();

  ShaderProgram* MRT_Program = nullptr;
  ShaderProgram* BRDF_Program = nullptr;
  ShaderProgram* FSQ_Program = nullptr;
  ShaderProgram* Shadow_Program = nullptr;
  ShaderProgram* Debug_Program = nullptr;
  ShaderProgram* Bone_Program = nullptr;
  ShaderProgram* Spline_Program = nullptr;
  ShaderProgram* IK_Program = nullptr;
  ShaderProgram* Spring_Program = nullptr;

  int width;
  int height;

  GBuffer gbuffer_;
  FBO lightFbo_;
  FBO fsqFbo_;
  FBO simplexFbo_;

  glm::vec3 clearColor_;
  float exposure = 1.0f;
  float shadowBias = 0.003f;
  bool gammaCorrection = true;
  bool castShadow = true;
  bool PCF = true;
  bool debugDraw = false;
  bool boneDraw = false;
  bool splineDraw = true;
  bool IKChainDraw = false;
  bool simplexDraw = false; // gjk
  int debugDrawType = to_integral(DebugDrawType::Invalid);
  bool depthCopy = true;

  GLuint emptyVAOid_;

  struct Sun
  {
    glm::mat4 SunView;
    glm::mat4 SunProj;
    glm::vec3 SunDir;
    glm::vec3 SunColor;
    float SunIntensity;
  };
  Sun sun;
  float ambientLight;

  struct DepthFBO
  {
    void SetUp(int w, int h, unsigned int texUnit)
    {
      width = w;
      height = h;
      textureUnit = texUnit;

      glCreateFramebuffers(1, &FboID);
      CHECKERROR;
      glCreateTextures(GL_TEXTURE_2D, 1, &textureID);
      glBindTextureUnit(textureUnit, textureID);
      glBindTexture(GL_TEXTURE_2D, textureID);

      CHECKERROR;
      glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT,
        width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);

      CHECKERROR;
      glTextureParameteri(textureID, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
      glTextureParameteri(textureID, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
      glTextureParameteri(textureID, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
      glTextureParameteri(textureID, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
      float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
      glTextureParameterfv(textureID, GL_TEXTURE_BORDER_COLOR, borderColor);

      CHECKERROR;
      glBindTexture(GL_TEXTURE_2D, 0);

      glBindFramebuffer(GL_FRAMEBUFFER, FboID);
      CHECKERROR;
      glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, textureID, 0);
      glDrawBuffer(GL_NONE);
      glReadBuffer(GL_NONE);
      glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }
    void Bind()
    {
      glBindFramebuffer(GL_FRAMEBUFFER, FboID);
    }
    void UnBind()
    {
      glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }
    GLuint FboID, textureID, textureUnit;
    int width, height;
  };

  DepthFBO shadowFBO_;
};
