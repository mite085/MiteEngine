#ifndef MITE_SCENE_COMPONENT_POOL
#define MITE_SCENE_COMPONENT_POOL

#include "entity.h"

namespace mite {
/**
 * @class IComponentPool
 * @brief ����صĳ�����ӿ�
 *
 * �ṩ����������������͵�ͨ�ò����ӿڣ�
 * ����ComponentManager��ͳһ��ʽ�����������͵������
 */
class IComponentPool {
 public:
  virtual ~IComponentPool() = default;

  /**
   * @brief �Ƴ�ָ��ʵ������
   * @param entity Ŀ��ʵ��
   */
  virtual void Remove(Entity entity) = 0;

  /**
   * @brief ���ʵ���Ƿ�ӵ�����
   * @param entity Ŀ��ʵ��
   * @return ������ڷ���true
   */
  virtual bool Has(Entity entity) const = 0;

  /**
   * @brief ��ȡ����ӵ�и������ʵ���б�
   * @return ʵ���б�
   */
  virtual std::vector<Entity> GetEntities() const = 0;

  /**
   * @brief ��ȡ�������
   * @return ��ǰ�洢���������
   */
  virtual size_t Size() const = 0;
};

/**
 * @class ComponentPool
 * @brief �ض���������ľ���洢ʵ��
 * @tparam T ������ͣ�����̳���Component
 *
 * ����SOA(Structure of Arrays)�洢ģʽ�Ż�����Ч�ʣ�
 * 1. ʵ��ID��������ݷֿ��洢
 * 2. ��ͬ������ʵ��������Ӧ
 * 3. ʹ�ù�ϣ�����ʵ�����
 */
template<typename T> class ComponentPool : public IComponentPool {
 public:
  /**
   * @brief ��������ʵ��
   * @param entity Ŀ��ʵ��
   * @return �����������
   * @note ���ʵ�����и�������׳��쳣
   */
  T &Add(Entity entity);

  /**
   * @brief �Ƴ�ʵ������(override)
   */
  void Remove(Entity entity) override;

  /**
   * @brief ���ʵ���Ƿ�ӵ�����(override)
   */
  bool Has(Entity entity) const override;

  /**
   * @brief ��ȡʵ������
   * @param entity Ŀ��ʵ��
   * @return �������
   * @throws std::out_of_range ������������
   */
  T &Get(Entity entity);

  /**
   * @brief ��ȡʵ������(�����汾)
   */
  const T &Get(Entity entity) const;

  /**
   * @brief ��ȡ����ʵ���б�(override)
   */
  std::vector<Entity> GetEntities() const override;

  /**
   * @brief ��ȡ�������(override)
   */
  size_t Size() const override;

 private:
  // Structure of Arrays (SOA)�洢��ʽ
  std::vector<Entity> m_Entities;	// ʵ���б�(��m_Componentsһһ��Ӧ)
  std::vector<T> m_Components;		// ������ݴ洢(�����ڴ�)

  // ʵ�嵽�����Ŀ���ӳ��
  std::unordered_map<Entity, size_t> m_EntityToIndex;
};
};

#endif
