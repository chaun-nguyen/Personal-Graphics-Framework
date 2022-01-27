#include "ObjectManager.h"
#include "Box.h"
#include "Sphere.h"
#include "Texture.h"
#include "Engine.h"
#include "Shader.h"
#include "Plane.h"
#include "Model.h"
#include "Transform.h"
#include "Octree.h"
#include "BspTree.h"
#include "GJK.h"
#include "Physics.h"
#include <iostream>

ObjectManager::~ObjectManager()
{
  for (auto& p : SpringMassDamperGeometry_)
  {
    if (p->shape)
      delete p->shape;
    if (p->diffuseTex)
      delete p->diffuseTex;
    if (p->normalTex)
      delete p->normalTex;
    delete p;
  }

  for (auto& obj : container_)
  {
    if (obj->shape)
      delete obj->shape;
    if (obj->diffuseTex)
      delete obj->diffuseTex;
    if (obj->normalTex)
      delete obj->normalTex;
    delete obj;
  }
  for (auto& Objmodel : models_)
  {
    if (Objmodel)
    {
      if (Objmodel->model)
      {
        delete Objmodel->model;
      }
      delete Objmodel;
    }
  }
}

void ObjectManager::CreateSpringMassDamperSystem()
{
  // Create Spring Mass Damper System
  float height = 3000.f;
  float mass = 10.f;
  float offset = 20.f;

  Shape* anchorPolygonA = new Sphere(2);
  Object* anchorPointsA = new Object(anchorPolygonA,
    { 0.f,1.f,0.f }, { 0.f,0.f,0.f }, 0.f);
  anchorPointsA->tiling = 1.f;
  anchorPointsA->diffuseTex = new TextureLoader("./textures/Brazilian_rosewood_pxr128.png");
  anchorPointsA->normalTex = new TextureLoader("./textures/Brazilian_rosewood_pxr128_normal.png");
  anchorPointsA->SetPosition({ -4000.0f,height,0.0f });
  //anchorPointsA->SetRotation(0.f);
  anchorPointsA->SetScale({ 100.f,100.f,100.f });
  anchorPointsA->BuildModelMatrix();

  SpringMassDamperGeometry_.push_back(anchorPointsA);

  Shape* stickShape1 = new Box();
  Object* stick1 = new Object(stickShape1,
    { 0.f,1.f,0.f }, { 0.f,0.f,0.f }, 0.f);
  stick1->tiling = 1.f;
  stick1->diffuseTex = new TextureLoader("./textures/space-crate1-albedo.png");
  stick1->normalTex = new TextureLoader("./textures/space-crate1-normal-ogl.png");
  stick1->SetPosition({ -3000.0f,height,0.0f });
  //stick1->SetRotation(0.f);
  stick1->SetScale({ 100.f,100.f,50.f });
  stick1->BuildModelMatrix();

  // populate physics component
  stick1->physics = new Physics();
  stick1->physics->Setup(stick1, stickShape1, mass / stickShape1->Pnt.size(), stick1->GetScale() * offset);

  SpringMassDamperGeometry_.push_back(stick1);

  Shape* stickShape2 = new Box();
  Object* stick2 = new Object(stickShape2,
    { 0.f,1.f,0.f }, { 0.f,0.f,0.f }, 0.f);
  stick2->tiling = 1.f;
  stick2->diffuseTex = new TextureLoader("./textures/space-crate1-albedo.png");
  stick2->normalTex = new TextureLoader("./textures/space-crate1-normal-ogl.png");
  stick2->SetPosition({ -2000.0f,height,0.0f });
  //stick2->SetRotation(0.f);
  stick2->SetScale({ 50.f,200.f,50.f });
  stick2->BuildModelMatrix();

  // populate physics component
  stick2->physics = new Physics();
  stick2->physics->Setup(stick2, stickShape2, mass / stickShape2->Pnt.size(), stick2->GetScale() * offset);

  SpringMassDamperGeometry_.push_back(stick2);

  Shape* stickShape3 = new Box();
  Object* stick3 = new Object(stickShape3,
    { 0.f,1.f,0.f }, { 0.f,0.f,0.f }, 0.f);
  stick3->tiling = 1.f;
  stick3->diffuseTex = new TextureLoader("./textures/space-crate1-albedo.png");
  stick3->normalTex = new TextureLoader("./textures/space-crate1-normal-ogl.png");
  stick3->SetPosition({ -1000.0f,height,0.0f });
  //stick3->SetRotation(0.f);
  stick3->SetScale({ 50.f,200.f,50.f });
  stick3->BuildModelMatrix();

  // populate physics component
  stick3->physics = new Physics();
  stick3->physics->Setup(stick3, stickShape3, mass / stickShape3->Pnt.size(), stick3->GetScale() * offset);

  SpringMassDamperGeometry_.push_back(stick3);

  Shape* stickShape4 = new Box();
  Object* stick4 = new Object(stickShape4,
    { 0.f,1.f,0.f }, { 0.f,0.f,0.f }, 0.f);
  stick4->tiling = 1.f;
  stick4->diffuseTex = new TextureLoader("./textures/space-crate1-albedo.png");
  stick4->normalTex = new TextureLoader("./textures/space-crate1-normal-ogl.png");
  stick4->SetPosition({ 0.0f,height,0.0f });
  //stick4->SetRotation(0.f);
  stick4->SetScale({ 200.f,50.f,50.f });
  stick4->BuildModelMatrix();

  // populate physics component
  stick4->physics = new Physics();
  stick4->physics->Setup(stick4, stickShape4, mass / stickShape4->Pnt.size(), stick4->GetScale() * offset);

  SpringMassDamperGeometry_.push_back(stick4);

  Shape* stickShape5 = new Box();
  Object* stick5 = new Object(stickShape5,
    { 0.f,1.f,0.f }, { 0.f,0.f,0.f }, 0.f);
  stick5->tiling = 1.f;
  stick5->diffuseTex = new TextureLoader("./textures/space-crate1-albedo.png");
  stick5->normalTex = new TextureLoader("./textures/space-crate1-normal-ogl.png");
  stick5->SetPosition({ 1000.0f,height,0.0f });
  //stick5->SetRotation(0.f);
  stick5->SetScale({ 200.f,50.f,50.f });
  stick5->BuildModelMatrix();

  // populate physics component
  stick5->physics = new Physics();
  stick5->physics->Setup(stick5, stickShape5, mass / stickShape5->Pnt.size(), stick5->GetScale() * offset);

  SpringMassDamperGeometry_.push_back(stick5);

  Shape* stickShape6 = new Box();
  Object* stick6 = new Object(stickShape6,
    { 0.f,1.f,0.f }, { 0.f,0.f,0.f }, 0.f);
  stick6->tiling = 1.f;
  stick6->diffuseTex = new TextureLoader("./textures/space-crate1-albedo.png");
  stick6->normalTex = new TextureLoader("./textures/space-crate1-normal-ogl.png");
  stick6->SetPosition({ 2000.0f,height,0.0f });
  stick6->SetScale({ 200.f,50.f,50.f });
  stick6->BuildModelMatrix();

  // populate physics component
  stick6->physics = new Physics();
  stick6->physics->Setup(stick6, stickShape6, mass / stickShape6->Pnt.size(), stick6->GetScale()* offset);

  SpringMassDamperGeometry_.push_back(stick6);

  Shape* stickShape7 = new Box();
  Object* stick7 = new Object(stickShape7,
    { 0.f,1.f,0.f }, { 0.f,0.f,0.f }, 0.f);
  stick7->tiling = 1.f;
  stick7->diffuseTex = new TextureLoader("./textures/space-crate1-albedo.png");
  stick7->normalTex = new TextureLoader("./textures/space-crate1-normal-ogl.png");
  stick7->SetPosition({ 3000.0f,height,0.0f });
  stick7->SetScale({ 100.f,100.f,50.f });
  stick7->BuildModelMatrix();

  // populate physics component
  stick7->physics = new Physics();
  stick7->physics->Setup(stick7, stickShape7, mass / stickShape7->Pnt.size(), stick7->GetScale()* offset);

  SpringMassDamperGeometry_.push_back(stick7);

  Shape* anchorPolygonB = new Sphere(2);
  Object* anchorPointsB = new Object(anchorPolygonB,
    { 0.f,1.f,0.f }, { 0.f,0.f,0.f }, 0.f);
  anchorPointsB->tiling = 1.f;
  anchorPointsB->diffuseTex = new TextureLoader("./textures/Brazilian_rosewood_pxr128.png");
  anchorPointsB->normalTex = new TextureLoader("./textures/Brazilian_rosewood_pxr128_normal.png");
  anchorPointsB->SetPosition({ 4000.0f,height,0.0f });
  //anchorPointsB->SetRotation(0.f);
  anchorPointsB->SetScale({ 100.f,100.f,100.f });
  anchorPointsB->BuildModelMatrix();

  SpringMassDamperGeometry_.push_back(anchorPointsB);
}


void ObjectManager::Setup()
{
  CreateSpringMassDamperSystem();

  // GJK object
  Shape* spherePolygon = new Sphere(32);
  Object* sphere = new Object(spherePolygon,
    spherePolygon->diffuseColor / 2.0f, spherePolygon->specularColor, spherePolygon->shininess);
  
  sphere->diffuseTex = new TextureLoader("./textures/Brazilian_rosewood_pxr128.png");
  sphere->normalTex = new TextureLoader("./textures/Brazilian_rosewood_pxr128_normal.png");
  sphere->SetPosition({ 0.0f,2500.0f,0.0f });
  //sphere->SetRotation(0.f);
  sphere->SetScale({ 25.f,25.f,25.f });
  sphere->BuildModelMatrix();
  
  Add(sphere);

  // create bounding volume
  BoundingVolume* bv = sphere->shape->bbox(); // bbox() actually create new bounding volume, dont call it every frame
  bv->bv_object->SetPosition(bv->center_ * bv->parent->GetScale() + bv->parent->GetPosition());
  bv->bv_object->SetScale(bv->parent->GetScale() * bv->size_); // world space scale * object space scale
  bv->Update(); // update bounding volume center and size in world space, need to be called before BuildModelMatrix()
  bv->bv_object->BuildModelMatrix();
  sphere->bv = bv;
  AddBoundingVolumeGJK(bv);

  // Inverse Kinematic
  // end effector for inverse kinematic
  Shape* boxPolygon = new Box();
  Object* box = new Object(boxPolygon,
    boxPolygon->diffuseColor, boxPolygon->specularColor, boxPolygon->shininess);

  box->diffuseTex = new TextureLoader("./textures/space-crate1-albedo.png");
  box->normalTex = new TextureLoader("./textures/space-crate1-normal-ogl.png");
  box->tiling = 1.f;
  box->SetPosition({ 500.0f,800.0f,500.0f });
  //box->SetRotation(0.f);
  box->SetScale({ 100.f,100.f,100.f });
  box->BuildModelMatrix();
  box->isWireFrame = true;

  //Add(box);
  Engine::managers_.GetManager<InverseKinematicManager*>()->Goal = box;

  // plane
  Shape* planePolygon = new Plane(6000.0f, 10);
  Object* plane = new Object(planePolygon,
    floorColor, planePolygon->specularColor, planePolygon->shininess);

  plane->diffuseTex = new TextureLoader("./textures/6670-diffuse.jpg");
  plane->normalTex = new TextureLoader("./textures/6670-normal.jpg");
  plane->SetPosition({ 0.0f, 0.0f, 0.0f });
  //plane->SetRotation(0.f);
  plane->SetScale({ 2.f,2.f,2.f });
  plane->BuildModelMatrix();

  Add(plane);

  //BoundingVolume* bv1 = plane->shape->bbox();
  //bv1->bv_object->SetPosition(bv1->parent->GetPosition());
  //bv1->bv_object->SetScale(bv1->parent->GetScale() * bv1->size_); // world space scale * object space scale
  //bv1->bv_object->BuildModelMatrix();
  //AddBoundingVolume(bv1);

  Shape* lightPolygon = new Sphere(32);
  sun = new Object(lightPolygon,
    { 1.0f,1.0f,1.0f }, lightPolygon->specularColor, lightPolygon->shininess);

  sun->SetPosition({ 100.0f,7500.0f,100.0f });
  //sun->SetRotation(0.f);
  sun->SetScale({ 1.f,1.f,1.f });
  sun->BuildModelMatrix();

  //Add(sun);

  //gjk
  gjkController.simplex = new Simplex();

  renderModel = false;
}

void ObjectManager::Update()
{
  // update final transform for spring mass damper system
  for (auto& p : SpringMassDamperGeometry_)
  {
    if (p)
      p->BuildModelMatrix();
  }

  // update procedural shape (cube, sphere, plane)
  for (auto& obj : container_)
  {
    if (obj)
      obj->BuildModelMatrix();
  }

  // update actual loaded model
  for (auto& model : models_)
  {
    if (model)
    {
      model->BuildModelMatrix();
    }
  }
  // update bounding volume of octree and movable objects
  if (octreeController.treeReady)
  {
    octreeController.Update(&octreeController.tree->root_);
  }
  // gjk only update sphere (movable object)
  bvs_gjk_[0]->bv_object->SetPosition(bvs_gjk_[0]->parent->GetPosition());
  bvs_gjk_[0]->Update(); // update bounding volume center and size in world space, need to be called before BuildModelMatrix()
  bvs_gjk_[0]->bv_object->BuildModelMatrix();

  // octree
  if (octreeController.buildFlag)
  {
    octreeController.start_bv = new BV_AABB(total_model_vertices_, models_[0], { 1.0f,0.0f,0.0f });
    // need to add relative transform and multiply relative scale
    octreeController.start_bv->bv_object->SetPosition(octreeController.start_bv->center_ * octreeController.start_bv->parent->GetScale() + octreeController.start_bv->parent->GetPosition());
    // world space scale * object space scale
    octreeController.start_bv->bv_object->SetScale(octreeController.start_bv->parent->GetScale() * octreeController.start_bv->size_);
    
    octreeController.tree = new Octree(total_model_vertices_, octreeController.start_bv, octreeController.max_triangles);
    octreeController.buildFlag = false;
    octreeController.treeEmpty = false;
    octreeController.treeReady = true;
  }
  else if (octreeController.deleteFlag)
  {
    octreeController.tree->Destroy(&octreeController.tree->root_);
    octreeController.deleteFlag = false;
    octreeController.treeReady = false;
    octreeController.treeEmpty = true;
  }
  // bsp tree
  if (bsptreeConroller.buildFlag)
  {
    bsptreeConroller.tree = new BspTree(total_model_indices_, total_model_vertices_, bsptreeConroller.max_triangles);
    bsptreeConroller.buildFlag = false;
    bsptreeConroller.treeEmpty = false;
    bsptreeConroller.treeReady = true;
  }
  else if (bsptreeConroller.deleteFlag)
  {
    bsptreeConroller.tree->Destroy(&bsptreeConroller.tree->root_);
    bsptreeConroller.tree->ClearLeafNodes();
    bsptreeConroller.deleteFlag = false;
    bsptreeConroller.treeReady = false;
    bsptreeConroller.treeEmpty = true;
  }
  // gjk
  if (gjkController.startFlag && !gjkController.stopFlag)
  {
    gjkController.updateSpherePos = false;
    // move sphere down
    float speed = 400.0f;
    float dt = Engine::managers_.GetManager<FrameRateManager*>()->delta_time;
    glm::vec3 pos = container_[0]->GetPosition();
    pos += dt * speed * gjkController.dir;
    container_[0]->SetPosition(pos);

    if (octreeController.tree->root_)
    {
      if (GJK::DetectCollision_BroadPhase(container_[0], octreeController.tree->root_))
      {
        gjkController.stopFlag = true;
        Engine::managers_.GetManager<RenderManager*>()->simplexDraw = true;

        if (!gjkController.simplex->vaoFlag_)
          gjkController.simplex->CreateVAOs();
      }
    }
  }
  if (gjkController.resetFlag)
  {
    bvs_gjk_.pop_back();
    gjkController.simplex->vertices_.clear();
    gjkController.simplex->indices_.clear();
    gjkController.simplex->vaoFlag_ = false;
    gjkController.startFlag = false;
    gjkController.stopFlag = false;
    gjkController.resetFlag = false;
    gjkController.updateSpherePos = true;
    Engine::managers_.GetManager<RenderManager*>()->simplexDraw = false;
  }
  if (gjkController.updateSpherePos)
  {
    auto* cm = Engine::managers_.GetManager<CameraManager*>();
    glm::vec4 eyePos = cm->WorldInverse * glm::vec4(0.f, 0.f, 0.f, 1.0f); // get camera position in world space
    container_[0]->SetPosition(glm::vec3(eyePos) + (cm->backVector * container_[0]->GetScale() * 10.0f));
    gjkController.dir = cm->lookAtVector;
  }
}

void ObjectManager::Add(Object* newObj)
{
  container_.push_back(newObj);
}

void ObjectManager::AddModel(Object* newModel)
{
  models_.push_back(newModel);
}

std::vector<Object*>& ObjectManager::GetModels()
{
  return models_;
}

void ObjectManager::AddBoundingVolumeGJK(BoundingVolume* bv)
{
  bvs_gjk_.push_back(bv);
}

void ObjectManager::Draw(ShaderProgram* shaderProgram)
{
  for (auto& obj : SpringMassDamperGeometry_)
  {
    if (obj)
    {
      int loc = glGetUniformLocation(shaderProgram->programID, "ModelTr");
      glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(obj->modelTr));

      loc = glGetUniformLocation(shaderProgram->programID, "NormalTr");
      glm::mat4 normalTr = glm::transpose(glm::inverse(obj->modelTr));
      glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(normalTr));

      loc = glGetUniformLocation(shaderProgram->programID, "isModel");
      glUniform1i(loc, 0);

      loc = glGetUniformLocation(shaderProgram->programID, "isTextureSupported");
      glUniform1i(loc, obj->isTextureSupported);

      loc = glGetUniformLocation(shaderProgram->programID, "tiling");
      glUniform1f(loc, obj->tiling);

      if (obj->isTextureSupported)
      {
        obj->diffuseTex->Bind(9, shaderProgram->programID, "texture_diffuse1");
        obj->diffuseTex->Unbind();

        obj->normalTex->Bind(10, shaderProgram->programID, "texture_normal1");
        obj->normalTex->Unbind();
      }
      else
      {
        loc = glGetUniformLocation(shaderProgram->programID, "diffuseColor");
        glUniform3fv(loc, 1, glm::value_ptr(obj->diffuseColor));
      }

      obj->Draw();
    }
  }

  for (auto& obj : container_)
  {
    if (obj)
    {
      int loc = glGetUniformLocation(shaderProgram->programID, "ModelTr");
      glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(obj->modelTr));

      loc = glGetUniformLocation(shaderProgram->programID, "NormalTr");
      glm::mat4 normalTr = glm::transpose(glm::inverse(obj->modelTr));
      glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(normalTr));

      loc = glGetUniformLocation(shaderProgram->programID, "isModel");
      glUniform1i(loc, 0);

      loc = glGetUniformLocation(shaderProgram->programID, "isTextureSupported");
      glUniform1i(loc, obj->isTextureSupported);

      loc = glGetUniformLocation(shaderProgram->programID, "tiling");
      glUniform1f(loc, obj->tiling);

      if (obj->isTextureSupported)
      {
        obj->diffuseTex->Bind(9, shaderProgram->programID, "texture_diffuse1");
        obj->diffuseTex->Unbind();

        obj->normalTex->Bind(10, shaderProgram->programID, "texture_normal1");
        obj->normalTex->Unbind();
      }
      else
      {
        loc = glGetUniformLocation(shaderProgram->programID, "diffuseColor");
        glUniform3fv(loc, 1, glm::value_ptr(obj->diffuseColor));
      }

      obj->Draw();
    }
  }
  if (renderModel)
  {
    for (auto& model : models_)
    {
      if (model)
      {
        if (model->model)
        {
          int loc = glGetUniformLocation(shaderProgram->programID, "ModelTr");
          glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(model->modelTr));

          loc = glGetUniformLocation(shaderProgram->programID, "isModel");
          glUniform1i(loc, 1);

          if (model->model->textures_loaded.empty())
          {
            int loc = glGetUniformLocation(shaderProgram->programID, "isTextureSupported");
            glUniform1i(loc, 0);

            glm::vec3 redColor = { 1.0f,0.0f,0.0f };
            glm::vec3 brassColor = { 0.6666667, 0.662745, 0.678431 };
            glm::vec3 finalColor = gjkController.stopFlag ? redColor : brassColor;
            loc = glGetUniformLocation(shaderProgram->programID, "diffuseColor");
            glUniform3fv(loc, 1, glm::value_ptr(finalColor));
          }
          else
          {
            int loc = glGetUniformLocation(shaderProgram->programID, "isTextureSupported");
            glUniform1i(loc, 1);
          }

          model->model->Draw(shaderProgram);
        }
      }
    }
  }
}

void ObjectManager::DebugDraw(ShaderProgram* shaderProgram, RenderManager::DebugDrawType type)
{
  switch (type)
  {
  case RenderManager::DebugDrawType::VertexNormal:
    for (auto& model : models_)
    {
      if (model)
      {
        if (model->model)
        {
          glm::mat4 identity(1.0f);
          identity = Scale(0.01f, 0.01f, 0.01f);
          int loc = glGetUniformLocation(shaderProgram->programID, "ModelTr");
          glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(identity));

          loc = glGetUniformLocation(shaderProgram->programID, "color");
          glUniform3fv(loc, 1, glm::value_ptr(glm::vec3(0.f, 1.f, 0.f)));

          model->model->DrawVertexNormals();
        }
      }
    }
    break;
  case RenderManager::DebugDrawType::FaceNormal:
    for (auto& model : models_)
    {
      if (model)
      {
        if (model->model)
        {
          glm::mat4 identity(1.0f);
          identity = Scale(0.01f, 0.01f, 0.01f);
          int loc = glGetUniformLocation(shaderProgram->programID, "ModelTr");
          glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(identity));

          loc = glGetUniformLocation(shaderProgram->programID, "color");
          glUniform3fv(loc, 1, glm::value_ptr(glm::vec3(0.f,1.f,0.f)));

          model->model->DrawFaceNormals();
        }
      }
    }
    break;
  case RenderManager::DebugDrawType::BoundingVolume:
    if (octreeController.treeReady)
    {
      octreeController.Draw(shaderProgram, &octreeController.tree->root_);
    }
    break;
  case RenderManager::DebugDrawType::BspTree:
    if (bsptreeConroller.treeReady)
    {
      if (bsptreeConroller.tree)
      {
        glm::mat4 modelTr = Translate(0.f, 0.f, 0.f) * Rotate(1, 0) * Scale(5.f, 5.f, 5.f);

        int loc = glGetUniformLocation(shaderProgram->programID, "ModelTr");
        glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(modelTr));

        bsptreeConroller.tree->Draw(shaderProgram);
      }
    }
    break;
  case RenderManager::DebugDrawType::GJK:
    for (auto& bv : bvs_gjk_)
    {
      if (bv)
      {
        int loc = glGetUniformLocation(shaderProgram->programID, "ModelTr");
        glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(bv->bv_object->modelTr));

        glm::vec3 green = { 0.f,1.0f,0.f };
        loc = glGetUniformLocation(shaderProgram->programID, "color");
        glUniform3fv(loc, 1, glm::value_ptr(green));

        bv->Draw();
      }
    }
    break;
  case RenderManager::DebugDrawType::Simplex:
    // finish creating vao
    if (gjkController.simplex->vaoFlag_)
    {
      glm::vec3 center = octreeController.start_bv->center_;
      glm::mat4 modelTr = Translate(center.x, center.y, center.z) * Scale(2.f, 2.f, 2.f); // identity matrix

      int loc = glGetUniformLocation(shaderProgram->programID, "ModelTr");
      glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(modelTr));

      glm::vec3 green = { 0.f,1.0f,0.f };
      loc = glGetUniformLocation(shaderProgram->programID, "color");
      glUniform3fv(loc, 1, glm::value_ptr(green));

      gjkController.simplex->Draw(shaderProgram);
    }
    break;
  }
}

void ObjectManager::SectionLoader(const char* path)
{
  auto* dm = Engine::managers_.GetManager<DeserializeManager*>();
  auto parts = dm->ReadSectionPath(std::string(path));

  for (int i = 0; i < models_.size(); ++i)
  {
    if (models_[i])
    {
      if (models_[i]->model)
      {
        delete models_[i]->model;
        models_[i]->model = nullptr;
      }
      delete models_[i];
      models_[i] = nullptr;
    }
  }
  models_.clear();
  total_model_indices_.clear();
  total_model_vertices_.clear();

  for (auto p : parts)
  {
    // load model
    Object* testObj = new Object(p.c_str(), true);
    testObj->SetPosition({ 0.f,0.f,0.f });
    //testObj->SetRotation(0.f);
    testObj->SetScale({ 5.f,5.f,5.f });
    testObj->BuildModelMatrix();

    // animation data
    auto* am = Engine::managers_.GetManager<AnimationManager*>();
    if (am->animation && am->animator)
    {
      am->animation.reset();
      am->animator.reset();
    }
    am->animation = std::make_unique<SkeletalAnimation>(p, testObj->model);
    am->animator = std::make_unique<Animator>(am->animation.get());

    // set up VAO for bone draw hierarchically
    am->animation->SetUpVAO();
    auto* ikm = Engine::managers_.GetManager<InverseKinematicManager*>();
    ikm->SetUpVAO();

    auto* sm = Engine::managers_.GetManager<SplineManager*>();
    // add player position as the first control point
    glm::vec3 firstcontrolPts = testObj->GetPosition() / 500.f;
    firstcontrolPts.y = 0.f;
    glm::vec3 lastControlPts = Engine::managers_.GetManager<InverseKinematicManager*>()->Goal->GetPosition() / 500.f;
    lastControlPts.y = 0.f;
    glm::vec3 middlePts = (firstcontrolPts + lastControlPts) / 2.f;
    middlePts.x += 1.f;
    //lastControlPts.x = lastControlPts.x > 0.f ? lastControlPts.x - 0.5f : lastControlPts.x + 0.5f;
    //lastControlPts.z = lastControlPts.z > 0.f ? lastControlPts.z - 0.5f : lastControlPts.z + 0.5f;

    Spline path;
    path.Add(firstcontrolPts);
    path.Add(middlePts);
    path.Add(lastControlPts);
    path.Construct();
    sm->AddCurve(path);
    ikm->SpaceCurveIndex = sm->GetSize() - 1;

    auto* rm = Engine::managers_.GetManager<RenderManager*>();
    rm->boneDraw = true;
    //rm->IKChainDraw = true;

    // create bounding volume
    //testObj->model->CreateBoundingBox();

    for (unsigned i = 0; i < testObj->model->meshes[0].indices.size(); ++i)
    {
      total_model_indices_.push_back(testObj->model->meshes[0].indices[i] + total_model_vertices_.size());
    }

    for (unsigned i = 0; i < testObj->model->meshes[0].Position.size(); ++i)
    {
      total_model_vertices_.push_back(testObj->model->meshes[0].Position[i]);
    }

    AddModel(testObj);
  }
}

// octree controller
// update internal center, min, max, size of the bounding volume aabb
void ObjectManager::OctreeController::Update(Octree::TreeNode** node)
{
  if (!(*node))
    return;

  (*node)->bv_->Update();
  (*node)->bv_->bv_object->BuildModelMatrix();
  for (int i = 0; i < MAX_CHILDREN; ++i)
  {
    Update(&(*node)->children_[i]);
  }
}

// draw octree recursively
void ObjectManager::OctreeController::Draw(ShaderProgram* shaderProgram, Octree::TreeNode** node)
{
  if (!(*node))
    return;

  int loc = glGetUniformLocation(shaderProgram->programID, "ModelTr");
  glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr((*node)->bv_->bv_object->modelTr));

  loc = glGetUniformLocation(shaderProgram->programID, "color");
  glUniform3fv(loc, 1, glm::value_ptr((*node)->bv_->bv_object->diffuseColor));
  (*node)->bv_->Draw();

  for (int i = 0; i < MAX_CHILDREN; ++i)
  {
    Draw(shaderProgram, &(*node)->children_[i]);
  }
}
