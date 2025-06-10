#ifndef MITE_SCENE_CORE
#define MITE_SCENE_CORE

#include "headers/headers.h"

namespace mite {
using Entity = uint32_t;

// 基础组件
struct TransformComponent {
  glm::vec3 position;
  glm::quat rotation;
  glm::vec3 scale;

  glm::mat4 getMatrix() const
  { /*...*/
  }
};

//struct MeshComponent {
//  std::shared_ptr<MeshData> mesh;
//  bool visible = true;
//};
//
//struct MaterialComponent {
//  std::shared_ptr<MaterialData> material;
//};

struct LightComponent {
  enum Type { Point, Directional, Spot };
  Type type;
  glm::vec3 color;
  float intensity;
  // 其他光照参数...
};

// 场景类
class Scene {
 public:

  void UpDate();
  void LoadDefaultScene();

  Entity createEntity();
  void destroyEntity(Entity entity);

  template<typename T> void addComponent(Entity entity, T component);

  template<typename T> T &getComponent(Entity entity);

  template<typename T> bool hasComponent(Entity entity) const;

  template<typename T> void removeComponent(Entity entity);

  // 获取所有具有特定组件的实体
  template<typename... Components> std::vector<Entity> getEntitiesWith() const;

 private:
  std::unordered_map<Entity, std::unordered_map<size_t, std::any>> m_components;
  Entity m_nextEntity = 0;
};

}  // namespace mite

#endif