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
   * @param registry 关联的EnTT registry
   * @param deltaTime 帧间隔时间(秒)
   */
  virtual void Update(SceneRegistry &registry, float deltaTime) = 0;

  /**
   * @brief 系统销毁（场景卸载时调用）
   * @param registry 关联的EnTT registry
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

  /**
   * @brief 是否需要在编辑器模式下运行
   */
  virtual bool RunInEditor() const
  {
    return true;
  }

 protected:
  // 保护构造函数，确保只能通过派生类实例化
  ComponentSystem() = default;

  // 禁用拷贝
  ComponentSystem(const ComponentSystem &) = delete;
  ComponentSystem &operator=(const ComponentSystem &) = delete;
};

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
 * 6. Scene销毁阶段--调用ShutdownAll()，关闭所有系统
 */
class ComponentSystemManager {
 public:
  ComponentSystemManager(SceneRegistry &registry);
  ~ComponentSystemManager();

  /**
   * @brief 注册组件系统
   * @tparam T 系统类型
   * @tparam Args 构造参数类型
   * @param args 构造参数
   * @return 注册的系统指针
   */
  template<typename T, typename... Args> T *RegisterSystem(Args &&...args);

  /**
   * @brief 获取已注册的系统
   * @tparam T 系统类型
   * @return 系统指针，未找到返回nullptr
   */
  template<typename T> bool HasSystem() const;

  /**
   * @brief 获取已注册的系统
   * @tparam T 系统类型
   * @return 系统指针，未找到返回nullptr
   */
  template<typename T> T *GetSystem() const;

  /**
   * @brief 初始化所有系统
   * @param registry EnTT registry
   */
  void InitializeAll();

  /**
   * @brief 更新所有系统
   * @param registry EnTT registry
   * @param deltaTime 帧间隔时间
   */
  void UpdateAll(float deltaTime);

  /**
   * @brief 销毁所有系统
   * @param registry EnTT registry
   */
  void ShutdownAll();

  /**
   * @brief 设置系统是否启用
   * @tparam T 系统类型
   * @param enabled 是否启用
   */
  template<typename T> void SetSystemEnabled(bool enabled)
  {
    const std::type_index type = typeid(T);
    auto it = m_SystemMap.find(type);
    if (it != m_SystemMap.end()) {
      it->second.enabled = enabled;
    }
  }

  /**
   * @brief 检查系统是否启用
   * @tparam T 系统类型
   */
  template<typename T> bool IsSystemEnabled() const
  {
    const std::type_index type = typeid(T);
    auto it = m_SystemMap.find(type);
    return it != m_SystemMap.end() && it->second.enabled;
  }

private:

  /**
   * @brief 当组件被添加时的处理
   * @param entity 实体
   * @param component 组件
   */
  void OnComponentAdded(Entity entity, Component &component);

  /**
   * @brief 当组件被更新时的处理
   * @param entity 实体
   * @param component 组件
   */
  void OnComponentUpdated(Entity entity, Component &component);

  /**
   * @brief 当组件被移除时的处理
   * @param entity 实体
   * @param component 组件
   */
  void OnComponentRemoved(Entity entity, Component &component);

  // 系统执行顺序排序
  void SortSystems();

 private:
  SceneRegistry &m_Registry;

  struct SystemEntry {
    std::unique_ptr<ComponentSystem> system;
    bool enabled = true;
  };

  std::vector<std::unique_ptr<ComponentSystem>> m_Systems;      // 用于遍历
  std::unordered_map<std::type_index, SystemEntry> m_SystemMap; // 用于查找
  bool m_SystemsSorted = false;
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

};

#endif
