#ifndef MITE_RENDER_QUEUE_H
#define MITE_RENDER_QUEUE_H

#include "renderable_item.h"

namespace mite {

/**
 * @brief 渲染队列管理器
 * @note 职责：管理渲染项的收集、排序和提交，为Renderer提供标准化的渲染数据接口
 * @note 扩展性考虑：支持多种排序策略，预留多队列管理和批处理优化接口
 */
class RenderQueue {
 public:
  /**
   * @brief 渲染队列类型枚举
   */
  enum class QueueType {
    Opaque,       // 不透明物体队列
    Transparent,  // 透明物体队列
    AlphaTest,    // Alpha测试物体队列
    Custom        // 自定义队列（预留）
  };

  /**
   * @brief 排序策略枚举
   */
  enum class SortStrategy {
    None,         // 不排序
    FrontToBack,  // 从前到后（用于不透明物体）
    BackToFront,  // 从后到前（用于透明物体）
    ByMaterial,   // 按材质排序（减少状态切换）
    ByShader      // 按Shader排序（减少状态切换）
  };

  /**
   * @brief 构造函数
   */
  RenderQueue();

  /**
   * @brief 析构函数
   */
  ~RenderQueue();

  // ==================== 队列管理接口 ====================
  /**
   * @brief 添加渲染项到指定队列
   * @param item 渲染项
   * @param queueType 队列类型
   */
  void AddItem(const RenderableItem &item, QueueType queueType = QueueType::Opaque);

  /**
   * @brief 批量添加渲染项
   * @param items 渲染项列表
   * @param queueType 队列类型
   */
  void AddItems(const std::vector<RenderableItem> &items, QueueType queueType = QueueType::Opaque);

  /**
   * @brief 清空指定队列
   * @param queueType 队列类型
   */
  void ClearQueue(QueueType queueType);

  /**
   * @brief 清空所有队列
   */
  void ClearAll();

  // ==================== 排序接口 ====================
  /**
   * @brief 设置队列排序策略
   * @param queueType 队列类型
   * @param strategy 排序策略
   */
  void SetSortStrategy(QueueType queueType, SortStrategy strategy);

  /**
   * @brief 对指定队列进行排序
   * @param queueType 队列类型
   */
  void SortQueue(QueueType queueType);

  /**
   * @brief 对所有队列进行排序
   */
  void SortAll();

  // ==================== 数据访问接口 ====================
  /**
   * @brief 获取指定队列的渲染项列表（只读）
   * @param queueType 队列类型
   * @return 渲染项列表的常量引用
   */
  const std::vector<RenderableItem> &GetItems(QueueType queueType) const;

  /**
   * @brief 获取指定队列的渲染项数量
   * @param queueType 队列类型
   * @return 队列中的项数
   */
  size_t GetItemCount(QueueType queueType) const;

  /**
   * @brief 获取所有队列的总渲染项数量
   * @return 总项数
   */
  size_t GetTotalItemCount() const;

  // ==================== 扩展接口（预留） ====================
  /**
   * @brief 设置自定义排序函数
   * @param queueType 队列类型
   * @param sortFunc 自定义排序函数
   */
  void SetCustomSortFunction(
      QueueType queueType,
      std::function<bool(const RenderableItem &, const RenderableItem &)> sortFunc);

  /**
   * @brief 设置队列可见性（预留多视口支持）
   * @param queueType 队列类型
   * @param visible 是否可见
   */
  void SetQueueVisibility(QueueType queueType, bool visible);

 private:
  /**
   * @brief 队列数据结构
   */
  struct QueueData {
    std::vector<RenderableItem> items;  // 渲染项列表
    SortStrategy sortStrategy;          // 排序策略
    bool isVisible;                     // 队列可见性
    std::function<bool(const RenderableItem &, const RenderableItem &)>
        customSortFunc;  // 自定义排序函数

    QueueData() : sortStrategy(SortStrategy::None), isVisible(true) {}
  };

  QueueData m_OpaqueQueue;       // 不透明物体队列
  QueueData m_TransparentQueue;  // 透明物体队列
  QueueData m_AlphaTestQueue;    // Alpha测试物体队列
  QueueData m_CustomQueue;       // 自定义队列（预留）

  /**
   * @brief 根据队列类型获取对应的队列数据
   * @param queueType 队列类型
   * @return 队列数据的引用
   */
  QueueData &GetQueueData(QueueType queueType);

  /**
   * @brief 根据队列类型获取对应的队列数据（常量版本）
   * @param queueType 队列类型
   * @return 队列数据的常量引用
   */
  const QueueData &GetQueueData(QueueType queueType) const;
};

}  // namespace mite

#endif  // MITE_RENDER_QUEUE_H
