#ifndef MITE_SCENE_COMPONENT
#define MITE_SCENE_COMPONENT

#include "headers/headers.h"
#include <entt/entt.hpp>

namespace mite {
// 前向声明
class Entity;

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
  // 组件家族类型标识，用于组件分类
  enum class Family : uint8_t {
    Core = 0,     // 核心组件(Transform等)
    Render,       // 渲染相关组件
    Geometry,     // 几何相关组件
    Logic,        // 逻辑/行为组件
    Custom = 100  // 自定义组件起始值
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
   */
  virtual std::shared_ptr<Component> Clone() const = 0;

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
   */
  virtual std::vector<std::type_index> GetDependencies() const
  {
    return {};
  }

 protected:
  // 保护构造函数，确保只能通过子类实例化，
  // 并接收对Entity的弱引用，确保支持向上查询
  explicit Component(std::weak_ptr<Entity> owner);

  std::weak_ptr<Entity> GetOwner() const
  {
    return m_OwnerEntity;
  }

 private:
  std::weak_ptr<Entity> m_OwnerEntity;

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

  explicit ComponentTraits(std::weak_ptr<Entity> owner) : Component(owner) {}

  Family GetFamily() const override
  {
    return family;
  }
  std::type_index GetType() const override
  {
    return typeid(T);
  }
  std::shared_ptr<Component> Clone() const override
  {
    return std::make_shared<T>(static_cast<const T &>(*this));
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
};

#endif
