#ifndef MITE_SCENE_COMPONENT
#define MITE_SCENE_COMPONENT

#include "headers/headers.h"

namespace mite {

class Entity;   // ǰ������

/**
 * @class Component
 * @brief ��������Ļ��࣬�ṩͨ�ýӿںͻ�������
 *
 * ����Ǵ�������������Ӧ����ҵ���߼���ÿ�����������һ��ʵ�壬
 * ��ͨ�����ͱ�ʶ�������֡����������������������ʵ�����
 */
class Component {
 public:
  // ʹ������ָ������򻯴���
  using Ptr = std::shared_ptr<Component>;
  using WeakPtr = std::weak_ptr<Component>;

  /**
   * @brief ����������ȷ����������ȷ����
   */
  virtual ~Component() = default;

  // ��ֹ��������͸�ֵ�����Ӧͨ��Clone()��������
  Component(const Component &) = delete;
  Component &operator=(const Component &) = delete;

  /**
   * @brief ��ȡ�������ID����������ʱ����ʶ��
   * @return ������͵�Ψһ��ʶ��
   */
  virtual uint32_t GetTypeId() const = 0;

  /**
   * @brief ��ȡ����������ƣ����ڵ��Ժ����л�
   * @return ������͵Ŀɶ�����
   */
  virtual const std::string &GetTypeName() const = 0;

  /**
   * @brief ��ȡӵ�д������ʵ��
   * @return ӵ�д������ʵ�������ã�����Ϊ��
   */
  Entity *GetOwner() const
  {
    return m_Owner;
  }

  /**
   * @brief �������������ʵ�壨����Entity���ڲ�ʹ�ã�
   * @param owner ӵ�д��������ʵ��
   */
  void SetOwner(Entity *owner)
  {
    m_Owner = owner;
  }

  /**
   * @brief �������Ƿ��Ѹ��ӵ�ʵ��
   * @return �������Ѹ��ӵ�ʵ���򷵻�true
   */
  bool IsAttached() const
  {
    return m_Owner != nullptr;
  }

  /**
   * @brief ������������
   * @return �������ʵ��������Ȩ�ɵ����߳���
   */
  virtual Ptr Clone() const = 0;

  /**
   * @brief ���л�������ݵ������
   * @param output Ŀ�������
   * @return ���л��Ƿ�ɹ�
   */
  virtual bool Serialize(std::ostream &output) const = 0;

  /**
   * @brief �������������л��������
   * @param input Դ������
   * @return �����л��Ƿ�ɹ�
   */
  virtual bool Deserialize(std::istream &input) = 0;

  /**
   * @brief ���������ӵ�ʵ��ʱ����
   */
  virtual void OnAttached() {}

  /**
   * @brief �������ʵ���Ƴ�ʱ����
   */
  virtual void OnDetached() {}

  /**
   * @brief �����������ʱ���ã�ʵ�弤�������������
   */
  virtual void OnEnabled() {}

  /**
   * @brief �����������ʱ���ã�ʵ����û�����������ã�
   */
  virtual void OnDisabled() {}

 protected:
  // �������캯������ֱֹ��ʵ��������
  Component() = default;

  // �����ƶ�����
  Component(Component &&) = default;
  Component &operator=(Component &&) = default;

 private:
  Entity *m_Owner = nullptr;  // ӵ�д������ʵ�壬��ӵ������Ȩ
};
}  // namespace mite

#endif