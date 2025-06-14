#ifndef MITE_SCENE_COMPONENT
#define MITE_SCENE_COMPONENT

#include "headers/headers.h"
#include <entt/entt.hpp>

namespace mite {
// ǰ������
class Entity;

/**
 * @brief ������࣬���г��������Ӧ�̳��Դ���
 *
 * ���ԭ��
 * 1. ������ - ���Ӧ�Ǽ򵥵���������
 * 2. �����л� - ֧�ֱ���ͼ���
 * 3. ���Ͱ�ȫ - �ṩ����ʱ������Ϣ
 * 4. �ɿ�¡ - ֧�����
 * 5. �ɹ۲� - ֧�ֱ��֪ͨ
 */
class Component {
 public:
  // ����������ͱ�ʶ�������������
  enum class Family : uint8_t {
    Core = 0,     // �������(Transform��)
    Render,       // ��Ⱦ������
    Geometry,     // ����������
    Logic,        // �߼�/��Ϊ���
    Custom = 100  // �Զ��������ʼֵ
  };

  virtual ~Component() = default;

  /**
   * @brief ��ȡ������ͼ���
   */
  virtual Family GetFamily() const = 0;

  /**
   * @brief ��ȡ�������ΨһID
   */
  virtual std::type_index GetType() const = 0;

  /**
   * @brief ��¡���(���)
   * @return �����ʵ���Ĺ���ָ��
   */
  virtual std::shared_ptr<Component> Clone() const = 0;

  /**
   * @brief ������Ϊ���޸�
   * @param dirty �Ƿ�Ϊ������
   */
  void SetDirty(bool dirty = true);

  /**
   * @brief �������Ƿ��޸Ĺ�
   */
  bool IsDirty() const;

  /**
   * @brief �������״̬
   */
  void SetEnabled(bool enabled);
  bool IsEnabled() const;

  /**
   * @brief ���л��������
   * @param output �����
   * @return �Ƿ�ɹ�
   */
  virtual bool Serialize(std::ostream &output) const;

  /**
   * @brief �����л��������
   * @param input ������
   * @return �Ƿ�ɹ�
   */
  virtual bool Deserialize(std::istream &input);

  /**
   * @brief ��������Ƿ��������������
   * @return ��������������б�
   */
  virtual std::vector<std::type_index> GetDependencies() const
  {
    return {};
  }

 protected:
  // �������캯����ȷ��ֻ��ͨ������ʵ������
  // �����ն�Entity�������ã�ȷ��֧�����ϲ�ѯ
  explicit Component(std::weak_ptr<Entity> owner);

  std::weak_ptr<Entity> GetOwner() const
  {
    return m_OwnerEntity;
  }

 private:
  std::weak_ptr<Entity> m_OwnerEntity;

  bool m_Dirty = false;   // ���ǣ���ʶ����Ƿ��޸�
  bool m_Enabled = true;  // ����Ƿ�����
};

/**
 * @brief �����������ģ�壬���ڼ��������
 * @tparam T �������
 * @tparam F �������
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

  // ���þ�̬���ͼ������ID��ȡ
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
