#ifndef MITE_SCENE_COMPONENT
#define MITE_SCENE_COMPONENT

#include "headers/headers.h"
#include "entity.h"

namespace mite {

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
    Core = 0,      // 核心组件(Transform等)
    Render = 1,    // 渲染相关组件
    Geometry = 2,  // 几何相关组件
    Logic = 3,     // 逻辑/行为组件
    Custom = 100   // 自定义组件起始值
  };

  virtual ~Component() = default;

  /**
   * @brief 获取组件类型家族
   */
  virtual Family GetFamily() const = 0;

  /**
   * @brief 获取组件类型唯一ID
   */
  virtual std::type_index GetType() const = 0;

  /**
   * @brief 克隆组件(深拷贝)
   * @return 新组件实例的共享指针
   *
   * 注意：Component克隆方法，和HierarchyComponent的禁用拷贝构造函数，
   * 相互冲突，引发编译错误。现阶段优先确保HierarchyComponent禁止拷贝，
   * 后续需要深拷贝时添加clone方法
   */
  // virtual std::shared_ptr<Component> Clone() const = 0;

  /**
   * @brief 标记组件为已修改
   * @param dirty 是否为脏数据
   */
  void SetDirty(bool dirty = true);
  /**
   * @brief 检查组件是否被修改过
   */
  bool IsDirty() const;
  /**
   * @brief 更新方法，通常每帧调用
   */
  void Update();
  /**
   * @brief 针对dirty对象进行处理
   */
  virtual void Recalculate() = 0;

  /**
   * @brief 组件启用状态
   */
  void SetEnabled(bool enabled);
  bool IsEnabled() const;

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

  /**
   * @brief 返回组件是否依赖于其他组件
   * @return 依赖的组件类型列表
   *
   * 注意:
   * 与场景树直接相关的组件(如TransformComponent、
   * LightComponent、CameraComponent等)，需要依赖
   * HierarchyComponent，它就是组成场景树的核心。
   *
   * 或者通过对TransformComponent的依赖，实现对
   * HierarchyComponent的间接依赖(如MeshComponent、
   * AnimationComponent等)。
   */
  virtual std::vector<std::type_index> GetDependencies() const
  {
    return {};
  }

  /**
   * @brief 设定所属实体对象
   * @param entity 实体对象
   * 
   * 注意：
   * 由于SceneRegistry::AddComponent所调用的
   * m_Registry.emplace<T>(entt::entity, Args &&...args)
   * 方法对完美转发的参数包的要求，Component的构造函数
   * 所传入的参数必须和参数包的参数类型一致，
   * 故需要单独将SetOwnerEntity分离开执行。
   * 
   * TODO: entt对这部分的设定，说明了Component的
   * 内部逻辑不应当依赖于Entity对象。所以该函数
   * 是违背entt的设计理念的。后续应当考虑删除
   * 
   */
  void SetOwnerEntity(Entity entity);

 protected:
  // 保护构造函数，确保只能通过子类实例化，
  explicit Component() = default;

  Entity GetOwnerEntity() const;

  Entity m_OwnerEntity;

  bool m_Dirty = false;   // 脏标记，标识组件是否被修改
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

  explicit ComponentTraits() : Component() {}

  Family GetFamily() const override
  {
    return family;
  }
  std::type_index GetType() const override
  {
    return typeid(T);
  }

  /**
   * 注意：Component克隆方法，和HierarchyComponent的禁用拷贝构造函数，
   * 相互冲突，引发编译错误。现阶段优先确保HierarchyComponent禁止拷贝，
   * 后续需要深拷贝时添加clone方法
   */
  //std::shared_ptr<Component> Clone() const override
  //{
  //  return std::make_shared<T>(static_cast<const T &>(*this));
  //}

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
