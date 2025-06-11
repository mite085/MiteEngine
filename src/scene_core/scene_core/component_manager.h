#ifndef MITE_SCENE_COMPONENT_MANAGER
#define MITE_SCENE_COMPONENT_MANAGER

#include "component.h"
#include "component_type.h"
#include "entity.h"

namespace mite {
// ǰ������
class IComponentPool;

/**
 * @class ComponentManager
 * @brief ���й�������ʵ������ĺ��Ĺ�����
 *
 * ְ��
 * 1. ��������֯����洢
 * 2. �ṩ���Ͱ�ȫ���������
 * 3. ���������������
 * 4. ֧�ָ�Ч�Ķ��������
 */
class ComponentManager {
 public:
  ComponentManager();
  ~ComponentManager();

  // ���ÿ���
  ComponentManager(const ComponentManager &) = delete;
  ComponentManager &operator=(const ComponentManager &) = delete;

  /**
   * @brief ��ʵ��������
   * @tparam T �������
   * @param entity Ŀ��ʵ��
   * @return �����������
   */
  template<typename T> T &AddComponent(Entity entity);

  /**
   * @brief �Ƴ�ʵ���ָ���������
   * @tparam T �������
   * @param entity Ŀ��ʵ��
   */
  template<typename T> void RemoveComponent(Entity entity);

  /**
   * @brief ���ʵ���Ƿ�ӵ��ָ���������
   * @tparam T �������
   * @param entity Ŀ��ʵ��
   * @return ������ڷ���true
   */
  template<typename T> bool HasComponent(Entity entity) const;

  /**
   * @brief ��ȡʵ���ָ���������
   * @tparam T �������
   * @param entity Ŀ��ʵ��
   * @return �������
   * @throws std::runtime_error ������������
   */
  template<typename T> T &GetComponent(Entity entity);

  /**
   * @brief ��ȡʵ���ָ���������(�����汾)
   */
  template<typename T> const T &GetComponent(Entity entity) const;

  /**
   * @brief ʵ������ʱ���ã�����������
   * @param entity �����ٵ�ʵ��
   */
  void OnEntityDestroyed(Entity entity);

  /**
   * @brief ��ȡ����ע����������
   * @return ������ͼ���
   */
  std::vector<ComponentType> GetRegisteredComponentTypes() const;

  /**
   * @brief ��ȡָ�����������ʵ����ͼ
   * @tparam T �������
   * @return �����������ʵ���б�
   */
  template<typename T> std::vector<Entity> GetEntitiesWith() const;

 private:
  /**
   * @brief ��ȡ�򴴽��ض����͵������
   * @tparam T �������
   * @param type �������������
   * @return ���������
   */
  template<typename T> IComponentPool &GetComponentPool(ComponentType type);

  // ����ش洢 (����->��)
  std::unordered_map<ComponentType, std::unique_ptr<IComponentPool>> m_ComponentPools;

  // ʵ�嵽������͵�ӳ�� (ʵ��->���ͼ���)
  std::unordered_map<Entity, std::vector<ComponentType>> m_EntityComponentMap;
};
}  // namespace mite

#endif