#ifndef MITE_SCENE_TRANSFORM_COMPONENT
#define MITE_SCENE_TRANSFORM_COMPONENT

#include "scene_core/component.h"

namespace mite {
/**
 * @brief �任���������ʵ���λ�á���ת������
 *
 * �������ԣ�
 * 1. ֧�־ֲ��ռ������ռ�任
 * 2. ��Ч�ľ��󻺴�͸��»���
 * 3. �ṩ���õı任�����ӿ�
 * 4. ��HierarchyComponentЭͬ���������νṹ
 *
 * ��ƿ��ǣ�
 * - ʹ����������ϵ��Y������
 * - ��תʹ����Ԫ���洢�����������
 * - �ṩ����ϵͳ�Ż��������
 */
class TransformComponent : public ComponentTraits<TransformComponent, Component::Family::Core> {
 public:
  // Ĭ�Ϲ��캯��
  TransformComponent(std::weak_ptr<Entity> owner);

  // ����ʼֵ�Ĺ��캯��
  explicit TransformComponent(std::weak_ptr<Entity> owner,
                              const glm::vec3 &position,
                              const glm::quat &rotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f),
                              const glm::vec3 &scale = glm::vec3(1.0f));

  // �ӱ任������
  explicit TransformComponent(std::weak_ptr<Entity> owner, const glm::mat4 &matrix);

  ~TransformComponent() override = default;

  // λ�ò��� ==============================================
  const glm::vec3 &GetLocalPosition() const
  {
    return m_Position;
  }
  void SetLocalPosition(const glm::vec3 &position);

  // ��ȡ����ռ�λ��(��Ҫ���任����)
  glm::vec3 GetWorldPosition() const;
  void SetWorldPosition(const glm::vec3 &position);

  // ����ƶ�
  void Translate(const glm::vec3 &translation);
  void Translate(float x, float y, float z);

  // ��ת���� ==============================================
  const glm::quat &GetLocalRotation() const
  {
    return m_Rotation;
  }
  void SetLocalRotation(const glm::quat &rotation);

  // ŷ���ǽӿ�(������)
  glm::vec3 GetLocalEulerAngles() const;
  void SetLocalEulerAngles(const glm::vec3 &eulerAngles);
  void SetLocalEulerAngles(float x, float y, float z);

  // ����ռ���ת
  glm::quat GetWorldRotation() const;
  void SetWorldRotation(const glm::quat &rotation);

  // ��ת����
  void Rotate(const glm::quat &rotation);
  void Rotate(const glm::vec3 &axis, float angle);
  void RotateAround(const glm::vec3 &point, const glm::vec3 &axis, float angle);
  void LookAt(const glm::vec3 &target, const glm::vec3 &up = glm::vec3(0.0f, 1.0f, 0.0f));

  // ���Ų��� ==============================================
  const glm::vec3 &GetLocalScale() const
  {
    return m_Scale;
  }
  void SetLocalScale(const glm::vec3 &scale);
  void SetLocalScale(float scale);

  // ����ռ�����(���Ƽ���)
  glm::vec3 GetWorldScale() const;

  // �任������� ==========================================
  glm::mat4 GetLocalMatrix() const;
  glm::mat4 GetWorldMatrix() const;

  void SetLocalMatrix(const glm::mat4 &matrix);
  void SetWorldMatrix(const glm::mat4 &matrix);

  // �������� ==============================================
  glm::vec3 Forward() const;  // ��Z�᷽��
  glm::vec3 Up() const;       // ��Y�᷽��
  glm::vec3 Right() const;    // ��X�᷽��

  // ����ӿ�ʵ�� ==========================================
  std::vector<std::type_index> GetDependencies() const override;
  bool Serialize(std::ostream &output) const override;
  bool Deserialize(std::istream &input) override;

  // �任���±�־
  void UpdateTransform();

 private:
  // ��������任����(�ݹ�)
  void CalculateWorldMatrix() const;

  // �Ӿ���ֽ�任
  void DecomposeMatrix(const glm::mat4 &matrix);

 private:
  // �ֲ��ռ�任����
  glm::vec3 m_Position = glm::vec3(0.0f);
  glm::quat m_Rotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
  glm::vec3 m_Scale = glm::vec3(1.0f);

  // ���󻺴�
  mutable glm::mat4 m_LocalMatrix = glm::mat4(1.0f);
  mutable glm::mat4 m_WorldMatrix = glm::mat4(1.0f);

  // ����
  mutable bool m_LocalMatrixDirty = true;
  mutable bool m_WorldMatrixDirty = true;
};
};

#endif
