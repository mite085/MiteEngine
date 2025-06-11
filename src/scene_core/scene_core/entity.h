#ifndef MITE_SCENE_ENTITY
#define MITE_SCENE_ENTITY

#include "headers/headers.h"

namespace mite {

class Scene;      // ǰ������
class Component;  // ǰ������

using EntityID = uint32_t;

/**
 * @class Entity
 * @brief ��ʾ�����е�һ��ʵ�壬��Ϊ���������
 *
 * ʵ�屾�������߼�������Ϊ����ļ��ϱ�ʶ����
 * ���й���ͨ�����ӵ����ʵ�֡�
 */
class Entity {
 public:
  /**
   * @brief ���캯��
   * @param scene ����������ָ��
   * @param id ʵ��ΨһID
   */
  Entity(Scene *scene, EntityID id);

  ~Entity();

  // ���ÿ�������͸�ֵ
  Entity(const Entity &) = delete;
  Entity &operator=(const Entity &) = delete;

  // �����ƶ�����
  Entity(Entity &&) = default;
  Entity &operator=(Entity &&) = default;

  /**
   * @brief ��ȡʵ��ID
   * @return ʵ��ΨһID
   */
  EntityID GetID() const;

  /**
   * @brief ��ȡ��������
   * @return ָ������������ָ��
   */
  Scene *GetScene() const;

  /**
   * @brief ���ʵ���Ƿ��Ծ
   * @return true���ʵ���Ծ��false����
   */
  bool IsActive() const;

  /**
   * @brief ����ʵ���Ծ״̬
   * @param active Ҫ���õ�״̬
   */
  void SetActive(bool active);

  /**
   * @brief ��ȡʵ������
   * @return ʵ�������ַ���
   */
  const std::string &GetName() const;

  /**
   * @brief ����ʵ������
   * @param name ������
   */
  void SetName(const std::string &name);

  // ==================== ������� ====================

  /**
   * @brief ��������ʵ��
   * @tparam T �������
   * @tparam Args ���캯����������
   * @param args ������캯������
   * @return ָ���������ָ��
   */
  template<typename T, typename... Args> T *AddComponent(Args &&...args);

  /**
   * @brief ��ʵ���Ƴ����
   * @tparam T �������
   */
  template<typename T> void RemoveComponent();

  /**
   * @brief ���ʵ���Ƿ����ض����͵����
   * @tparam T �������
   * @return true���ʵ���и������false����
   */
  template<typename T> bool HasComponent() const;

  /**
   * @brief ��ȡʵ���ϵ����
   * @tparam T �������
   * @return ָ�������ָ�룬���û���򷵻�nullptr
   */
  template<typename T> T *GetComponent();

  /**
   * @brief ��ȡʵ���ϵ����(�����汾)
   * @tparam T �������
   * @return ָ������ĳ���ָ�룬���û���򷵻�nullptr
   */
  template<typename T> const T *GetComponent() const;

  /**
   * @brief ��ȡʵ���������
   * @return ������͵����ָ���ӳ��
   */
  const std::unordered_map<std::type_index, std::unique_ptr<Component>> &GetAllComponents() const
  {
    return m_Components;
  }

  // ==================== ���ӹ�ϵ���� ====================

  /**
   * @brief ���ø�ʵ��
   * @param parent Ҫ����Ϊ��ʵ���ָ��
   */
  void SetParent(Entity *parent);

  /**
   * @brief ��ȡ��ʵ��
   * @return ��ʵ��ָ�룬���û���򷵻�nullptr
   */
  Entity *GetParent() const
  {
    return m_Parent;
  }

  /**
   * @brief ��ȡ������ʵ��
   * @return ��ʵ��ָ�������
   */
  const std::vector<Entity *> &GetChildren() const
  {
    return m_Children;
  }

  /**
   * @brief �����ʵ��
   * @param child Ҫ��ӵ���ʵ��ָ��
   */
  void AddChild(Entity *child);

  /**
   * @brief �Ƴ���ʵ��
   * @param child Ҫ�Ƴ�����ʵ��ָ��
   */
  void RemoveChild(Entity *child);

  /**
   * @brief ����Ƿ�Ϊ��һ��ʵ�������
   * @param entity Ҫ����ʵ��
   * @return true��������ȣ�false����
   */
  bool IsAncestorOf(const Entity *entity) const;

  /**
   * @brief ����Ƿ�Ϊ��һ��ʵ��ĺ��
   * @param entity Ҫ����ʵ��
   * @return true����Ǻ����false����
   */
  bool IsDescendantOf(const Entity *entity) const;

 private:
  Scene *m_Scene;      // ��������
  EntityID m_ID;       // Ψһ��ʶ��
  bool m_IsActive;     // ��Ծ״̬
  std::string m_Name;  // ʵ������

  // ����洢
  std::unordered_map<std::type_index, std::unique_ptr<Component>> m_Components;

  // �㼶��ϵ
  Entity *m_Parent = nullptr;        // ��ʵ��
  std::vector<Entity *> m_Children;  // ��ʵ���б�
};

}  // namespace mite

#endif