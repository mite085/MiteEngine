#include "transform_component.h"
#include "hierarchy_component.h"
#include "scene_core/entity.h"

namespace mite {
TransformComponent::TransformComponent() : ComponentTraits() {}

TransformComponent::TransformComponent(const glm::vec3 &position,
                                       const glm::quat &rotation,
                                       const glm::vec3 &scale)
    : ComponentTraits(), m_Position(position), m_Rotation(rotation), m_Scale(scale)
{
  m_LocalMatrixDirty = true;
  m_WorldMatrixDirty = true;
}

TransformComponent::TransformComponent( const glm::mat4 &matrix)
    : ComponentTraits()
{
  DecomposeMatrix(matrix);
}

void TransformComponent::ProcessDirty(SceneRegistry &reg)
{
  CalculateWorldMatrix(reg);
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
    m_LocalMatrixDirty = true;
    m_WorldMatrixDirty = true;
    MarkDirty();
  }
}

glm::vec3 TransformComponent::GetWorldPosition(SceneRegistry &reg) const
{
  const glm::mat4 &worldMat = GetWorldMatrix(reg);
  return glm::vec3(worldMat[3]);
}

void TransformComponent::SetWorldPosition(SceneRegistry &reg, const glm::vec3 &position)
{
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
  SetLocalPosition(position);
}

void TransformComponent::Translate(const glm::vec3 &translation)
{
  m_Position += translation;
  m_LocalMatrixDirty = true;
  m_WorldMatrixDirty = true;
  MarkDirty();
}

void TransformComponent::Translate(float x, float y, float z)
{
  Translate(glm::vec3{x, y, z});
}

// 旋转相关方法 ==============================================

const glm::quat &TransformComponent::GetLocalRotation() const
{
  return m_Rotation;
}

void TransformComponent::SetLocalRotation(const glm::quat &rotation)
{
  if (m_Rotation != rotation) {
    m_Rotation = rotation;
    m_LocalMatrixDirty = true;
    m_WorldMatrixDirty = true;
    MarkDirty();
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
  const glm::mat4 &worldMat = GetWorldMatrix(reg);
  glm::vec3 scale;
  glm::quat rotation;
  glm::vec3 translation;
  glm::vec3 skew;
  glm::vec4 perspective;
  glm::decompose(worldMat, scale, rotation, translation, skew, perspective);
  return rotation;
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
      SetLocalRotation(glm::inverse(parentWorldRot) * rotation);
      return;
    }
  }
  // 没有父节点，世界旋转就是局部旋转
  SetLocalRotation(rotation);
}

void TransformComponent::Rotate(const glm::quat &rotation)
{
  // 应用旋转（局部空间，右乘）
  m_Rotation = rotation * m_Rotation;

  // 标记矩阵需要更新
  m_LocalMatrixDirty = true;
  m_WorldMatrixDirty = true;
  MarkDirty();
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

  // 计算从旋转中心到实体的向量
  const glm::vec3 toObject = worldPos - point;

  // 创建旋转四元数
  const glm::vec3 normalizedAxis = glm::normalize(axis);
  const glm::quat rotation = glm::angleAxis(angle, normalizedAxis);

  // 旋转向量并计算新位置
  const glm::vec3 rotatedVec = rotation * toObject;
  const glm::vec3 newWorldPos = point + rotatedVec;

  // 更新世界位置（会自动处理父子关系）
  SetWorldPosition(reg, newWorldPos);

  // 同时应用旋转到实体朝向（世界空间）
  if (reg.HasComponent<HierarchyComponent>(GetOwnerEntity())) {
    auto &hierarchy = reg.GetComponent<HierarchyComponent>(GetOwnerEntity());
    if (hierarchy.GetParent().IsValid()) {
      // 如果有父节点，转换为局部旋转
      TransformComponent &parentTransform = reg.GetComponent<TransformComponent>(
          hierarchy.GetParent());
      glm::quat parentWorldRot = parentTransform.GetWorldRotation(reg);
      glm::quat localRot = glm::inverse(parentWorldRot) * rotation * parentWorldRot;
      Rotate(localRot);
    }
    else {
      Rotate(rotation);
    }
  }
  else {
    Rotate(rotation);
  }
}

void TransformComponent::LookAt(SceneRegistry &reg, const glm::vec3 &target, const glm::vec3 &up)
{
  const glm::vec3 position = GetWorldPosition(reg);
  const glm::mat4 lookAtMat = glm::lookAt(position, target, up);
  const glm::quat rotation = glm::quat_cast(glm::inverse(lookAtMat));

  if (reg.HasComponent<HierarchyComponent>(GetOwnerEntity())) {
    auto &hierarchy = reg.GetComponent<HierarchyComponent>(GetOwnerEntity());
    if (hierarchy.GetParent().IsValid()) {
      // 转换为局部旋转
      TransformComponent &parentTransform = reg.GetComponent<TransformComponent>(
          hierarchy.GetParent());
      glm::quat parentRotation = parentTransform.GetWorldRotation(reg);
      SetLocalRotation(glm::inverse(parentRotation) * rotation);
      return;
    }
  }

  SetLocalRotation(rotation);
}

// 缩放相关方法 ==============================================

const glm::vec3 &TransformComponent::GetLocalScale() const
{
  return m_Scale;
}

void TransformComponent::SetLocalScale(const glm::vec3 &scale)
{
  if (m_Scale != scale) {
    m_Scale = scale;
    m_LocalMatrixDirty = true;
    m_WorldMatrixDirty = true;
    MarkDirty();
  }
}

void TransformComponent::SetLocalScale(float scale)
{
  SetLocalScale(glm::vec3{scale, scale, scale});
}

glm::vec3 TransformComponent::GetWorldScale(SceneRegistry &reg) const
{
  const glm::mat4 &worldMat = GetWorldMatrix(reg);
  glm::vec3 scale;
  glm::quat rotation;
  glm::vec3 translation;
  glm::vec3 skew;
  glm::vec4 perspective;
  glm::decompose(worldMat, scale, rotation, translation, skew, perspective);
  return scale;
}

// 矩阵相关方法 ==============================================

glm::mat4 TransformComponent::GetLocalMatrix() const
{
  if (m_LocalMatrixDirty) {
    CalculateLocalMatrix();
  }
  return m_LocalMatrix;
}

glm::mat4 TransformComponent::GetWorldMatrix(SceneRegistry &reg) const
{
  if (m_WorldMatrixDirty) {
    CalculateWorldMatrix(reg);
  }
  return m_WorldMatrix;
}

void TransformComponent::SetLocalMatrix(const glm::mat4 &matrix)
{
  DecomposeMatrix(matrix);
}

void TransformComponent::SetWorldMatrix(SceneRegistry &reg, const glm::mat4 &matrix)
{
  if (reg.HasComponent<HierarchyComponent>(GetOwnerEntity())) {
    auto &hierarchy = reg.GetComponent<HierarchyComponent>(GetOwnerEntity());
    if (hierarchy.GetParent().IsValid()) {
      // 将世界矩阵转换为局部矩阵
      TransformComponent &parentTransform = reg.GetComponent<TransformComponent>(
          hierarchy.GetParent());
      glm::mat4 parentWorldMat = parentTransform.GetWorldMatrix(reg);
      glm::mat4 localMat = glm::inverse(parentWorldMat) * matrix;
      DecomposeMatrix(localMat);
      return;
    }
  }
  DecomposeMatrix(matrix);
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

  // 序列化位置
  output.write(reinterpret_cast<const char *>(&m_Position), sizeof(m_Position));

  // 序列化旋转
  output.write(reinterpret_cast<const char *>(&m_Rotation), sizeof(m_Rotation));

  // 序列化缩放
  output.write(reinterpret_cast<const char *>(&m_Scale), sizeof(m_Scale));

  return !output.fail();
}

bool TransformComponent::Deserialize(std::istream &input)
{
  Component::Deserialize(input);  // 反序列化基类数据

  // 反序列化位置
  input.read(reinterpret_cast<char *>(&m_Position), sizeof(m_Position));

  // 反序列化旋转
  input.read(reinterpret_cast<char *>(&m_Rotation), sizeof(m_Rotation));

  // 反序列化缩放
  input.read(reinterpret_cast<char *>(&m_Scale), sizeof(m_Scale));

  // 标记矩阵需要更新
  m_LocalMatrixDirty = true;
  m_WorldMatrixDirty = true;

  return !input.fail();
}

// 私有方法 ==============================================

void TransformComponent::CalculateLocalMatrix() const {
  m_LocalMatrix = glm::translate(glm::mat4(1.0f), m_Position) * glm::mat4_cast(m_Rotation) *
                  glm::scale(glm::mat4(1.0f), m_Scale);
  m_LocalMatrixDirty = false;
}

void TransformComponent::CalculateWorldMatrix(SceneRegistry &reg) const
{
  // 在执行GetLocalMatrix()时，就已经清理了m_LocalMatrixDirty标记
  const glm::mat4 localMat = GetLocalMatrix();

  if (reg.HasComponent<HierarchyComponent>(GetOwnerEntity())) {
    auto &hierarchy = reg.GetComponent<HierarchyComponent>(GetOwnerEntity());
    if (hierarchy.GetParent().IsValid()) {
      // 如果有父节点，计算世界矩阵
      // 
      // 注意：
      // SceneGraph::UpdateWorldTransformsAndVisibility的
      // 深度优先遍历算法，可以确保此时parentTransform的
      // m_WorldMatrixDirty脏标记已经被处理，不会出现处理
      // 子节点的transform时，需要同步处理父节点的问题。
      // 
      // 但如果真的出现了，GetWorldMatrix()也会检查
      // parentTransform的脏标记并更新其transform
      TransformComponent &parentTransform = reg.GetComponent<TransformComponent>(
          hierarchy.GetParent());
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

  m_WorldMatrixDirty = false;
}

void TransformComponent::DecomposeMatrix(const glm::mat4 &matrix)
{
  glm::vec3 skew;
  glm::vec4 perspective;
  glm::decompose(matrix, m_Scale, m_Rotation, m_Position, skew, perspective);

  // 正交化旋转
  m_Rotation = glm::normalize(m_Rotation);

  m_LocalMatrixDirty = true;
  m_WorldMatrixDirty = true;
  MarkDirty();
}

void TransformSystem::ProcessDirtyComponents(float deltaTime, SceneRegistry &registry)
{
  // 并行处理所有根节点。
  // 其他叶节点需要串行处理，
  // 交由SceneGraph的UpdateWorldTransformsAndVisibility执行
  // TODO: Transform的dirty flag在向上传递时存在问题
  std::for_each(std::execution::par,
                m_DirtyComponents.begin(),
                m_DirtyComponents.end(),
                [&](TransformComponent *tf) {
                  if (!tf->HasParent(registry)) {
                    tf->Update(registry);
                  }
                });
}

};  // namespace mite