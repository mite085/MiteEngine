#include "transform_component.h"
#include "hierarchy_component.h"
#include "scene_core/entity.h"

namespace mite {
TransformComponent::TransformComponent() : ComponentTraits(), dirtyFlags(0) {}

TransformComponent::TransformComponent(const glm::vec3 &position,
                                       const glm::quat &rotation,
                                       const glm::vec3 &scale)
    : ComponentTraits(), m_Position(position), m_Rotation(rotation), m_Scale(scale), dirtyFlags(0)
{
  UpdateLocalMatrix();
}

TransformComponent::TransformComponent(const glm::mat4 &matrix) : ComponentTraits()
{
  SetLocalMatrix(matrix);
}

void TransformComponent::ProcessDirty(float deltaTime, SceneRegistry &reg)
{
  // 无需处理的情况快速返回
  if (!(m_Dirty || dirtyFlags))
    return;

  // 更新局部矩阵（如果局部属性变化）
  if (dirtyFlags & (LOCAL_DIRTY)) {
    UpdateWorldMatrix(reg);
  }

  // 仅计算自己的世界矩阵，不处理子节点
  if (dirtyFlags & (LOCAL_DIRTY | HIERARCHY_DIRTY)) {
    const glm::mat4 localMat = GetLocalMatrix();
    if (reg.HasComponent<HierarchyComponent>(GetOwnerEntity())) {
      auto &hierarchy = reg.GetComponent<HierarchyComponent>(GetOwnerEntity());
      if (hierarchy.GetParent().IsValid()) {
        m_WorldMatrix =
            reg.GetComponent<TransformComponent>(hierarchy.GetParent()).GetWorldMatrix(reg) *
            localMat;
      }
      else {
        m_WorldMatrix = localMat;
      }
    }
    else {
      m_WorldMatrix = localMat;
    }

    // 发布事件通知SceneGraph处理子节点更新
    EventBus::Get().Post(TransformUpdatedEvent(GetOwnerEntity(), *this));
  }

  // 清除标记（保留HIERARCHY_DIRTY供子节点处理）
  dirtyFlags &= ~(LOCAL_DIRTY | WORLD_DIRTY);
  m_Dirty = false;
}
// 位置相关方法 ==============================================

const glm::vec3 &TransformComponent::GetLocalPosition() const
{
  return m_Position;
}

void TransformComponent::SetLocalPosition(const glm::vec3 &position)
{
  if (m_Position != position) {
    m_Position = position;

    // 更新脏标记，需要重新计算世界变换
    dirtyFlags |= LOCAL_DIRTY;
    dirtyFlags |= HIERARCHY_DIRTY;
    MarkDirty();

    // 发布变更事件
    EventBus::Get().Post(PositionChangedEvent(GetOwnerEntity(), *this, m_Position, false));
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
  if (reg.HasComponent<HierarchyComponent>(GetOwnerEntity())) {
    auto &hierarchy = reg.GetComponent<HierarchyComponent>(GetOwnerEntity());
    if (hierarchy.GetParent().IsValid()) {
      // 如果有父节点，转换为局部位置
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

const glm::quat &TransformComponent::GetLocalRotation() const
{
  return m_Rotation;
}

void TransformComponent::SetLocalRotation(const glm::quat &rotation)
{
  if (m_Rotation != rotation) {
    m_Rotation = glm::normalize(rotation);  // 确保单位四元数

    // 更新脏标记，需要重新计算世界变换
    dirtyFlags |= LOCAL_DIRTY;
    dirtyFlags |= HIERARCHY_DIRTY;
    MarkDirty();

    // 发布事件通知SceneGraph处理子节点更新
    EventBus::Get().Post(RotationChangedEvent(GetOwnerEntity(), *this, m_Rotation, false));
  }
}

glm::vec3 TransformComponent::GetLocalEulerAngles() const
{
  return glm::eulerAngles(m_Rotation);
}

void TransformComponent::SetLocalEulerAngles(const glm::vec3 &eulerAngles)
{
  SetLocalRotation(glm::quat(eulerAngles));
}

void TransformComponent::SetLocalEulerAngles(float x, float y, float z)
{
  SetLocalEulerAngles(glm::vec3{x, y, z});
}

glm::quat TransformComponent::GetWorldRotation(SceneRegistry &reg) const
{
  // 由GetWorldMatrix确保世界矩阵是最新的
  const glm::mat4 &worldMat = GetWorldMatrix(reg);
  // 从世界矩阵提取旋转
  return glm::quat_cast(worldMat);
}

void TransformComponent::SetWorldRotation(SceneRegistry &reg, const glm::quat &rotation)
{
  if (reg.HasComponent<HierarchyComponent>(GetOwnerEntity())) {
    auto &hierarchy = reg.GetComponent<HierarchyComponent>(GetOwnerEntity());
    if (hierarchy.GetParent().IsValid()) {
      // 如果有父节点，转换为局部旋转
      TransformComponent &parentTransform = reg.GetComponent<TransformComponent>(
          hierarchy.GetParent());
      glm::quat parentWorldRot = parentTransform.GetWorldRotation(reg);
      // 计算相对旋转 = 目标旋转 * 父旋转逆
      SetLocalRotation(rotation * glm::inverse(parentWorldRot));
      return;
    }
  }
  // 没有父节点，世界旋转就是局部旋转
  SetLocalRotation(rotation);
}

void TransformComponent::Rotate(const glm::quat &rotation)
{
  // 应用旋转（局部空间，右乘）
  SetLocalRotation(glm::normalize(rotation * m_Rotation));
}

void TransformComponent::Rotate(const glm::vec3 &axis, float angle)
{  // 确保旋转轴是单位向量
  const glm::vec3 normalizedAxis = glm::normalize(axis);

  // 创建旋转四元数 (角度/2 因为四元数使用半角)
  const glm::quat rotation = glm::angleAxis(angle, normalizedAxis);

  // 应用旋转
  Rotate(rotation);
}

void TransformComponent::RotateAround(SceneRegistry &reg,
                                      const glm::vec3 &point,
                                      const glm::vec3 &axis,
                                      float angle)
{
  // 获取当前世界位置
  const glm::vec3 worldPos = GetWorldPosition(reg);

  // 创建旋转四元数
  const glm::vec3 normalizedAxis = glm::normalize(axis);
  const glm::quat rotation = glm::angleAxis(angle, normalizedAxis);

  // 计算新位置
  const glm::vec3 toObject = worldPos - point;
  const glm::vec3 rotatedVec = rotation * toObject;
  const glm::vec3 newWorldPos = point + rotatedVec;

  // 计算新的世界旋转
  glm::quat newWorldRot = rotation * GetWorldRotation(reg);

  // 应用变换
  SetWorldPosition(reg, newWorldPos);
  SetWorldRotation(reg, newWorldRot);
}

void TransformComponent::LookAt(SceneRegistry &reg, const glm::vec3 &target, const glm::vec3 &up)
{
  // 1. 获取世界空间位置
  glm::vec3 worldPos = GetWorldPosition(reg);

  // 2. 计算目标方向
  glm::vec3 forward = glm::normalize(target - worldPos);

  // 3. 处理向上向量特殊情况
  if (glm::length(forward - up) < 0.001f) {
    forward = glm::vec3(forward.x + 0.01f, forward.y, forward.z);
    forward = glm::normalize(forward);
  }

  // 4. 创建世界空间旋转矩阵
  glm::mat4 lookAtMat = glm::lookAt(worldPos, target, up);
  glm::quat worldRot = glm::quat_cast(glm::inverse(lookAtMat));

  // 5. 转换为局部旋转（如果有父级）
  if (reg.HasComponent<HierarchyComponent>(GetOwnerEntity())) {
    auto &hierarchy = reg.GetComponent<HierarchyComponent>(GetOwnerEntity());
    if (hierarchy.GetParent().IsValid()) {
      TransformComponent &parentTrans = reg.GetComponent<TransformComponent>(
          hierarchy.GetParent());
      glm::quat parentWorldRot = parentTrans.GetWorldRotation(reg);
      worldRot = worldRot * glm::inverse(parentWorldRot);
    }
  }

  // 6. 应用旋转
  SetLocalRotation(worldRot);
}

// 缩放相关方法 ==============================================

const glm::vec3 &TransformComponent::GetLocalScale() const
{
  return m_Scale;
}

void TransformComponent::SetLocalScale(const glm::vec3 &scale)
{
  if (m_Scale != scale) {
    glm::vec3 oldScale = m_Scale;
    m_Scale = scale;

    // 更新脏标记，需要重新计算世界变换
    dirtyFlags |= LOCAL_DIRTY;
    dirtyFlags |= HIERARCHY_DIRTY;
    MarkDirty();

    // 发布事件通知SceneGraph处理子节点更新
    EventBus::Get().Post(ScaleChangedEvent(GetOwnerEntity(), *this, m_Scale, false));
  }
}

void TransformComponent::SetLocalScale(float scale)
{
  SetLocalScale(glm::vec3{scale, scale, scale});
}

glm::vec3 TransformComponent::GetWorldScale(SceneRegistry &reg) const
{
  // 如果无需更新，直接提取缩放分量
  if (!(dirtyFlags & (LOCAL_DIRTY | HIERARCHY_DIRTY))) {
    return glm::vec3(glm::length(glm::vec3(m_WorldMatrix[0])),
                     glm::length(glm::vec3(m_WorldMatrix[1])),
                     glm::length(glm::vec3(m_WorldMatrix[2])));
  }
  // 需要更新世界矩阵的情况
  glm::mat4 worldMatrix = GetWorldMatrix(reg);
  return glm::vec3(glm::length(glm::vec3(worldMatrix[0])),  // X轴缩放
                   glm::length(glm::vec3(worldMatrix[1])),  // Y轴缩放
                   glm::length(glm::vec3(worldMatrix[2]))   // Z轴缩放
  );
}

// 矩阵相关方法 ==============================================

glm::mat4 TransformComponent::GetLocalMatrix() const
{
  if (dirtyFlags & LOCAL_DIRTY) {
    UpdateLocalMatrix();
  }
  return m_LocalMatrix;
}

glm::mat4 TransformComponent::GetWorldMatrix(SceneRegistry &reg) const
{
  // 需要更新的情况：
  // 1. 局部变换有修改(LOCAL_DIRTY)
  // 2. 世界变换需要更新(WORLD_DIRTY)
  // 3. 层次结构有变化(HIERARCHY_DIRTY)
  if (dirtyFlags & (LOCAL_DIRTY | WORLD_DIRTY | HIERARCHY_DIRTY)) {
    UpdateWorldMatrix(reg);
  }
  return m_WorldMatrix;
}

void TransformComponent::SetLocalMatrix(const glm::mat4 &matrix)
{
  // 分解矩阵到TRS组件
  glm::vec3 skew;
  glm::vec4 perspective;
  glm::decompose(matrix, m_Scale, m_Rotation, m_Position, skew, perspective);

  // 更新脏标记，需要重新计算世界变换
  dirtyFlags |= LOCAL_DIRTY;
  dirtyFlags |= HIERARCHY_DIRTY;
  MarkDirty();

  // 直接更新缓存矩阵（避免下次Get时重复计算）
  m_LocalMatrix = matrix;

  // 发布事件通知SceneGraph处理子节点更新
  EventBus::Get().Post(TransformChangedEvent(GetOwnerEntity(), *this, m_LocalMatrix, false));
}

void TransformComponent::SetWorldMatrix(SceneRegistry &reg, const glm::mat4 &matrix)
{
  if (reg.HasComponent<HierarchyComponent>(GetOwnerEntity())) {
    auto &hierarchy = reg.GetComponent<HierarchyComponent>(GetOwnerEntity());
    if (hierarchy.GetParent().IsValid()) {
      // 转换为父级局部空间
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
  return m_Rotation * glm::vec3(0.0f, 0.0f, -1.0f);
}

glm::vec3 TransformComponent::Up() const
{
  return m_Rotation * glm::vec3(0.0f, 1.0f, 0.0f);
}

glm::vec3 TransformComponent::Right() const
{
  return m_Rotation * glm::vec3(1.0f, 0.0f, 0.0f);
}

// 组件接口实现 ==========================================

std::vector<std::type_index> TransformComponent::GetDependencies() const
{
  return {typeid(HierarchyComponent)};
}

bool TransformComponent::Serialize(std::ostream &output) const
{
  Component::Serialize(output);  // 序列化基类数据

  //// 序列化位置
  // output.write(reinterpret_cast<const char *>(&m_Position), sizeof(m_Position));

  //// 序列化旋转
  // output.write(reinterpret_cast<const char *>(&m_Rotation), sizeof(m_Rotation));

  //// 序列化缩放
  // output.write(reinterpret_cast<const char *>(&m_Scale), sizeof(m_Scale));

  return !output.fail();
}

bool TransformComponent::Deserialize(std::istream &input)
{
  Component::Deserialize(input);  // 反序列化基类数据

  //// 反序列化位置
  // input.read(reinterpret_cast<char *>(&m_Position), sizeof(m_Position));

  //// 反序列化旋转
  // input.read(reinterpret_cast<char *>(&m_Rotation), sizeof(m_Rotation));

  //// 反序列化缩放
  // input.read(reinterpret_cast<char *>(&m_Scale), sizeof(m_Scale));

  //// 标记矩阵需要更新
  // m_LocalMatrixDirty = true;
  // m_WorldMatrixDirty = true;

  return !input.fail();
}

// 私有方法 ==============================================

void TransformComponent::UpdateLocalMatrix() const
{
  m_LocalMatrix = glm::mat4(1.0f);
  m_LocalMatrix = glm::translate(m_LocalMatrix, m_Position);
  m_LocalMatrix *= glm::mat4_cast(m_Rotation);
  m_LocalMatrix = glm::scale(m_LocalMatrix, m_Scale);

  dirtyFlags &= ~LOCAL_DIRTY;
}

void TransformComponent::UpdateWorldMatrix(SceneRegistry &reg) const
{
  // 在执行GetLocalMatrix()时，就已经清理了LOCAL_DIRTY标记
  const glm::mat4 localMat = GetLocalMatrix();

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

  // 清除计算相关脏标记（保留HIERARCHY_DIRTY用于子节点检测）
  dirtyFlags &= ~(LOCAL_DIRTY | WORLD_DIRTY);
}

void TransformSystem::ProcessDirtyComponents(float deltaTime, SceneRegistry &registry)
{
  // 获取所有对象
  auto view = registry.GetEntitiesWith<TransformComponent, HierarchyComponent>();

  // 第一阶段：处理根实体
  for (auto entity : view) {
    TransformComponent &transform = registry.GetComponent<TransformComponent>(entity);
    HierarchyComponent &hierarchy = registry.GetComponent<HierarchyComponent>(entity);
    // 筛选根实体
    if (!hierarchy.GetParent().IsValid() && (transform.IsDirty() || transform.dirtyFlags)) {
      transform.ProcessDirty(deltaTime, registry);
    }
  }

  // 第二阶段：处理子实体（保证父节点已处理）
  for (auto entity : view) {
    TransformComponent &transform = registry.GetComponent<TransformComponent>(entity);
    HierarchyComponent &hierarchy = registry.GetComponent<HierarchyComponent>(entity);
    // 筛选子实体
    if (hierarchy.GetParent().IsValid() && (transform.IsDirty() || transform.dirtyFlags)) {
      transform.ProcessDirty(deltaTime, registry);
    }
  }

  // TODO: 第三阶段：处理没有HierarchyComponent的独立实体
  // 需要使用参数包的完美转发，将entt::exclude<HierarchyComponent>
  // 作为参数传入GetEntitiesWith()，使m_Registry.view传入该参数：
  //
  // reg.view<TransformComponent>(entt::exclude<HierarchyComponent>)
}
};  // namespace mite