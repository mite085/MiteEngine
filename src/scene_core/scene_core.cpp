#include "scene_core.h"

namespace mite {

void Scene::UpDate() {}
void Scene::LoadDefaultScene() {}
Entity Scene::createEntity()
{
  return Entity();
}
void Scene::destroyEntity(Entity entity) {}

template<typename T> inline void Scene::addComponent(Entity entity, T component) {}
template<typename T> inline T &Scene::getComponent(Entity entity)
{
  // TODO: 在此处插入 return 语句
}
template<typename T> inline bool Scene::hasComponent(Entity entity) const
{
  return false;
}
template<typename T> inline void Scene::removeComponent(Entity entity) {}
template<typename... Components> inline std::vector<Entity> Scene::getEntitiesWith() const
{
  return std::vector<Entity>();
}
}  // namespace mite