#pragma once
#include "ManagerBase.h"
#include <vector>
#include "Object.h"
#include "BoundingVolume.h"
#include "RenderManager.h"
#include "Octree.h"

class Simplex;
class BspTree;
class ShaderProgram;

class ObjectManager : public ManagerBase<ObjectManager>
{
public:
  struct OctreeController
  {
    void Update(Octree::TreeNode** node);
    void Draw(ShaderProgram* shaderProgram, Octree::TreeNode** node);
    BoundingVolume* start_bv = nullptr;
    Octree* tree = nullptr;
    int max_triangles = 250;
    bool buildFlag = false;
    bool deleteFlag = false;
    bool treeReady = false;
    bool treeEmpty = true;
  };
  struct BspTreeController
  {
    BspTree* tree = nullptr;
    int max_triangles = 500;
    bool buildFlag = false;
    bool deleteFlag = false;
    bool treeReady = false;
    bool treeEmpty = true;
  };
  struct GJK_Controller
  {
    Simplex* simplex = nullptr;
    glm::vec3 dir;
    bool startFlag = false;
    bool stopFlag = false;
    bool resetFlag = false;
    bool updateSpherePos = true;
  };
public:
  ObjectManager() = default;
  ~ObjectManager() override;

  void Setup() override;
  void Update() override;

  void Add(Object* newObj);
  void AddModel(Object* newModel);
  std::vector<Object*>& GetModels();
  void AddBoundingVolumeGJK(BoundingVolume* bv);
  void Draw(ShaderProgram* shaderProgram);
  void DebugDraw(ShaderProgram* shaderProgram, RenderManager::DebugDrawType type);

  Object* sun = nullptr;

  glm::vec3 woodColor = { 87.0 / 255.0, 51.0 / 255.0, 35.0 / 255.0 };
  glm::vec3 brickColor = { 134.0 / 255.0, 60.0 / 255.0, 56.0 / 255.0 };
  glm::vec3 floorColor = { 6 * 16 / 255.0, 5.5 * 16 / 255.0, 3 * 16 / 255.0 };
  glm::vec3 brassColor = { 0.5, 0.5, 0.1 };
  glm::vec3 grassColor = { 62.0 / 255.0, 102.0 / 255.0, 38.0 / 255.0 };
  glm::vec3 waterColor = { 0.3, 0.3, 1.0 };

  void SectionLoader(const char* path);

  OctreeController octreeController;
  BspTreeController bsptreeConroller;
  GJK_Controller gjkController;

  bool renderModel = true;

  std::vector<glm::vec3> total_model_vertices_; // for calculating bounding volume
  std::vector<unsigned int> total_model_indices_; // for calculating bounding volume
  std::vector<Object*> container_;
private:
  std::vector<Object*> models_;
  std::vector<BoundingVolume*> bvs_gjk_;
};
