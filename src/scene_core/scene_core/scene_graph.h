#ifndef MITE_SCENE_GRAPH
#define MITE_SCENE_GRAPH

#include "headers/headers.h"
#include <entt/entt.hpp>

namespace mite {
/**
 * @class SceneGraph
 * @brief ��������ʵ��Ĳ�νṹ��ϵ���ṩ��Ч�ı����Ͳ�ѯ����
 *
 * ����EnTT ECS��HierarchyComponent������֧�֣�
 * - ��νṹ��ά������֤
 * - ���ֱ�����ʽ(DFS/BFS)
 * - �ռ�任�̳�
 * - �ɼ��Լ̳�
 * - ����ͼ�¼�֪ͨ
 */
class SceneGraph {
 public:
  // ����˳��ö��
  enum class TraversalOrder {
    DepthFirst,         // �������
    BreadthFirst,       // �������
    ReverseDepthFirst,  // ���������(��Ҷ�ӵ���)
  };

  // �����ص���������
  using VisitorFunc = std::function<bool(entt::entity)>;

  /**
   * @brief ���캯��
   * @param registry EnTTע�������
   */
  explicit SceneGraph(entt::registry &registry) : m_Registry(registry) {}

  // ��ֹ����
  SceneGraph(const SceneGraph &) = delete;
  SceneGraph &operator=(const SceneGraph &) = delete;

  /**
   * @brief ����ʵ�常�ڵ�
   * @param entity Ŀ��ʵ��
   * @param newParent �¸�ʵ��(entt::null��ʾ��Ϊ���ڵ�)
   * @return �Ƿ����óɹ�
   *
   * @note �����ѭ��������飬����γ�ѭ�������ʧ��
   */
  bool SetParent(entt::entity entity, entt::entity newParent);

  /**
   * @brief ��ȡʵ�常�ڵ�
   * @param entity Ŀ��ʵ��
   * @return ��ʵ����(entt::null��ʾ�޸��ڵ�)
   */
  entt::entity GetParent(entt::entity entity) const;

  /**
   * @brief ��ȡʵ���ӽڵ��б�
   * @param entity Ŀ��ʵ��
   * @return ��ʵ���б�(�����˳��)
   */
  const std::vector<entt::entity> &GetChildren(entt::entity entity) const;

  /**
   * @brief ���ʵ���Ƿ�Ϊ���ڵ�
   * @param entity Ŀ��ʵ��
   */
  bool IsRoot(entt::entity entity) const;

  /**
   * @brief ���ʵ���Ƿ�ΪҶ�ڵ�
   * @param entity Ŀ��ʵ��
   */
  bool IsLeaf(entt::entity entity) const;

  /**
   * @brief ��ȡʵ���ڳ���ͼ�е����
   * @param entity Ŀ��ʵ��
   * @return ���ֵ(���ڵ�Ϊ0)
   */
  size_t GetDepth(entt::entity entity) const;

  /**
   * @brief ��������ͼ
   * @param root ��ʼʵ��
   * @param visitor �����߻ص�����
   * @param order ����˳��
   *
   * @note ���visitor����false����ֹͣ����
   */
  void Traverse(entt::entity root,
                const VisitorFunc &visitor,
                TraversalOrder order = TraversalOrder::DepthFirst) const;

  /**
   * @brief ������������ͼ(�����и��ڵ㿪ʼ)
   * @param visitor �����߻ص�����
   * @param order ����˳��
   */
  void TraverseAll(const VisitorFunc &visitor,
                   TraversalOrder order = TraversalOrder::DepthFirst) const;

  /**
   * @brief ��ȡ��ʵ�嵽���ڵ��·��
   * @param entity ��ʼʵ��
   * @return ·��ʵ���б�(��ʵ�嵽���ڵ�˳��)
   */
  std::vector<entt::entity> GetPathToRoot(entt::entity entity) const;

  /**
   * @brief �������ʵ���Ƿ���ͬһ����νṹ��
   * @param entity1 ʵ��1
   * @param entity2 ʵ��2
   */
  bool IsInSameHierarchy(entt::entity entity1, entt::entity entity2) const;

  /**
   * @brief ��ȡ���������и�ʵ��
   * @return ��ʵ���б�
   */
  std::vector<entt::entity> GetRoots() const;

  /**
   * @brief ���¼��㲢��������ʵ������ֵ
   *
   * @note ͨ���������޸Ĳ�νṹ���������ߺ�����ѯ����
   */
  void RecalculateAllDepths();

  /**
   * @brief ÿ֡���³���ͼ״̬
   * @param timestep ֡ʱ����(��)
   *
   * ��Ҫ����:
   * 1. �����任����(�����ǵĸ��ڵ㵽�ӽڵ�)
   * 2. ���¿ɼ���״̬
   * 3. ά��������Ҫÿ֡���µĳ���ͼ״̬
   * 4. �����ӳٵĲ�νṹ���
   */
  void SceneGraph::OnUpdate(float timestep);

 private:
  /**
   * @brief �ڲ����� - ����Ƿ��γ�ѭ������
   * @param child ��ʵ��
   * @param newParent �¸�ʵ��
   */
  bool WouldFormCycle(entt::entity child, entt::entity newParent) const;

  /**
   * @brief �ڲ����� - ������ȱ���ʵ��
   * @param entity ��ǰʵ��
   * @param visitor �����߻ص�
   * @return �Ƿ��������
   */
  bool TraverseDFS(entt::entity entity, const VisitorFunc &visitor) const;

  /**
   * @brief �ڲ����� - ������ȱ���ʵ��
   * @param entity ��ʼʵ��
   * @param visitor �����߻ص�
   */
  void TraverseBFS(entt::entity entity, const VisitorFunc &visitor) const;

  /**
   * @brief �ڲ����� - ��������ȱ���ʵ��
   * @param entity ��ǰʵ��
   * @param visitor �����߻ص�
   * @return �Ƿ��������
   */
  bool TraverseReverseDFS(entt::entity entity, const VisitorFunc &visitor) const;

  // EnTTע�������
  entt::registry &m_Registry;
};
};  // namespace mite

#endif
