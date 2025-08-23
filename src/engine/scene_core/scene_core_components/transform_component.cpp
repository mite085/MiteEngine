#include "transform_component.h"
#include "hierarchy_component.h"
#include "scene_core/entity.h"

namespace mite {
TransformComponent::TransformComponent() : ComponentTraits(), m_Transform() {}

TransformComponent::TransformComponent(const glm::vec3 &position,
                                       const glm::vec3 &rotation,
                                       const glm::vec3 &scale)
    : ComponentTraits(), m_Transform(position, rotation, scale)
{
}

TransformComponent::TransformComponent(const glm::mat4 &matrix)
    : ComponentTraits(), m_Transform(matrix)
{
}

void TransformComponent::ProcessDirty(float deltaTime, SceneRegistry &reg)
{
  if (!IsDirty() && !m_HierarchyDirty)
    return;
  // 更新世界矩阵
  UpdateWorldMatrix(reg);

  // 发布更新事件
  EventBus::Get().Post(TransformUpdatedEvent(GetOwnerEntity(), *this));

  // 清除标记
  m_HierarchyDirty = false;
  ClearDirty();
}
// 位置相关方法 ==============================================

const glm::vec3 &TransformComponent::GetLocalPosition() const
{
  return m_Transform.GetPosition();
}

void TransformComponent::SetLocalPosition(const glm::vec3 &position)
{
  if (m_Transform.GetPosition() != position) {
    m_Transform.SetPosition(position);
    MarkDirty();

    EventBus::Get().Post(PositionChangedEvent(GetOwnerEntity(), *this, position, false));
  }
}

glm::vec3 TransformComponent::GetWorldPosition(SceneRegistry &reg) const
{
  // 由GetWorldMatrix确保世界矩阵是最新的
  const glm::mat4 &worldMat = GetWorldMatrix(reg);
  return glm::vec3(worldMat[3]);
}

void TransformComponent::SetWorldPosition(SceneRegistry &reg, const glm::vec3 &position)
{
  // 转换为LocalPosition后调用SetLocalPosition
  // 注意：MarkDirty和EventPost均由SetLocal函数负责，此处调用SetLocal函数后，无需执行这些操作
  if (reg.HasComponent<HierarchyComponent>(GetOwnerEntity())) {
    auto &hierarchy = reg.GetComponent<HierarchyComponent>(GetOwnerEntity());
    if (hierarchy.GetParent().IsValid()) {
      // 如果有父节点，转换为局部空间
      TransformComponent &parentTransform = reg.GetComponent<TransformComponent>(
          hierarchy.GetParent());
      glm::mat4 parentWorldMat = parentTransform.GetWorldMatrix(reg);
      glm::mat4 inverseParent = glm::inverse(parentWorldMat);
      glm::vec4 localPos = inverseParent * glm::vec4(position, 1.0f);
      SetLocalPosition(glm::vec3(localPos));
      return;
    }
  }
  // 无父节点，或父节点不可用，即此节点为根节点
  SetLocalPosition(position);
}

// 旋转相关方法 ==============================================

glm::vec3 TransformComponent::GetLocalRotation() const
{
  return m_Transform.GetRotation();
}

void TransformComponent::SetLocalRotation(const glm::vec3 &rotation)
{
  if (m_Transform.GetRotation() != rotation) {
    m_Transform.SetRotation(rotation);
    MarkDirty();

    EventBus::Get().Post(RotationChangedEvent(GetOwnerEntity(), *this, rotation, false));
  }
}

void TransformComponent::SetLocalRotation(float x, float y, float z)
{
  SetLocalRotation(glm::vec3{x, y, z});
}

glm::quat TransformComponent::GetLocalRotationQuat() const
{
  return EulerDegreesToQuat(m_Transform.GetRotation());
}
void TransformComponent::SetLocalRotationQuat(const glm::quat &rotation)
{
  glm::vec3 euler = QuatToEulerDegrees(rotation);
  SetLocalRotation(euler);
}

glm::vec3 TransformComponent::GetWorldRotation(SceneRegistry &reg) const
{
  const glm::mat4 &worldMat = GetWorldMatrix(reg);
  glm::quat worldQuat = glm::quat_cast(worldMat);
  return QuatToEulerDegrees(worldQuat);
}

void TransformComponent::SetWorldRotation(SceneRegistry &reg, const glm::vec3 &rotation)
{
  // 转换为LocalRotation后调用SetLocalRotation
  // 注意：MarkDirty和EventPost均由SetLocal函数负责，此处调用SetLocal函数后，无需执行这些操作
  if (reg.HasComponent<HierarchyComponent>(GetOwnerEntity())) {
    auto &hierarchy = reg.GetComponent<HierarchyComponent>(GetOwnerEntity());
    if (hierarchy.GetParent().IsValid()) {
      // 转换为局部旋转
      TransformComponent &parentTransform = reg.GetComponent<TransformComponent>(
          hierarchy.GetParent());
      glm::vec3 parentWorldRot = parentTransform.GetWorldRotation(reg);

      // 计算相对旋转（欧拉角减法可能不准确，建议使用四元数）
      // 这里使用四元数进行精确计算
      glm::quat targetWorldQuat = EulerDegreesToQuat(rotation);
      glm::quat parentWorldQuat = EulerDegreesToQuat(parentWorldRot);
      glm::quat localQuat = targetWorldQuat * glm::inverse(parentWorldQuat);

      SetLocalRotationQuat(localQuat);
      return;
    }
  }
  SetLocalRotation(rotation);
}

void TransformComponent::SetWorldRotationQuat(SceneRegistry &reg, const glm::quat &rotation)
{
  SetWorldRotation(reg, QuatToEulerDegrees(rotation));
}

void TransformComponent::Rotate(const glm::vec3 &axis, float angle)
{
  // 使用Transform内置的Rotate方法（deg）
  m_Transform.Rotate(axis, angle);
  MarkDirty();
}

void TransformComponent::RotateAround(SceneRegistry &reg,
                                      const glm::vec3 &worldPoint,
                                      const glm::vec3 &worldAxis,
                                      float angle)
{
  // 获取当前世界位置和旋转
  glm::vec3 worldPos = GetWorldPosition(reg);
  glm::vec3 worldRot = GetWorldRotation(reg);

  // 如果有父节点，需要将世界坐标转换为局部坐标
  if (reg.HasComponent<HierarchyComponent>(GetOwnerEntity())) {
    auto &hierarchy = reg.GetComponent<HierarchyComponent>(GetOwnerEntity());
    if (hierarchy.GetParent().IsValid()) {
      TransformComponent &parentTransform = reg.GetComponent<TransformComponent>(
          hierarchy.GetParent());

      // 将世界坐标点转换到父节点局部空间
      glm::mat4 parentWorldMat = parentTransform.GetWorldMatrix(reg);
      glm::mat4 inverseParent = glm::inverse(parentWorldMat);
      glm::vec4 localPoint = inverseParent * glm::vec4(worldPoint, 1.0f);

      // 将世界轴转换到父节点局部空间
      glm::vec3 localAxis = glm::vec3(glm::inverse(glm::mat3(parentWorldMat)) * worldAxis);

      // 在局部空间执行旋转
      m_Transform.RotateAround(glm::vec3(localPoint), localAxis, angle);
      MarkDirty();
      return;
    }
  }

  // 没有父节点，直接在世界空间执行
  m_Transform.RotateAround(worldPoint, worldAxis, angle);
  MarkDirty();
}

void TransformComponent::LookAt(SceneRegistry &reg, const glm::vec3 &target, const glm::vec3 &up)
{
  glm::vec3 worldPos = GetWorldPosition(reg);
  m_Transform.LookAt(worldPos, target, up);
  MarkDirty();
}

// 缩放相关方法 ==============================================

const glm::vec3 &TransformComponent::GetLocalScale() const
{
  return m_Transform.GetScale();
}

void TransformComponent::SetLocalScale(const glm::vec3 &scale)
{
  if (m_Transform.GetScale() != scale) {
    m_Transform.SetScale(scale);
    MarkDirty();

    EventBus::Get().Post(ScaleChangedEvent(GetOwnerEntity(), *this, scale, false));
  }
}

void TransformComponent::SetLocalScale(float scale)
{
  SetLocalScale(glm::vec3{scale, scale, scale});
}

glm::vec3 TransformComponent::GetWorldScale(SceneRegistry &reg) const
{
  const glm::mat4 &worldMat = GetWorldMatrix(reg);
  return glm::vec3(glm::length(glm::vec3(worldMat[0])),
                   glm::length(glm::vec3(worldMat[1])),
                   glm::length(glm::vec3(worldMat[2])));
}

// 矩阵相关方法 ==============================================

glm::mat4 TransformComponent::GetLocalMatrix() const
{
  return m_Transform.GetLocalMatrix();
}

glm::mat4 TransformComponent::GetWorldMatrix(SceneRegistry &reg) const
{
  if (IsDirty() || m_HierarchyDirty) {
    UpdateWorldMatrix(reg);
  }
  return m_WorldMatrix;
}

void TransformComponent::SetLocalMatrix(const glm::mat4 &matrix)
{
  m_Transform.SetLocalMatrix(matrix);
  MarkDirty();

  EventBus::Get().Post(TransformChangedEvent(GetOwnerEntity(), *this, matrix, false));
}

void TransformComponent::SetWorldMatrix(SceneRegistry &reg, const glm::mat4 &matrix)
{
  if (reg.HasComponent<HierarchyComponent>(GetOwnerEntity())) {
    auto &hierarchy = reg.GetComponent<HierarchyComponent>(GetOwnerEntity());
    if (hierarchy.GetParent().IsValid()) {
      // 转换为局部空间
      TransformComponent &parentTransform = reg.GetComponent<TransformComponent>(
          hierarchy.GetParent());
      glm::mat4 parentWorldMat = parentTransform.GetWorldMatrix(reg);
      glm::mat4 localMat = glm::inverse(parentWorldMat) * matrix;
      SetLocalMatrix(localMat);
      return;
    }
  }
  // 没有父节点时直接设置本地矩阵
  SetLocalMatrix(matrix);
}

// 方向向量 ==============================================

glm::vec3 TransformComponent::Forward() const
{
  return m_Transform.GetForward();
}

glm::vec3 TransformComponent::Up() const
{
  return m_Transform.GetUp();
}

glm::vec3 TransformComponent::Right() const
{
  return m_Transform.GetRight();
}

// 组件接口实现 ==========================================

std::vector<std::type_index> TransformComponent::GetDependencies() const
{
  return {typeid(HierarchyComponent)};
}

bool TransformComponent::Serialize(std::ostream &output) const
{
  Component::Serialize(output);
  // 序列化留空，等待序列化模块
  return !output.fail();
}
bool TransformComponent::Deserialize(std::istream &input)
{
  Component::Deserialize(input);
  // 反序列化留空，等待序列化模块
  return !input.fail();
}

const Transform &TransformComponent::GetTransform() const
{
  return m_Transform;
}

Transform &TransformComponent::GetTransform()
{
  return m_Transform;
}

void TransformComponent::MarkHierarchyDirty()
{
  m_HierarchyDirty = true;
  MarkDirty();
}

bool TransformComponent::IsHierarchyDirty() const
{
  return m_HierarchyDirty;
}

// 私有方法 ==============================================

void TransformComponent::UpdateWorldMatrix(SceneRegistry &reg) const
{
  glm::mat4 localMat = GetLocalMatrix();

  if (reg.HasComponent<HierarchyComponent>(GetOwnerEntity())) {
    auto &hierarchy = reg.GetComponent<HierarchyComponent>(GetOwnerEntity());
    if (hierarchy.GetParent().IsValid()) {
      TransformComponent &parentTransform = reg.GetComponent<TransformComponent>(
          hierarchy.GetParent());
      // GetWorldMatrix递归计算确保最新
      // 这里确保向上传递正确，SceneGraph确保向下传递正确。
      m_WorldMatrix = parentTransform.GetWorldMatrix(reg) * localMat;
    }
    else {
      // 没有父节点，局部矩阵就是世界矩阵
      m_WorldMatrix = localMat;
    }
  }
  else {
    // 没有层次组件，局部矩阵就是世界矩阵
    m_WorldMatrix = localMat;
  }
}

glm::vec3 TransformComponent::QuatToEulerDegrees(const glm::quat &quat)
{
  glm::vec3 radians = glm::eulerAngles(quat);
  return glm::degrees(radians);
}
glm::quat TransformComponent::EulerDegreesToQuat(const glm::vec3 &euler)
{
  glm::vec3 radians = glm::radians(euler);
  return glm::quat(radians);
}

// ==================== 组件系统实现 ====================
void TransformComponentSystem::ProcessDirtyComponents(float deltaTime, SceneRegistry &registry)
{
  // 使用预分配的内存池（极端条件下该函数每帧都需要调用，需要减少多次分配带来的性能开销）
  thread_local static std::vector<Entity> processingBuffer;
  processingBuffer.clear();

  // 第一阶段：收集所有需要处理的实体
  auto view = registry.GetEntitiesWithAllOf<TransformComponent, HierarchyComponent>();
  for (auto entity : view) {
    auto &transform = registry.GetComponent<TransformComponent>(entity);
    if (transform.IsDirty() || transform.IsHierarchyDirty()) {
      processingBuffer.push_back(entity);
    }
  }

  // 定义深度获取Lambda函数
  auto GetDepth = [](Entity entity, SceneRegistry &registry) -> size_t {
    if (registry.HasComponent<HierarchyComponent>(entity)) {
      auto &hierarchy = registry.GetComponent<HierarchyComponent>(entity);
      return hierarchy.GetDepth(registry);
    }
    else {
      return 0;
    }
  };

  // 按层级深度排序（确保父先子后）
  std::sort(processingBuffer.begin(),
            processingBuffer.end(),
            [&registry, &GetDepth](Entity a, Entity b) {
              return GetDepth(a, registry) < GetDepth(b, registry);
            });

  // 按照父先子后的排序结果批量处理
  for (auto entity : processingBuffer) {
    auto &transform = registry.GetComponent<TransformComponent>(entity);
    transform.ProcessDirty(deltaTime, registry);
  }

  // 处理独立实体（这一步可以多线程，独立实体之间互相不影响）
  auto independentView = registry.GetEntitiesWith<TransformComponent>();
  for (auto entity : independentView) {
    auto &transform = registry.GetComponent<TransformComponent>(entity);
    if (transform.IsDirty()) {
      transform.ProcessDirty(deltaTime, registry);
    }
  }
}


};  // namespace mite