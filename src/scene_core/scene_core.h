#ifndef MITE_SCENE_CORE
#define MITE_SCENE_CORE

#include "headers/headers.h"

namespace mite {
using Entity = uint32_t;

// �������
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
  // �������ղ���...
};

// ������
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

  // ��ȡ���о����ض������ʵ��
  template<typename... Components> std::vector<Entity> getEntitiesWith() const;

 private:
  std::unordered_map<Entity, std::unordered_map<size_t, std::any>> m_components;
  Entity m_nextEntity = 0;
};

}  // namespace mite

#endif