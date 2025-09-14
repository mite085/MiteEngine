#ifndef MITE_SCENE_COMPONENT_SYSTEM_MANAGER
#define MITE_SCENE_COMPONENT_SYSTEM_MANAGER

#include "component_system.h"
#include "scene_core_event.h"
#include "scene_core_event_callback_adapter.h"
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

  // SFINAE 检测 T 是否有 ComponentType 成员类型
  template<typename T, typename = void> struct HasComponentType : std::false_type {};
  template<typename T>
  struct HasComponentType<T, std::void_t<typename T::ComponentType>> : std::true_type {};

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

    const std::type_index type = typeid(T);

    // 1. 检查是否已注册，若已注册则直接返回已有的系统
    if (m_SystemMap.find(type) != m_SystemMap.end()) {
      return static_cast<T *>(m_SystemMap[type]);
    }

    // 2. 创建新系统
    auto system = std::make_unique<T>(std::forward<Args>(args)...);
    T *rawPtr = system.get();

    // 3. 存入管理结构
    m_SystemMap[type] = rawPtr;
    m_Systems.push_back(std::move(
        system));  // 注意此处的system为临时变量，未能正确move会提前触发组件系统的析构函数
    m_SystemsSorted = false;

    // 4. 注册通用组件回调
    if constexpr (HasComponentType<T>::value) {
      using U = typename T::ComponentType;
      static_assert(std::is_base_of<Component, U>::value,
                    "Registered component must inherit from class component");
      m_Registry.GetEventPublisher().RegisterComponentCallbacks<U>();
    }
    return rawPtr;
  }

  /**
   * @brief 注销组件系统
   * @tparam T 系统类型
   * @tparam U 组件类型
   */
  template<typename T> void UnregisterSystem()
  {
    static_assert(std::is_base_of<ComponentSystem, T>::value,
                  "Registered system must inherit from ComponentSystem");
    const std::type_index type = typeid(T);
    // 1. 检查是否已注销，若已注销则直接返回
    auto mapIt = m_SystemMap.find(type);
    if (mapIt == m_SystemMap.end()) {
      return;
    }

    // 2. 从unordered_map中删除
    ComponentSystem *systemPtr = mapIt->second;
    m_SystemMap.erase(mapIt);

    // 3. 从vector中移除对应的unique_ptr
    auto vecIt = std::find_if(m_Systems.begin(),
                              m_Systems.end(),
                              [systemPtr](const std::unique_ptr<ComponentSystem> &ptr) {
                                return ptr.get() == systemPtr;
                              });

    if (vecIt != m_Systems.end()) {
      m_Systems.erase(vecIt);  // 这会删除unique_ptr，从而释放内存
    }
    else {
      // 仅当map和vector双重存储结构出现问题时触发：map中
      // 能查找到并且正常删除，但vector未能查找到对应的unique_ptr
      assert(false && "Inconsistent state: system found in map but not in vector");
      return;
    }

    // 4. 注销通用组件回调
    if constexpr (HasComponentType<T>::value) {
      using U = typename T::ComponentType;
      static_assert(std::is_base_of<Component, U>::value,
                    "Registered component must inherit from class component");
      m_Registry.GetEventPublisher().UnregisterComponentCallbacks<U>();
    }
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
    if (it != m_SystemMap.end() && it->second) {
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
    if (it != m_SystemMap.end()) {
      return static_cast<T *>(it->second);
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
   * @brief 系统执行顺序排序
   *
   * 通过获取当前系统依赖的其他系统类型，
   * 使用拓扑排序算法，确保被依赖的系统排序在前。
   * 在系统更新阶段，优先更新被依赖的系统，
   * 后更新依赖其他系统的系统
   */
  void SortSystems();

 private:
  SceneRegistry &m_Registry;

  std::vector<std::unique_ptr<ComponentSystem>> m_Systems;             // 用于遍历
  std::unordered_map<std::type_index, ComponentSystem *> m_SystemMap;  // 用于查找
  bool m_SystemsSorted = false;
};
};  // namespace mite

#endif
