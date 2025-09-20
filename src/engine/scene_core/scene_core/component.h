#ifndef MITE_SCENE_COMPONENT
#define MITE_SCENE_COMPONENT

#include "entity.h"
#include "scene_core/component_snapshot.h"
#include "subscription_group.h"

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
    return false;  // 默认不支持，需要支持的直接override该方法
  }

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
 * @tparam T 组件类型（主要用于快照和序列化）
 * @tparam F 组件家族
 */
template<typename T, Component::Family F>
class ComponentTraits : public Component {
 public:
  static constexpr Family family = F;
  using DataType = T;
  ComponentTraits() : Component() {}
  virtual ~ComponentTraits() = default;
  Family GetFamily() const override
  {
    return family;
  }
  std::type_index GetType() const override
  {
    return typeid(*this);
  }
};

/**
 * @brief 支持快照的组件基类，若需要处理脏标记则需要继承自该类
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
class SnapshotComponent : public Component {
 public:
  virtual ~SnapshotComponent() = default;
};

/**
 * @brief 支持快照的组件类型特征模板，用于简化组件定义
 * @tparam T 组件类型
 * @tparam F 组件家族
 */
template<typename T, Component::Family F>
class SnapshotComponentTraits: public SnapshotComponent {
 public:
  // 添加自描述类型别名
  using SnapshotDataType = T;
  static constexpr Family family = F;
  SnapshotComponentTraits() : SnapshotComponent() {}
  virtual ~SnapshotComponentTraits() = default;
  Family GetFamily() const override
  {
    return family;
  }
  std::type_index GetType() const override
  {
    return typeid(*this);
  }
  // ================== 快照相关 ======================
  /**
   * @brief 创建组件快照
   *
   * 需要支持快照的组件必须重写此方法，返回该组件数据的快照
   * 快照应该是组件数据的深拷贝，确保撤销/重做操作的安全性
   *
   * @return std::unique_ptr<ISnapshot> 组件快照智能指针
   * @throws 如果组件不支持快照，默认实现返回nullptr
   */
  auto CreateSnapshot() const
  {
    return CreateComponentSnapshot<T>(GetEntity(), GetSnapshotData());
  }
  /**
   * @brief 应用快照数据到组件
   *
   * 需要支持快照的组件必须重写此方法，将快照数据应用到当前组件
   * 应用成功后应该发布相应的组件更新事件，通知其他系统数据变更
   *
   * @param snapshotData 快照的GetData()
   * @return bool 是否成功应用快照
   * @throws 如果组件不支持快照，默认实现返回false
   */
  bool ApplySnapshot(const T &snapshotData)
  {
    try {
      SetSnapshotData(snapshotData);
      return true;
    }
    catch (const std::exception &e) {
      LOG_ERROR("Failed to apply snapshot: {}", e.what());
      return false;
    }
  }
  /**
   * @brief 获取快照数据大小
   *
   * 用于内存统计和优化，返回该组件快照数据的大小
   *
   * @return size_t 快照数据大小（字节）
   */
  size_t GetSnapshotDataSize() const
  {
    return sizeof(T);
  }
 protected:
  /**
   * @brief 获取 / 设置快照数据 - 子类必须实现
   */
  virtual T GetSnapshotData() const = 0;
  virtual void SetSnapshotData(const T &data) = 0;
};

/**
 * @brief 支持脏标记的组件基类，若需要处理脏标记则需要继承自该类
 */
class DirtyComponent : public Component {
 public:
  virtual ~DirtyComponent() = default;
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
  using DataType = T;
  DirtyComponentTraits() : DirtyComponent() {}
  virtual ~DirtyComponentTraits() = default;
  Family GetFamily() const override
  {
    return family;
  }
  std::type_index GetType() const override
  {
    return typeid(*this);
  }
};
};  // namespace mite

#endif
