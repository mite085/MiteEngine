#ifndef MITE_SCENE_COMPONENT
#define MITE_SCENE_COMPONENT

#include "headers/headers.h"

namespace mite {

class Entity;   // 前向声明

/**
 * @class Component
 * @brief 所有组件的基类，提供通用接口和基础功能
 *
 * 组件是纯数据容器，不应包含业务逻辑。每个组件关联到一个实体，
 * 并通过类型标识进行区分。组件生命周期由其所属的实体管理。
 */
class Component {
 public:
  // 使用智能指针别名简化代码
  using Ptr = std::shared_ptr<Component>;
  using WeakPtr = std::weak_ptr<Component>;

  /**
   * @brief 虚析构函数确保派生类正确析构
   */
  virtual ~Component() = default;

  // 禁止拷贝构造和赋值，组件应通过Clone()方法复制
  Component(const Component &) = delete;
  Component &operator=(const Component &) = delete;

  /**
   * @brief 获取组件类型ID，用于运行时类型识别
   * @return 组件类型的唯一标识符
   */
  virtual uint32_t GetTypeId() const = 0;

  /**
   * @brief 获取组件类型名称，用于调试和序列化
   * @return 组件类型的可读名称
   */
  virtual const std::string &GetTypeName() const = 0;

  /**
   * @brief 获取拥有此组件的实体
   * @return 拥有此组件的实体弱引用，可能为空
   */
  Entity *GetOwner() const
  {
    return m_Owner;
  }

  /**
   * @brief 设置组件所属的实体（仅供Entity类内部使用）
   * @param owner 拥有此组件的新实体
   */
  void SetOwner(Entity *owner)
  {
    m_Owner = owner;
  }

  /**
   * @brief 检查组件是否已附加到实体
   * @return 如果组件已附加到实体则返回true
   */
  bool IsAttached() const
  {
    return m_Owner != nullptr;
  }

  /**
   * @brief 创建组件的深拷贝
   * @return 组件的新实例，所有权由调用者持有
   */
  virtual Ptr Clone() const = 0;

  /**
   * @brief 序列化组件数据到输出流
   * @param output 目标输出流
   * @return 序列化是否成功
   */
  virtual bool Serialize(std::ostream &output) const = 0;

  /**
   * @brief 从输入流反序列化组件数据
   * @param input 源输入流
   * @return 反序列化是否成功
   */
  virtual bool Deserialize(std::istream &input) = 0;

  /**
   * @brief 当组件被添加到实体时调用
   */
  virtual void OnAttached() {}

  /**
   * @brief 当组件从实体移除时调用
   */
  virtual void OnDetached() {}

  /**
   * @brief 当组件被激活时调用（实体激活或组件单独激活）
   */
  virtual void OnEnabled() {}

  /**
   * @brief 当组件被禁用时调用（实体禁用或组件单独禁用）
   */
  virtual void OnDisabled() {}

 protected:
  // 保护构造函数，防止直接实例化基类
  Component() = default;

  // 允许移动语义
  Component(Component &&) = default;
  Component &operator=(Component &&) = default;

 private:
  Entity *m_Owner = nullptr;  // 拥有此组件的实体，不拥有所有权
};
}  // namespace mite

#endif