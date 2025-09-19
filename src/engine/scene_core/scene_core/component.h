#ifndef MITE_SCENE_COMPONENT
#define MITE_SCENE_COMPONENT

#include "entity.h"
#include "subscription_group.h"
#include "scene_core/component_snapshot.h"

namespace mite {
// 前向声明
class SceneRegistry;

/**
 * @brief 组件基类，所有场景组件都应继承自此类
 *
 * 设计原则：
 * 1. 轻量级 - 组件应是简单的数据容器
 * 2. 可序列化 - 支持保存和加载
 * 3. 类型安全 - 提供运行时类型信息
 * 4. 可克隆 - 支持深拷贝
 * 5. 可观察 - 支持变更通知
 */
class Component {
 public:
  // 组件家族类型标识，用于组件分类，以及优先级判断
  enum class Family : uint8_t {
    Core = 0,         // 核心基础组件（必须最先执行）
    Hierarchy = 10,   // 层级关系组件
    Transform = 20,   // 变换相关组件
    Geometry = 30,    // 几何数据组件
    Visibility = 40,  // 可见性计算组件
    SceneGraph = 50,  // 场景图同步组件
    Render = 60,      // 渲染准备组件
    Logic = 70,       // 逻辑/行为组件(未来扩展)
    Cleanup = 80,     // 清理相关组件
    Custom = 100,     // 自定义组件起始值(暂未规划)
    PostUpdate = 255  // 确保最后更新的组件(特殊用途)
  };

  virtual ~Component() = default;

  // ================== 组件标识相关 ======================
  /**
   * @brief 获取组件类型家族
   */
  virtual Family GetFamily() const = 0;
  /**
   * @brief 获取组件类型唯一ID
   */
  virtual std::type_index GetType() const = 0;
  /**
   * @brief 返回组件是否依赖于其他组件
   * @return 依赖的组件类型列表
   */
  virtual std::vector<std::type_index> GetDependencies() const
  {
    return {};
  }
  /**
   * @brief 组件启用状态
   */
  void SetEnabled(bool enabled);
  bool IsEnabled() const;

  // ================== 快照相关 ======================
  /**
   * @brief 是否支持快照（用于Undo和Redo）
   * 
   * 1. 需要快照的组件类型（举例）：
   *    TransformComponent - 核心变换数据
   *    MeshRendererComponent - 渲染相关状态
   *    LightComponent - 光源参数
   *    CameraComponent - 相机设置
   *    Hierarchy - 父子关系（SceneGraph）
   * 
   * 2. 不需要快照的组件类型（举例）：
   *    TagComponent - 标签信息（通常不重要）
   *    ScriptComponent - 脚本状态（复杂且难以序列化）
   *    TemporaryComponent - 临时数据
   *    SystemComponent - 系统内部状态
   */
  virtual bool SupportsSnapshot() const
  {
    return false; // 默认不支持，需要支持的直接override该方法
  }
  /**
   * @brief 创建快照
   * @return 组件快照对象
   */
  virtual std::unique_ptr<ComponentSnapshot> CreateSnapshot() const {}
  /**
   * @brief 应用快照
   * @param snapshot 
   */
  virtual void ApplySnapshot(const ComponentSnapshot &snapshot){}


  // ================== 序列化相关 ======================
  /**
   * @brief 序列化组件数据
   * @param output 输出流
   * @return 是否成功
   */
  virtual bool Serialize(std::ostream &output) const;
  /**
   * @brief 反序列化组件数据
   * @param input 输入流
   * @return 是否成功
   */
  virtual bool Deserialize(std::istream &input);



  // ================== 实体绑定相关 ======================
  /**
   * @brief 设定所属实体对象
   * @param entity 实体对象
   *
   * 问题：
   * 组件是否应当维护实体？存疑。
   * 原则上组件和实体应当是完全解耦的，
   * 该方法应当删除
   * 
   * 目前仅有事件发布和MainCamera维护需要实体
   * 事件订阅者即便延迟处理事件，也需要记录Entity
   */
  void SetOwnerEntity(Entity entity);
  /**
   * @brief 获取组件绑定的实体
   */
  Entity GetEntity() const;

 protected:
  // 保护构造函数，确保只能通过子类实例化，
  explicit Component() = default;

  // 禁用拷贝构造和赋值，组件仅由注册表维护
  Component(const Component &) = delete;
  Component &operator=(const Component &) = delete;

  Entity m_Entity;

  bool m_Enabled = true;  // 组件是否启用
};

/**
 * @brief 组件类型特征模板，用于简化组件定义
 * @tparam T 组件类型
 * @tparam F 组件家族
 */
template<typename T, Component::Family F> class ComponentTraits : public Component {
 public:
  static constexpr Family family = F;

  ComponentTraits() : Component() {}

  Family GetFamily() const override
  {
    return family;
  }
  std::type_index GetType() const override
  {
    return typeid(T);
  }

  // 启用静态类型检查的组件ID获取
  static std::type_index GetStaticType()
  {
    return typeid(T);
  }
  static Family GetStaticFamily()
  {
    return family;
  }
};

/**
 * @brief 支持脏标记的组件基类，若需要处理脏标记则需要继承自该类
 */
class DirtyComponent : public Component {
 public:
  /**
   * @brief 标记组件为已修改
   */
  void MarkDirty();
  /**
   * @brief 检查组件是否被修改过
   */
  bool IsDirty() const;
  /**
   * @brief 清理组件修改状态
   */
  void ClearDirty();

  /**
   * @brief 更新方法，每帧调用。
   */
  void Update(float deltaTime, SceneRegistry &reg);
  /**
   * @brief 针对dirty对象进行处理
   */
  virtual void ProcessDirty(float deltaTime, SceneRegistry &reg) = 0;

 protected:
  std::atomic<bool> m_Dirty{false};  // 脏标记，标识组件是否被修改
};

/**
 * @brief 支持脏标记的组件类型特征模板，用于简化组件定义
 * @tparam T 组件类型
 * @tparam F 组件家族
 */
template<typename T, Component::Family F> class DirtyComponentTraits : public DirtyComponent {
 public:
  static constexpr Family family = F;

  DirtyComponentTraits() : DirtyComponent() {}

  Family GetFamily() const override
  {
    return family;
  }
  std::type_index GetType() const override
  {
    return typeid(T);
  }

  // 启用静态类型检查的组件ID获取
  static std::type_index GetStaticType()
  {
    return typeid(T);
  }
  static Family GetStaticFamily()
  {
    return family;
  }
};
};  // namespace mite

#endif
