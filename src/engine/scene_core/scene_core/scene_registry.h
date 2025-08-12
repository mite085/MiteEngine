#ifndef MITE_SCENE_REGISTRY
#define MITE_SCENE_REGISTRY

#include "component.h"
#include "entity.h"
#include "scene_event_callback_adapter.h"

namespace mite {
/**
 * @brief 组件与实体注册表
 *
 * 负责存储已注册的组件与实体，
 * 并提供类型安全与线程安全的实体和组件操作
 */
class SceneRegistry {
 public:
  SceneRegistry();
  ~SceneRegistry();

  SceneEventCallbackAdapter &GetEventCallbackAdapter();

  // 1. 实体管理 ============================================

  /**
   * @brief 创建新实体
   * @param name 实体名称
   * @return 新创建的实体
   */
  Entity CreateEntity(const std::string &name = "");

  /**
   * @brief 销毁实体及其所有组件
   * @param entity 要销毁的实体
   */
  void DestroyEntity(Entity entity);

  /**
   * @brief 检查实体是否有效
   */
  bool IsValid(Entity entity) const;

  /**
   * @brief 清空注册表
   */
  void Clear();

  // 2. 组件操作 ============================================

  /**
   * @brief 添加组件
   */
  template<typename T, typename... Args> T &AddComponent(Entity entity, Args &&...args)
  {
    // 先检查实体有效性（不需要锁）
    if (!IsValid(entity)) {
      throw std::runtime_error("Cannot add component to invalid entity");
    }

    // 如果已有同类型组件，先移除
    if (HasComponent<T>(entity)) {
      RemoveComponent<T>(entity);
    }

    std::unique_lock lock(m_ComponentMutex);

    // 创建组件并设置所有者
    auto component = std::make_shared<T>(std::forward<Args>(args)...);
    component->SetOwnerEntity(entity);

    // 存储组件
    auto &componentMap = m_Components[typeid(T)];
    componentMap[entity] = component;

    // 触发构造事件
    m_EventCallbackAdapter.OnComponentConstructed<T>(entity, *component);

    return *component;
  }

  /**
   * @brief 获取或添加组件
   */
  template<typename T> T &GetOrAddComponent(Entity entity)
  {
    std::shared_lock lock(m_ComponentMutex);

    if (HasComponent<T>(entity)) {
      return GetComponent<T>(entity);
    }

    lock.unlock();
    return AddComponent<T>(entity);
  }

  /**
   * @brief 移除组件
   */
  template<typename T> void RemoveComponent(Entity entity)
  {
    std::unique_lock lock(m_ComponentMutex);

    // 先确保ComponentTypeMap中存在该类型的ComponentMap
    auto it = m_Components.find(typeid(T));
    if (it != m_Components.end()) {
      // 再确保ComponentMap中，使用该entity可查询到Component
      auto componentIt = it->second.find(entity);
      if (componentIt != it->second.end()) {
        // 触发移除事件
        m_EventCallbackAdapter.OnComponentDestroyed<T>(
            entity, *static_cast<T *>(componentIt->second.get()));

        // 最后移除
        it->second.erase(entity);
      }
    }
  }

  /**
   * @brief 检查实体是否拥有组件
   */
  template<typename T> bool HasComponent(Entity entity) const
  {
    std::shared_lock lock(m_ComponentMutex);

    auto it = m_Components.find(typeid(T));
    if (it != m_Components.end()) {
      return it->second.find(entity) != it->second.end();
    }
    return false;
  }

  /**
   * @brief 检查实体是否同时拥有所有指定组件
   * @tparam Components 要检查的组件类型列表
   * @param entity 要检查的实体
   * @return 如果实体拥有所有指定组件返回true，否则false
   */
  template<typename... Components> bool HasComponentWithAllOf(Entity entity) const
  {
    static_assert(sizeof...(Components) > 0, "At least one component type must be specified");

    std::shared_lock lock(m_ComponentMutex);

    // 检查实体有效性
    if (!IsValid(entity)) {
      return false;
    }

    // 使用折叠表达式检查所有组件
    bool hasAll = true;
    ((hasAll = hasAll && (m_Components.find(typeid(Components)) != m_Components.end() &&
                          m_Components.at(typeid(Components)).find(entity) !=
                              m_Components.at(typeid(Components)).end())),
     ...);

    return hasAll;
  }

  /**
   * @brief 获取组件
   */
  template<typename T> T &GetComponent(Entity entity)
  {
    std::shared_lock lock(m_ComponentMutex);

    auto it = m_Components.find(typeid(T));
    if (it != m_Components.end()) {
      auto componentIt = it->second.find(entity);
      if (componentIt != it->second.end()) {
        return *static_cast<T *>(componentIt->second.get());
      }
    }
    LOG_CRITICAL("Component not found");
    throw std::runtime_error("Component not found");
  }
  /**
   * @brief 获取组件 const版本
   */
  template<typename T> T &GetComponent(Entity entity) const
  {
    std::shared_lock lock(m_ComponentMutex);

    auto it = m_Components.find(typeid(T));
    if (it != m_Components.end()) {
      auto componentIt = it->second.find(entity);
      if (componentIt != it->second.end()) {
        return *static_cast<T *>(componentIt->second.get());
      }
    }
    LOG_CRITICAL("Component not found");
    throw std::runtime_error("Component not found");
  }

  /**
   * @brief 尝试获取组件
   */
  template<typename T> T *TryGetComponent(Entity entity)
  {
    std::shared_lock lock(m_ComponentMutex);

    auto it = m_Components.find(typeid(T));
    if (it != m_Components.end()) {
      auto componentIt = it->second.find(entity);
      if (componentIt != it->second.end()) {
        return static_cast<T *>(componentIt->second.get());
      }
    }
    return nullptr;
  }
  /**
   * @brief 尝试获取组件 const版本
   */
  template<typename T> T *TryGetComponent(Entity entity) const
  {
    std::shared_lock lock(m_ComponentMutex);

    auto it = m_Components.find(typeid(T));
    if (it != m_Components.end()) {
      auto componentIt = it->second.find(entity);
      if (componentIt != it->second.end()) {
        return static_cast<T *>(componentIt->second.get());
      }
    }
    return nullptr;
  }

  // 3. 查询操作 ============================================

  /**
   * @brief 获取所有实体
   */
  std::vector<Entity> GetAllEntities();

  /**
   * @brief 获取拥有指定组件的所有实体
   */
  template<typename T> std::vector<Entity> GetEntitiesWith()
  {
    std::shared_lock lock(m_ComponentMutex);

    std::vector<Entity> entities;
    auto it = m_Components.find(typeid(T));
    if (it != m_Components.end()) {
      for (const auto &pair : it->second) {
        if (IsValid(pair.first)) {
          entities.push_back(pair.first);
        }
      }
    }
    return entities;
  }
  /**
   * @brief 获取拥有所有指定组件的实体
   * @tparam Components 要查询的组件类型列表
   * @return 拥有所有指定组件的实体列表
   */
  template<typename... Components> std::vector<Entity> GetEntitiesWithAllOf()
  {
    static_assert(sizeof...(Components) > 0, "At least one component type must be specified");

    std::shared_lock lock(m_ComponentMutex);

    // 如果没有实体，直接返回空列表
    if (m_Components.empty()) {
      return {};
    }

    // 获取第一个组件类型的实体列表作为基准
    const std::type_index firstType = typeid(
        typename std::tuple_element<0, std::tuple<Components...>>::type);
    auto firstIt = m_Components.find(firstType);
    if (firstIt == m_Components.end()) {
      return {};
    }

    std::vector<Entity> result;

    // 预分配空间
    result.reserve(firstIt->second.size());

    // 遍历第一个组件类型的实体列表，
    // 检查每个实体是否拥有所有指定组件
    for (const auto &pair : firstIt->second) {
      Entity entity = pair.first;
      if (!IsValid(entity)) {
        continue;
      }

      // 检查是否拥有所有组件
      bool hasAllComponents = true;
      ((hasAllComponents = hasAllComponents && HasComponent<Components>(entity)), ...);

      if (hasAllComponents) {
        result.push_back(entity);
      }
    }

    return result;
  }
 private:
  // 组件存储结构
  using ComponentMap = std::unordered_map<Entity, std::shared_ptr<Component>>;
  using ComponentTypeMap = std::unordered_map<std::type_index, ComponentMap>;

  mutable std::shared_mutex m_ComponentMutex;  // 组件操作的读写锁
  ComponentTypeMap m_Components;               // 组件存储

  SceneEventCallbackAdapter m_EventCallbackAdapter;
};
};  // namespace mite

#endif
