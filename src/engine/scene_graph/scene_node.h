#ifndef MITE_SCENE_NODE_H
#define MITE_SCENE_NODE_H

#include "bounding_volumes_types.h"
#include "scene_core/entity.h"

namespace mite {

/**
 * @class SceneNode
 * @brief 场景节点类，管理场景中实体的层级关系和空间信息
 *
 * 每个SceneNode对应一个ECS实体，维护父子关系、局部变换和包围体信息
 * 用于场景图管理和空间查询优化
 */
class SceneNode {
 public:
  /**
   * @brief 构造函数
   * @param entity 关联的ECS实体
   */
  explicit SceneNode(Entity entity);

  /**
   * @brief 析构函数
   */
  ~SceneNode();

  /**
   * @brief 获取关联的ECS实体
   * @return ECS实体引用
   */
  Entity GetEntity() const
  {
    return entity_;
  }

  /**
   * @brief 设置父节点
   * @param parent 父节点指针
   */
  void SetParent(SceneNode *parent);

  /**
   * @brief 获取父节点
   * @return 父节点指针（可能为nullptr）
   */
  SceneNode *GetParent() const
  {
    return parent_;
  }

  /**
   * @brief 添加子节点
   * @param child 子节点指针
   */
  void AddChild(SceneNode *child);

  /**
   * @brief 移除子节点
   * @param child 要移除的子节点指针
   * @return 是否成功移除
   */
  bool RemoveChild(SceneNode *child);

  /**
   * @brief 获取所有子节点
   * @return 子节点指针列表
   */
  const std::vector<SceneNode *> &GetChildren() const
  {
    return children_;
  }

  /**
   * @brief 设置局部变换矩阵
   * @param localTransform 局部变换矩阵
   */
  void SetLocalTransform(const glm::mat4 &localTransform);

  /**
   * @brief 获取局部变换矩阵
   * @return 局部变换矩阵
   */
  const glm::mat4 &GetLocalTransform() const
  {
    return localTransform_;
  }

  /**
   * @brief 获取世界变换矩阵
   * @return 世界变换矩阵
   */
  const glm::mat4 &GetWorldTransform() const
  {
    return worldTransform_;
  }

  /**
   * @brief 设置局部包围盒（模型空间）
   * @param localBounds 局部包围盒
   */
  void SetLocalBounds(const AABB &localBounds);

  /**
   * @brief 获取局部包围盒（模型空间）
   * @return 局部包围盒
   */
  const AABB &GetLocalBounds() const
  {
    return localBounds_;
  }

  /**
   * @brief 获取世界包围盒（世界空间）
   * @return 世界包围盒
   */
  const AABB &GetWorldBounds() const
  {
    return worldBounds_;
  }

  /**
   * @brief 标记变换为脏状态，需要重新计算世界矩阵
   */
  void MarkTransformDirty();

  /**
   * @brief 标记包围盒为脏状态，需要重新计算世界包围盒
   */
  void MarkBoundsDirty();

  /**
   * @brief 更新世界变换和包围盒（如果为脏状态）
   * @param force 强制更新（即使不是脏状态）
   */
  void Update(bool force = false);

  /**
   * @brief 判断节点是否为脏状态（需要更新）
   * @return 是否为脏状态
   */
  bool IsDirty() const
  {
    return transformDirty_ || boundsDirty_;
  }

  /**
   * @brief 获取节点在场景树中的深度（根节点为0）
   * @return 节点深度
   */
  int GetDepth() const;

  /**
   * @brief 判断节点是否为根节点
   * @return 是否为根节点
   */
  bool IsRoot() const
  {
    return parent_ == nullptr;
  }

  /**
   * @brief 判断节点是否为叶子节点
   * @return 是否为叶子节点
   */
  bool IsLeaf() const
  {
    return children_.empty();
  }

  /**
   * @brief 获取节点的完整路径（用于调试）
   * @return 节点路径字符串
   */
  std::string GetPath() const;

  /**
   * @brief 检查节点是否可见
   * @param registry 场景注册表
   * @return 是否可见
   */
  bool IsNodeVisible(SceneRegistry &registry, uint32_t visibilityMask) const;

 private:
  /**
   * @brief 递归更新子节点的变换状态
   */
  void MarkChildrenTransformDirty();

  /**
   * @brief 递归更新子节点的包围盒状态
   */
  void MarkChildrenBoundsDirty();

  /**
   * @brief 计算世界变换矩阵
   */
  void UpdateWorldTransform();

  /**
   * @brief 计算世界包围盒
   */
  void UpdateWorldBounds();

 private:
  Entity entity_;                      ///< 关联的ECS实体
  SceneNode *parent_ = nullptr;        ///< 父节点指针
  std::vector<SceneNode *> children_;  ///< 子节点列表

  glm::mat4 localTransform_ = glm::mat4(1.0f);  ///< 局部变换矩阵
  glm::mat4 worldTransform_ = glm::mat4(1.0f);  ///< 世界变换矩阵

  AABB localBounds_;  ///< 局部空间包围盒
  AABB worldBounds_;  ///< 世界空间包围盒

  bool transformDirty_ = true;  ///< 变换脏标记
  bool boundsDirty_ = true;     ///< 包围盒脏标记

  // 禁止拷贝和赋值
  SceneNode(const SceneNode &) = delete;
  SceneNode &operator=(const SceneNode &) = delete;
};

}  // namespace mite

#endif  // MITE_SCENE_NODE_H
