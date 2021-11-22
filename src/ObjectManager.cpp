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

#include <iostream>

ObjectManager::~ObjectManager()
{
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

void ObjectManager::Setup()
{
  //
  //Shape* boxPolygon2 = new Box();
  //Object* box2 = new Object(boxPolygon2,
  //  boxPolygon2->diffuseColor + glm::vec3(1,0.04,0.2), boxPolygon2->specularColor, boxPolygon2->shininess);
  //
  //box2->diffuseTex = new TextureLoader("./textures/Brazilian_rosewood_pxr128.png");
  //box2->normalTex = new TextureLoader("./textures/Brazilian_rosewood_pxr128_normal.png");
  //box2->SetPosition({ 10.0f,5.0f,-5.0f });
  //box2->SetRotation(0.f);
  //box2->SetScale({ 2.f,2.f,2.f });
  //box2->BuildModelMatrix();
  //
  //Add(box2);
  //
  Shape* spherePolygon = new Sphere(32);
  Object* sphere = new Object(spherePolygon,
    spherePolygon->diffuseColor / 2.0f, spherePolygon->specularColor, spherePolygon->shininess);
  
  sphere->diffuseTex = new TextureLoader("./textures/Brazilian_rosewood_pxr128.png");
  sphere->normalTex = new TextureLoader("./textures/Brazilian_rosewood_pxr128_normal.png");
  sphere->SetPosition({ 0.0f,2500.0f,0.0f });
  sphere->SetRotation(0.f);
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

  // end effector for inverse kinematic
  Shape* boxPolygon = new Box();
  Object* box = new Object(boxPolygon,
    boxPolygon->diffuseColor, boxPolygon->specularColor, boxPolygon->shininess);

  box->diffuseTex = new TextureLoader("./textures/space-crate1-albedo.png");
  box->normalTex = new TextureLoader("./textures/space-crate1-normal-ogl.png");
  box->tiling = 1.f;
  box->SetPosition({ 500.0f,800.0f,500.0f });
  box->SetRotation(0.f);
  box->SetScale({ 100.f,100.f,100.f });
  box->BuildModelMatrix();

  Add(box);
  Engine::managers_.GetManager<InverseKinematicManager*>()->Goal = box;

  // plane
  Shape* planePolygon = new Plane(6000.0f, 10);
  Object* plane = new Object(planePolygon,
    floorColor, planePolygon->specularColor, planePolygon->shininess);

  plane->diffuseTex = new TextureLoader("./textures/6670-diffuse.jpg");
  plane->normalTex = new TextureLoader("./textures/6670-normal.jpg");
  plane->SetPosition({ 0.0f, 0.0f, 0.0f });
  plane->SetRotation(0.f);
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

  sun->SetPosition({ 3000.0f,3500.0f,100.0f });
  sun->SetRotation(0.f);
  sun->SetScale({ 1.f,1.f,1.f });
  sun->BuildModelMatrix();

  //Add(sun);

  //gjk
  gjkController.simplex = new Simplex();

  //Object* backpackObj = new Object("./model/backpack/backpack.obj", true);
  //backpackObj->SetPosition({ 0.f,15.f,0.f });
  //backpackObj->SetRotation(0.f);
  //backpackObj->SetScale({ 2.f,2.f,2.f });
  //backpackObj->BuildModelMatrix();
  //
  //AddModel(backpackObj);

  //Object* xyzrgb_dragon = new Object("./model/xyzrgb_dragon/xyzrgb_dragon.obj", true);
  //xyzrgb_dragon->SetPosition({ 0.f,5.f,0.f });
  //xyzrgb_dragon->SetRotation(-90.f);
  //xyzrgb_dragon->SetScale({ 0.125f,0.125f,0.125f });
  //xyzrgb_dragon->BuildModelMatrix();
  //
  //AddModel(xyzrgb_dragon);
}

void ObjectManager::Update()
{
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
    testObj->SetRotation(0.f);
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
