#ifndef MITE_SCENE
#define MITE_SCENE

#include "headers/headers.h"

namespace mite {
using Entity = uint32_t;

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