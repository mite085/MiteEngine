#ifndef MITE_SCENE_COMPONENT_SYSTEM_MANAGER
#define MITE_SCENE_COMPONENT_SYSTEM_MANAGER

#include "component_system.h"

namespace mite {
/**
 * @brief 组件系统管理器，集中管理所有组件系统
 *
 *
 * 使用方法：
 * 1. Scene构造阶段--调用默认构造函数，构造Manager对象
 * 2. Scene构造阶段--调用RegisterSystem()，逐个注册各个ComponentSystem
 *
 * 3. Scene初始化阶段--调用InitializeAll()，初始化所有系统
 *
 * 4. Scene运行阶段--每帧调用UpdateAll(deltaTime)，更新所有系统
 * 5. Scene运行阶段--视情况调用GetSystem()，针对某一系统进行处理
 *
 * 6.
 * Scene运行阶段--每当新的Component创建/更新/移除，触发对应ComponentSystem的回调函数OnComponentAdded等
 *
 * 7. Scene销毁阶段--调用ShutdownAll()，关闭所有系统
 */
class ComponentSystemManager {
 public:
  ComponentSystemManager(SceneRegistry &registry);
  ~ComponentSystemManager();

  /**
   * @brief 注册组件系统
   *
   * @tparam T 系统类型
   * @tparam U 组件类型
   * @tparam Args 构造参数类型
   * @param args 构造参数
   * @return 注册的系统指针
   */
  template<typename T, typename... Args> T *RegisterSystem(Args &&...args)
  {
    static_assert(std::is_base_of<ComponentSystem, T>::value,
                  "Registered system must inherit from ComponentSystem");
    using U = typename T::ComponentType;
    static_assert(std::is_base_of<Component, U>::value,
                  "Registered component must inherit from class component");

    const std::type_index type = typeid(T);

    // 检查是否已注册，若已注册则直接返回已有的系统
    if (m_SystemMap.find(type) != m_SystemMap.end()) {
      return static_cast<T *>(m_SystemMap[type]);
    }

    // 创建新系统
    auto system = std::make_unique<T>(std::forward<Args>(args)...);
    T *rawPtr = system.get();

    // 存入管理结构
    m_SystemMap[type] = rawPtr;
    m_Systems.push_back(std::move(system)); // 注意此处的system为临时变量，未能正确move会提前触发组件系统的析构函数
    m_SystemsSorted = false;

    // 注册组件事件回调
    m_Registry.RegisterCallbackComponentConstruct<U>(
        [this](Entity e, Component &c) { OnComponentAdded<U>(e, static_cast<U &>(c)); });
    m_Registry.RegisterCallbackComponentUpdate<U>(
        [this](Entity e, Component &c) { OnComponentUpdated<U>(e, static_cast<U &>(c)); });
    m_Registry.RegisterCallbackComponentDestroy<U>(
        [this](Entity e, Component &c) { OnComponentRemoved<U>(e, static_cast<U &>(c)); });

    int a = 1;
    return rawPtr;
  }

  /**
   * @brief 检查系统是否注册
   *
   * @tparam T 系统类型
   * @return 系统指针，未找到返回nullptr
   */
  template<typename T> bool HasSystem() const
  {
    const std::type_index type = typeid(T);
    auto it = m_SystemMap.find(type);
    if (it != m_SystemMap.end() && it->second.enabled) {
      return true;
    }
    return false;
  }

  /**
   * @brief 获取已注册的系统
   *
   * @tparam T 系统类型
   * @return 系统指针，未找到返回nullptr
   */
  template<typename T> T *GetSystem() const
  {
    // 获取前使用assert断言检查，便于在debug阶段发现问题。
    assert(HasSystem<T>());
    const std::type_index type = typeid(T);
    auto it = m_SystemMap.find(type);
    if (it != m_SystemMap.end() && it->second.enabled) {
      return static_cast<T *>(it->second.system.get());
    }
    return nullptr;
  }

  /**
   * @brief 初始化所有系统
   */
  void InitializeAll();

  /**
   * @brief 更新所有系统
   *
   * @param deltaTime 帧间隔时间
   */
  void UpdateAll(float deltaTime);

  /**
   * @brief 销毁所有系统
   */
  void ShutdownAll();

 private:
  /**
   * @brief 当组件被添加时的处理
   *
   * @param entity 实体
   * @param component 组件
   */
  template<typename T> void OnComponentAdded(Entity entity, T &component)
  {
    static_assert(std::is_base_of<Component, T>::value,
                  "Registered component must inherit from class component");

    const auto componentType = component.GetType();

    for (auto &system : m_Systems) {
      // 检查系统是否管理此组件类型
      for (const auto &managedType : system->GetComponentTypes()) {
        if (managedType == componentType) {
          system->OnComponentAdded(entity, component);
          break;
        }
      }
    }
  }

  /**
   * @brief 当组件被更新时的处理
   *
   * @param entity 实体
   * @param component 组件
   */
  template<typename T> void OnComponentUpdated(Entity entity, T &component)
  {
    static_assert(std::is_base_of<Component, T>::value,
                  "Registered component must inherit from class component");

    const auto componentType = component.GetType();

    for (auto &system : m_Systems) {
      // 检查系统是否管理此组件类型
      for (const auto &managedType : system->GetComponentTypes()) {
        if (managedType == componentType) {
          system->OnComponentUpdated(entity, component);
          break;
        }
      }
    }
  }

  /**
   * @brief 当组件被移除时的处理
   *
   * @param entity 实体
   * @param component 组件
   */
  template<typename T> void OnComponentRemoved(Entity entity, T &component)
  {
    static_assert(std::is_base_of<Component, T>::value,
                  "Registered component must inherit from class component");

    const auto componentType = component.GetType();

    for (auto &system : m_Systems) {
      // 检查系统是否管理此组件类型
      for (const auto &managedType : system->GetComponentTypes()) {
        if (managedType == componentType) {
          system->OnComponentRemoved(entity, component);
          break;
        }
      }
    }
  }

  // 系统执行顺序排序
  void SortSystems();

 private:
  SceneRegistry &m_Registry;

  std::vector<std::unique_ptr<ComponentSystem>> m_Systems;       // 用于遍历
  std::unordered_map<std::type_index, ComponentSystem *> m_SystemMap;  // 用于查找
  bool m_SystemsSorted = false;
};
};

#endif
