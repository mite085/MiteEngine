#ifndef MITE_SCENE_COMPONENT_SYSTEM
#define MITE_SCENE_COMPONENT_SYSTEM

#include "scene_core_event.h"
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
class IComponentSystem {
 public:
  virtual ~IComponentSystem() = default;

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
   */
  virtual void Initialize() = 0;

  /**
   * @brief 系统销毁（场景卸载时调用）
   */
  virtual void Shutdown() = 0;

  /**
   * @brief 获取该系统管理的组件类型列表
   */
  virtual std::vector<std::type_index> GetComponentTypes() const = 0;

  /**
   * @brief 获取该系统依赖的其他系统类型
   */
  virtual std::vector<std::type_index> GetSystemDependencies() const = 0;

 protected:
  // 保护构造函数，确保只能通过派生类实例化
  IComponentSystem() = default;

  // 禁用拷贝
  IComponentSystem(const IComponentSystem &) = delete;
  IComponentSystem &operator=(const IComponentSystem &) = delete;

  // 日志系统
  Logger m_Logger;
  // 订阅事件集合
  SubscriptionGroup m_EventSubscriptions;
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
 * @brief 基础组件系统模板类，负责组件管理和事件处理
 *
 * 职责：
 * 1. 管理组件注册/注销
 * 2. 处理组件添加/移除事件
 * 3. 维护所有组件列表
 * 4. Update方法为空，由子类实现具体更新逻辑
 */
template<typename T> class ComponentSystem : public IComponentSystem {
  // 限制模板T必须继承自Component类型
  static_assert(std::is_base_of<Component, T>::value, "T must inherit from Component");

 public:
  // 暴露组件类型
  using ComponentType = T;

  ComponentSystem() : IComponentSystem()
  {
    m_Logger = mite::LoggerSystem::CreateModuleLogger("Component System: {" + type_name<T>() +
                                                      "}");
    m_Logger->trace("Created base component system: {}", type_name<T>());
  }
  virtual void Initialize() override
  {
    // 订阅组件添加/移除事件
    // Immediate同步模式：
    // 组件添加/移除是ECS核心操作，需要立即响应以确保系统状态一致性，避免异步导致的时序问题
    m_EventSubscriptions.SubscribeImmediate<ComponentAddedEvent<T>>(
        BIND_DISPATCH_FN(OnComponentAdded), EventPriority::High);
    m_EventSubscriptions.SubscribeImmediate<ComponentRemovedEvent<T>>(
        BIND_DISPATCH_FN(OnComponentRemoved), EventPriority::High);
  }
  virtual void Shutdown() override
  {
    m_EventSubscriptions.UnsubscribeAll();
    m_AllComponents.clear();
  }

  std::vector<std::type_index> GetComponentTypes() const override
  {
    return {typeid(T)};
  }
  virtual std::vector<std::type_index> GetSystemDependencies() const override
  {
    return {};
  }
  virtual Component::Family GetExecutionOrder() const override
  {
    return T::family;
  }

  /**
   * @brief 获取所有管理的组件
   */
  const std::vector<T *> &GetAllComponents() const
  {
    return m_AllComponents;
  }
  /**
   * @brief 获取组件数量
   */
  size_t GetComponentCount() const
  {
    return m_AllComponents.size();
  }

 protected:
  /**
   * @brief 处理组件添加事件
   */
  virtual void OnComponentAdded(ComponentAddedEvent<T> &e)
  {
    Register(&e.GetComponent());
    e.SetResult(EventResult::Handled);
  }
  /**
   * @brief 处理组件移除事件
   */
  virtual void OnComponentRemoved(ComponentRemovedEvent<T> &e)
  {
    Unregister(&e.GetComponent());
    e.SetResult(EventResult::Handled);
  }
  /**
   * @brief 注册组件
   */
  void Register(T *component)
  {
    std::lock_guard<std::mutex> lock(m_Mutex);
    m_AllComponents.push_back(component);
  }
  /**
   * @brief 注销组件
   */
  void Unregister(T *component)
  {
    std::lock_guard<std::mutex> lock(m_Mutex);
    m_AllComponents.erase(std::remove(m_AllComponents.begin(), m_AllComponents.end(), component),
                          m_AllComponents.end());
  }
  std::vector<T *> m_AllComponents;
  std::mutex m_Mutex;
};

// 非模板的基类
class DirtyComponentSystemBase : public IComponentSystem {
 public:
  virtual ~DirtyComponentSystemBase() = default;
  virtual void Update(float deltaTime, SceneRegistry &registry) = 0;
  virtual size_t GetDirtyComponentCount() const = 0;
};

/**
 * @brief 基于Dirty Flag驱动的组件系统模板类
 *
 * 作用：
 * 所有具体的Component均应当继承自此类，
 * 具有自脏特性的可以直接继承自
 * DirtyComponentSystem<T, SelfDirtyPolicy>
 *
 * 基本实现原理
 * 1. 标记为脏（Dirty）：当组件的状态发生变化时，将 m_Dirty 设为 true
 * 2. 处理脏状态：在适当的时机（如每帧更新时）检查并处理脏状态
 * 3. 清除脏标记：处理完成后将 m_Dirty 设为 false
 */
template<typename T>
class DirtyComponentSystem : public ComponentSystem<T>, public DirtyComponentSystemBase {
  // 限制模板T必须继承自Component类型
  static_assert(std::is_base_of<DirtyComponent, T>::value, "T must inherit from Dirty Component");
 public:
  DirtyComponentSystem() : ComponentSystem<T>(){};

   /**
   * @brief 系统更新（每帧调用）
   * @param deltaTime 帧间隔时间(秒)
   * @param registry 注册表
   */
  virtual void Update(float deltaTime, SceneRegistry &registry) override
  {
    // 阶段1：收集脏组件
    CollectDirtyComponents();

    // 阶段2：并行处理
    ProcessDirtyComponents(deltaTime, registry);
  }
  /**
   * @brief 获取脏组件数量
   */
  size_t GetDirtyComponentCount() const override
  {
    return m_DirtyComponents.size();
  }

  /**
   * @brief 强制标记所有组件为脏（用于特殊情况）
   */
  void MarkAllComponentsDirty()
  {
    std::lock_guard<std::mutex> lock(this->m_Mutex);
    for (auto comp : this->m_AllComponents) {
      comp->MarkDirty();
    }
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
   * @brief 并行执行脏组件的Update
   */
  virtual void ProcessDirtyComponents(float deltaTime, SceneRegistry &registry)
  {
    // 并行处理优化
    ParallelUtils::ForEach(m_DirtyComponents, [deltaTime, &registry](T *comp) {
      if (comp) {
        static_cast<DirtyComponent *>(comp)->Update(deltaTime, registry);
      }
    });
  }

  std::vector<T *> m_DirtyComponents;
};
};  // namespace mite

#endif
