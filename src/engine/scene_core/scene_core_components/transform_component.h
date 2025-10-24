#ifndef MITE_SCENE_TRANSFORM_COMPONENT
#define MITE_SCENE_TRANSFORM_COMPONENT

#include "basic_data/transform.h"
#include "scene_core/component_system.h"

namespace mite {
/**
 * @brief 变换组件，管理实体的位置、旋转和缩放
 *
 * 功能特性：
 * 1. 维护Transform，并常用的变换操作接口
 * 2. 仅支持局部空间变换，世界空间变换由SceneNode负责传递
 * 3. 提供相机View矩阵创建功能
 */
class TransformComponent
    : public SnapshotComponentTraits<Transform, Component::Family::Transform> {
 public:
  /**
   * @brief 默认构造函数
   */
  TransformComponent();

  /**
   * @brief 带初始值的构造函数
   * @param position 位置坐标
   * @param rotation 旋转
   * @param scale 缩放
   * @param order 旋转顺序
   */
  explicit TransformComponent(const glm::vec3 position,
                              const glm::vec3 rotation = glm::vec3(0.0f),
                              const glm::vec3 scale = glm::vec3(1.0f),
                              const Transform::EulerOrder order = Transform::EulerOrder::XYZ);

  /**
   * @brief 使用变换矩阵的构造函数
   * @param matrix 变换矩阵
   * @param order 旋转顺序
   */
  explicit TransformComponent(const glm::mat4 &matrix,
                              const Transform::EulerOrder order = Transform::EulerOrder::XYZ);

  ~TransformComponent() override = default;
  std::vector<std::type_index> GetDependencies() const override;

  // ==================== 数据操作 ====================
  const Transform& GetLocalTransform() const;
  /**
   * @brief 支持直接的值赋予与对矩阵直接执行操作两种模式。确保可便捷访问矩阵接口
   * @note 使用实例：
   * 1. transformComponent.SetLocalTransform(newTransform);
   * 2. transformComponent.SetLocalTransform([](Transform &localtrans) {
   *     localtrans.SetPosition(glm::vec3(5.0f, 5.0f, 5.0f));
   *     localtrans.LookAt(glm::vec3(0.0f, 0.0f, 0.0f));
   *   });
   */
  void SetLocalTransform(const Transform &transform);
  void SetLocalTransform(std::function<void(Transform&)> transformOperator);

  // ==================== 序列化接口 ====================
  bool Serialize(std::ostream &output) const override;
  bool Deserialize(std::istream &input) override;

 protected:
  // ==================== 快照接口 ====================
  const Transform &GetSnapshotData() const override;
  void SetSnapshotData(const Transform &data) override;

 private:
  std::shared_ptr<Transform> m_Transform;  // 基础变换对象
};

// ==================== 组件系统 ====================
class TransformComponentSystem : public SnapshotComponentSystem<TransformComponent> {
  DECLARE_COMPONENT_SYSTEM(TransformComponentSystem)
};
// ==================== 事件定义 ====================
/**
 * @class TransformUpdatedEvent
 * @brief Transform组件更新事件
 */
class TransformUpdatedEvent : public ComponentEvent<TransformComponent> {
 public:
  TransformUpdatedEvent(Entity entity, TransformComponent &component)
      : ComponentEvent<TransformComponent>(entity, component)
  {
  }
  EVENT_CLASS_CATEGORY(EVENT_CATEGORY_SCENE_CHANGE)
  Event *Clone() const override
  {
    return new TransformUpdatedEvent(entity, component);
  }
};
};  // namespace mite

#endif
