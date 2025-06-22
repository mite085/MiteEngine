#ifndef MITE_SCENE_COMPONENT_SYSTEM
#define MITE_SCENE_COMPONENT_SYSTEM

#include "component.h"
#include "scene_registry.h"

namespace mite {
/**
 * @brief 组件系统基类，管理特定类型组件的更新逻辑
 *
 * 设计目标：
 * 1. 提供统一的组件系统接口
 * 2. 支持自动注册到场景更新循环
 * 3. 高效处理组件批量操作
 * 4. 支持组件依赖和执行顺序控制
 * 5. 提供系统间通信机制
 */
class ComponentSystem {
 public:
  virtual ~ComponentSystem() = default;

  /**
   * @brief 获取系统类型ID(使用辅助宏定义，无需在子类实现)
   */
  virtual std::type_index GetSystemType() const = 0;

  /**
   * @brief 系统执行优先级(越小越先执行)
   */
  virtual Component::Family GetExecutionOrder() const = 0;

  /**
   * @brief 系统初始化（场景加载时调用）
   * @param registry 关联的EnTT registry
   */
  virtual void Initialize(SceneRegistry &registry) = 0;

  /**
   * @brief 系统更新（每帧调用）
   * @param deltaTime 帧间隔时间(秒)
   */
  virtual void Update(float deltaTime, SceneRegistry &registry) = 0;

  /**
   * @brief 系统销毁（场景卸载时调用）
   */
  virtual void Shutdown(SceneRegistry &registry) = 0;

  /**
   * @brief 获取该系统管理的组件类型列表
   */
  virtual std::vector<std::type_index> GetComponentTypes() const = 0;

  /**
   * @brief 获取该系统依赖的其他系统类型
   */
  virtual std::vector<std::type_index> GetSystemDependencies() const = 0;

  /**
   * @brief 当组件被添加时的回调
   * @param entity 实体
   * @param component 组件
   */
  virtual void OnComponentAdded(Entity entity, Component &component) = 0;

  /**
   * @brief 当组件被替换时的回调
   * @param entity 实体
   * @param component 组件
   *
   * 注意：
   * 仅当调用registry.replace<T>(entity, ...)
   * 或registry.patch<T>(entity, ...)，修改现有组件时触发，
   * 故基本无需考虑在子类override该方法。
   */
  virtual void OnComponentUpdated(Entity entity, Component &component){};

  /**
   * @brief 当组件被移除时的回调
   * @param entity 实体
   * @param component 组件
   */
  virtual void OnComponentRemoved(Entity entity, Component &component) = 0;

 protected:
  // 保护构造函数，确保只能通过派生类实例化
  ComponentSystem() = default;

  // 禁用拷贝
  ComponentSystem(const ComponentSystem &) = delete;
  ComponentSystem &operator=(const ComponentSystem &) = delete;
};

/**
 * @brief 组件系统辅助宏，简化系统定义
 */
#define DECLARE_COMPONENT_SYSTEM(system_name) \
 public: \
  std::type_index GetSystemType() const override \
  { \
    return typeid(system_name); \
  } \
  static std::type_index GetStaticType() \
  { \
    return typeid(system_name); \
  }

/**
 * @brief 基于Dirty Flag驱动的组件系统模板类
 *
 * 基本实现原理
 * 1. 标记为脏（Dirty）：当组件的状态发生变化时，将 m_Dirty 设为 true
 * 2. 处理脏状态：在适当的时机（如每帧更新时）检查并处理脏状态
 * 3. 清除脏标记：处理完成后将 m_Dirty 设为 false
 */
template<typename T, typename Policy = void> class DirtyComponentSystem : public ComponentSystem {
  // 限制模板T必须继承自Component类型
  static_assert(std::is_base_of<Component, T>::value, "T must inherit from Component");

 public:
  // 暴露组件类型
  using ComponentType = T;

  /**
   * @brief 将组件注册进维护列表
   * @param component 组件指针
   */
  void Register(T *component)
  {
    std::lock_guard<std::mutex> lock(m_Mutex);
    m_AllComponents.push_back(component);
  }

  /**
   * @brief 将组件从维护列表移除
   * @param component 组件指针
   */
  void Unregister(T *component)
  {
    std::lock_guard<std::mutex> lock(m_Mutex);
    m_AllComponents.erase(std::remove(m_AllComponents.begin(), m_AllComponents.end(), component),
                          m_AllComponents.end());
  }

  /**
   * @brief 按照脏标记更新组件(逐帧调用)
   * @param deltaTime 帧与帧时间间隔
   */
  void Update(float deltaTime, SceneRegistry &registry) override
  {
    // 阶段1：收集脏组件
    CollectDirtyComponents();

    // 阶段2：并行处理
    ProcessDirtyComponents(deltaTime, registry);
  }

  /**
   * @brief 获取该系统管理的组件类型列表
   */
  std::vector<std::type_index> GetComponentTypes() const
  {
    return {typeid(T)};
  }

  /**
   * @brief 获取该系统依赖的其他系统类型
   */
  std::vector<std::type_index> GetSystemDependencies() const override
  {
    return {};
  }

  /**
   * @brief 系统执行优先级(越小越先执行)
   */
  virtual Component::Family GetExecutionOrder() const
  {
    return T::family;
  }

  /**
   * @brief 当组件被添加时的回调
   * @param entity 实体
   * @param component 组件
   */
  void OnComponentAdded(Entity entity, Component &component) override
  {
    Register(static_cast<T *>(&component));
  }

  /**
   * @brief 当组件被移除时的回调
   * @param entity 实体
   * @param component 组件
   */
  void OnComponentRemoved(Entity entity, Component &component) override
  {
    Unregister(static_cast<T *>(&component));
  }

 protected:
  /**
   * @brief 获取脏组件列表
   */
  void CollectDirtyComponents()
  {
    m_DirtyComponents.clear();

    // 并行收集优化
    std::vector<T *> localDirtyComponents;
    std::mutex mutex;
    std::for_each(
        std::execution::par, m_AllComponents.begin(), m_AllComponents.end(), [&](T *comp) {
          if (comp->IsDirty()) {
            std::lock_guard<std::mutex> lock(mutex);
            localDirtyComponents.push_back(comp);
          }
        });

    // 合并结果
    m_DirtyComponents.insert(
        m_DirtyComponents.end(), localDirtyComponents.begin(), localDirtyComponents.end());
  }

  /**
   * @brief 处理脏组件，派生类必须重写该方法
   * 
   * 注意：
   * 如果是Self_Dirty的组件，可以直接继承自
   * DirtyComponentSystem<T, SelfDirtyPolicy>，
   * 自动实现并行处理脏组件，无需重写该方法，
   * （详见下文）
   */
  virtual void ProcessDirtyComponents(float deltaTime, SceneRegistry &registry) = 0;

  std::vector<T *> m_AllComponents;
  std::vector<T *> m_DirtyComponents;
  std::mutex m_Mutex;
};

/**
 * @brief 特化自脏策略版本的 组件系统模板类（继承自主模板）.
 * 
 * 注意：
 * 该类型仅适用于Self_Dirty的组件，
 * 即不受Hierarchy层次关系影响，且
 * 不受Dependencies依赖项影响的组件
 */
struct SelfDirtyPolicy {};  // 自脏策略标签
template<typename T>
class DirtyComponentSystem<T, SelfDirtyPolicy> : public DirtyComponentSystem<T, void> {
 protected:
  /**
   * @brief 并行执行脏组件的Update
   */
  void ProcessDirtyComponents(float deltaTime, SceneRegistry &registry) override
  {
    // 并行处理优化
    std::for_each(std::execution::par,
                  m_DirtyComponents.begin(),
                  m_DirtyComponents.end(),
                  [&](T *comp) { static_cast<Component *>(comp)->Update(deltaTime, registry); });
  }
};

};  // namespace mite

#endif
