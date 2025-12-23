#ifndef MITE_SCENE_NODE_H
#define MITE_SCENE_NODE_H

#include "basic_data/bounding_volume.h"
#include "basic_data/transform.h"
#include "event.h"
#include "scene_core/entity.h"

namespace mite {
/**
 * @class SceneNode
 * @brief 场景节点类，管理场景中实体的层级关系和空间信息
 *
 * 基于ECS组件的场景节点，从TransformComponent和BoundingVolumeComponent
 * 获取局部Transform数据与局部空间包围盒，计算并缓存世界空间状态
 */
class SceneNode : public std::enable_shared_from_this<SceneNode> {
 public:
  /**
   * @brief 构造函数
   * @param entity 关联的ECS实体
   */
  explicit SceneNode(Entity entity);
  ~SceneNode();

  // 禁止拷贝和赋值
  SceneNode(const SceneNode &) = delete;
  SceneNode &operator=(const SceneNode &) = delete;

  // ==================== 实体和关系操作 ====================
  /**
   * @brief 获取关联的ECS实体
   * @return ECS实体引用
   */
  Entity GetEntity();
  /**
   * @brief 设置父节点
   * @param parent 父节点指针
   */
  void SetParent(std::shared_ptr<SceneNode> parent);
  /**
   * @brief 获取父节点
   * @return 父节点指针（可能为nullptr）
   */
  std::shared_ptr<SceneNode> GetParent() const;
  /**
   * @brief 添加子节点
   * @param child 子节点指针
   */
  void AddChild(std::shared_ptr<SceneNode> child);
  /**
   * @brief 移除子节点
   * @param child 要移除的子节点指针
   * @return 是否成功移除
   */
  bool RemoveChild(std::shared_ptr<SceneNode> child);
  /**
   * @brief 递归判断child是否为当前节点的第n代子节点
   */
  bool IsChild(std::shared_ptr<SceneNode> child);
  /**
   * @brief 获取所有子节点
   * @return 子节点指针列表
   */
  const std::vector<std::shared_ptr<SceneNode> > &GetChildren() const;

  // ==================== 状态查询 ====================
  /**
   * @brief 判断节点是否为根节点
   * @return 是否为根节点
   */
  bool IsRoot() const;
  /**
   * @brief 判断节点是否为叶子节点
   * @return 是否为叶子节点
   */
  bool IsLeaf() const;
  /**
   * @brief 获取节点在场景树中的深度（根节点为0）
   * @return 节点深度
   */
  int GetDepth() const;
  /**
   * @brief 获取节点的完整路径（用于调试）
   * @return 节点路径字符串
   */
  std::string GetPath();

  // ==================== 变换相关 ====================
  /**
   * @brief 获取世界变换矩阵
   * @return 世界变换矩阵
   */
  const Transform &GetWorldTransform() const;
  /**
   * @brief 获取变换的脏状态
   */
  bool IsTransformDirty() const;
  /**
   * @brief 标记变换为脏状态，需要重新计算世界矩阵
   * @param transformBias 矩阵偏差，用于修正本地矩阵
   */
  void MarkTransformDirty();

  // ==================== 包围盒相关 ====================
  /**
   * @brief 获取世界包围盒（世界空间）
   * @return 世界包围盒
   */
  BoundingVolume GetWorldBounds() const;
  /**
   * @brief  获取包围盒的脏状态
   */
  bool IsBoundsDirty() const;
  /**
   * @brief 标记包围盒为脏状态，需要重新计算世界包围盒
   */
  void MarkBoundsDirty();

  // ==================== 可见性相关 ====================
  /**
   * @brief 获取世界可见性状态（考虑父子继承关系）
   * @return 在世界空间中是否可见
   */
  bool IsWorldVisible() const;
  /**
   * @brief 获取可见性掩码（本地掩码即为世界掩码）
   * @return 32位可见性掩码
   */
  uint32_t GetVisibilityMask() const;
  /**
   * @brief 获取可见性脏状态
   * @return 可见性是否需要重新计算
   */
  bool IsVisibilityDirty() const;
  /**
   * @brief 标记可见性为脏状态
   */
  void MarkVisibilityDirty();

  // ==================== 更新操作 ====================
  /**
   * @brief 更新世界变换
   * @param registry ECS注册表
   */
  void UpdateWorldTransform(const SceneRegistry &registry);
  /**
   * @brief 更新世界包围盒
   * @param registry ECS注册表
   */
  void UpdateWorldBounds(const SceneRegistry &registry);
  /**
   * @brief 更新可见性
   * @param registry ECS注册表
   */
  void UpdateVisibility(const SceneRegistry &registry);
  /**
   * @brief 执行更新操作
   * @param registry ECS注册表
   */
  void Update(const SceneRegistry &registry, bool force = false);

 private:
  Entity m_Entity;                                      // 关联的ECS实体
  std::shared_ptr<SceneNode> m_Parent = nullptr;        // 父节点指针
  std::vector<std::shared_ptr<SceneNode> > m_Children;  // 子节点列表

  // 世界空间缓存
  Transform m_WorldTransform;    // 世界变换矩阵
  BoundingVolume m_WorldBounds;  // 世界空间包围盒

  // 可见性状态
  bool m_WorldVisible = true;  // 世界可见性状态（计算得出）
  uint32_t m_VisibilityMask = 0xFFFFFFFF;  // 可见性掩码
  bool m_VisibilityDirty = true;           // 可见性需要重新计算

  // 脏标记
  bool m_TransformDirty = true;  // 变换需要更新
  bool m_BoundsDirty = true;     // 包围盒需要更新
  glm::mat4 m_transformBias = glm::mat4(1.0f);
};

/**
 * @class SceneNode
 * @brief SceneTree的SceneNode选择事件
 * @note 需要依赖SceneNode，无法放在Data的render_event.h中
 */
class SceneNodeSelectedEvent : public Event {
 public:
  explicit SceneNodeSelectedEvent(std::shared_ptr<SceneNode> node)
      : m_node(node) {}
  std::shared_ptr<SceneNode> GetSceneNode() const { return m_node; }
  EVENT_CLASS_CATEGORY(EVENT_CATEGORY_RENDER)
  Event *Clone() const override { return new SceneNodeSelectedEvent(m_node); }

 private:
  std::shared_ptr<SceneNode> m_node;
};

class SceneNodeParentChangeEvent : public Event {
 public:
  explicit SceneNodeParentChangeEvent(std::shared_ptr<SceneNode> node,
                                      std::shared_ptr<SceneNode> newParent)
      : m_node(node), m_newParent(newParent) {}
  std::shared_ptr<SceneNode> GetSceneNode() const { return m_node; }
  std::shared_ptr<SceneNode> GetNewParent() const { return m_newParent; }
  EVENT_CLASS_CATEGORY(EVENT_CATEGORY_RENDER)
  Event *Clone() const override {
    return new SceneNodeParentChangeEvent(m_node, m_newParent);
  }

 private:
  std::shared_ptr<SceneNode> m_node;
  std::shared_ptr<SceneNode> m_newParent;
};
}  // namespace mite

#endif  // MITE_SCENE_NODE_H
