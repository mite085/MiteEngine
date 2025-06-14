#ifndef MITE_SCENE_ENTITY
#define MITE_SCENE_ENTITY

#include "component.h"

namespace mite {

// ǰ������
class Scene;

/**
 * @brief ʵ���࣬��ʾ�����е�һ����������
 *
 * ��װEnTTʵ�壬�ṩ���Ͱ�ȫ����������Ͳ�νṹ����
 * ע�⣺Entity�������������ģ��������ɸ��ƺʹ���
 */
class Entity {
 public:
  // ���캯�� ===================================================

  /**
   * @brief Ĭ�Ϲ���һ����ʵ�壨��Чʵ�壩
   */
  Entity() = default;

  /**
   * @brief �ӳ�����EnTTʵ�幹��
   * @param scene ����������������
   * @param handle �ײ�EnTTʵ����
   */
  Entity(std::weak_ptr<Scene> scene, entt::entity handle);

  /**
   * @brief Ĭ����������
   */
  ~Entity() = default;

  // ������� ===================================================

  /**
   * @brief �������������������
   * @tparam T �������
   * @tparam Args �����������
   * @param args ����������
   * @return ����ӵ��������
   */
  template<typename T, typename... Args> T &AddComponent(Args &&...args);

  /**
   * @brief �Ƴ����
   * @tparam T �������
   */
  template<typename T> void RemoveComponent();

  /**
   * @brief ����Ƿ�ӵ��ĳ�����
   * @tparam T �������
   * @return �Ƿ�ӵ�и����
   */
  template<typename T> bool HasComponent() const;

  /**
   * @brief ��ȡ�������const�汾��
   * @tparam T �������
   * @return �������
   * @throws std::runtime_error �����������ʱ�׳��쳣
   */
  template<typename T> T &GetComponent();

  /**
   * @brief ��ȡ�����const�汾��
   * @tparam T �������
   * @return ���const����
   * @throws std::runtime_error �����������ʱ�׳��쳣
   */
  template<typename T> const T &GetComponent() const;

  /**
   * @brief ���Ի�ȡ�������const�汾��
   * @tparam T �������
   * @return ���ָ�루������ʱ����nullptr��
   */
  template<typename T> T *TryGetComponent();

  /**
   * @brief ���Ի�ȡ�����const�汾��
   * @tparam T �������
   * @return ���constָ�루������ʱ����nullptr��
   */
  template<typename T> const T *TryGetComponent() const;

  // ��νṹ���� ================================================

  /**
   * @brief ���ø�ʵ��
   * @param parent ��ʵ��
   * @param keepWorldTransform �Ƿ񱣳�����ռ�任
   */
  void SetParent(Entity parent, bool keepWorldTransform = true);

  /**
   * @brief ��ȡ��ʵ��
   * @return ��ʵ�壨��Чʵ���ʾû�и�����
   */
  Entity GetParent() const;

  /**
   * @brief ��ȡ������ʵ��
   * @return ��ʵ���б�������˳��
   */
  const std::vector<Entity> &GetChildren() const;

  /**
   * @brief �����ʵ��
   * @param child ��ʵ��
   * @param keepWorldTransform �Ƿ񱣳�����ռ�任
   */
  void AddChild(Entity child, bool keepWorldTransform = true);

  /**
   * @brief �Ƴ���ʵ��
   * @param child ��ʵ��
   * @param keepWorldTransform �Ƿ񱣳�����ռ�任
   */
  void RemoveChild(Entity child, bool keepWorldTransform = true);

  /**
   * @brief ����Ƿ�Ϊĳ��ʵ�������
   * @param potentialAncestor ���ܵ�����ʵ��
   * @return �Ƿ�������
   */
  bool IsDescendantOf(Entity potentialAncestor) const;

  /**
   * @brief ��ȡʵ��㼶
   * @return ��ǰʵ��Ĳ���
   */
  size_t GetDepth() const;

  // ʵ��״̬ ===================================================

  /**
   * @brief ��ȡʵ��ID���־û���ʶ��
   * @return UUID�ַ���
   */
  const std::string &GetID() const;

  /**
   * @brief ���ʵ���Ƿ���Ч
   * @return �Ƿ���Ч��δ�����٣�
   */
  bool IsValid() const;

  /**
   * @brief ���ٴ�ʵ�壨����������ʵ�壩
   */
  void Destroy();

  /**
   * @brief ��ȡ�ײ�EnTTʵ����
   */
  entt::entity GetHandle() const;

  /**
   * @brief ��ȡ��������
   * @return ��������ָ�루����Ϊ�գ�
   */
  std::shared_ptr<Scene> GetScene() const;

  // ���������� =================================================

  /**
   * @brief �Ƚ�����ʵ���Ƿ���ͬ
   */
  bool operator==(const Entity &other) const;
  bool operator!=(const Entity &other) const;

  /**
   * @brief ת��Ϊbool��ʾʵ���Ƿ���Ч
   */
  explicit operator bool() const;

 private:
  // �ڲ����� ===================================================
  void UpdateChildParentRelationship(Entity child, bool keepWorldTransform);
  void RemoveFromParent();

 private:
  std::weak_ptr<Scene> m_Scene;       // ���������������ã�����ѭ�����ã�
  entt::entity m_Handle{entt::null};  // �ײ�EnTTʵ����
};

};  // namespace mite

// ��ϣ֧�֣����ڽ�Entity����unordered_map��key
namespace std {
template<> struct hash<mite::Entity> {
  size_t operator()(const mite::Entity &entity) const
  {
    return hash<entt::entity>()(entity.GetHandle());
  }
};
}  // namespace std

#endif
