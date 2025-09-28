#include "render_queue.h"

namespace mite {

RenderQueue::RenderQueue()
{
  // 设置默认排序策略
  m_OpaqueQueue.sortStrategy = SortStrategy::FrontToBack;
  m_TransparentQueue.sortStrategy = SortStrategy::BackToFront;
  m_AlphaTestQueue.sortStrategy = SortStrategy::ByMaterial;
  m_CustomQueue.sortStrategy = SortStrategy::None;
}

RenderQueue::~RenderQueue()
{
  // 清理资源
  ClearAll();
}

void RenderQueue::AddItem(const RenderableItem &item, QueueType queueType)
{
  QueueData &queue = GetQueueData(queueType);
  queue.items.push_back(item);
}

void RenderQueue::AddItems(const std::vector<RenderableItem> &items, QueueType queueType)
{
  QueueData &queue = GetQueueData(queueType);
  queue.items.insert(queue.items.end(), items.begin(), items.end());
}

void RenderQueue::ClearQueue(QueueType queueType)
{
  QueueData &queue = GetQueueData(queueType);
  queue.items.clear();
}

void RenderQueue::ClearAll()
{
  m_OpaqueQueue.items.clear();
  m_TransparentQueue.items.clear();
  m_AlphaTestQueue.items.clear();
  m_CustomQueue.items.clear();
}

void RenderQueue::SetSortStrategy(QueueType queueType, SortStrategy strategy)
{
  QueueData &queue = GetQueueData(queueType);
  queue.sortStrategy = strategy;
}

void RenderQueue::SortQueue(QueueType queueType)
{
  QueueData &queue = GetQueueData(queueType);

  // 如果设置了自定义排序函数，优先使用自定义排序
  if (queue.customSortFunc) {
    std::sort(queue.items.begin(), queue.items.end(), queue.customSortFunc);
  }
  // 否则按照排序策略执行
  else {

    switch (queue.sortStrategy) {
      case SortStrategy::None:
        // 不排序
        break;

      case SortStrategy::FrontToBack:
        // 按距离从前到后排序（减少overdraw）
        std::sort(queue.items.begin(),
                  queue.items.end(),
                  [](const RenderableItem &a, const RenderableItem &b) {
                    return a.distanceToCamera < b.distanceToCamera;
                  });
        break;

      case SortStrategy::BackToFront:
        // 按距离从后到前排序（透明物体正确混合）
        std::sort(queue.items.begin(),
                  queue.items.end(),
                  [](const RenderableItem &a, const RenderableItem &b) {
                    return a.distanceToCamera > b.distanceToCamera;
                  });
        break;

      case SortStrategy::ByMaterial:
        // 按材质排序（减少状态切换）
        std::sort(queue.items.begin(),
                  queue.items.end(),
                  [](const RenderableItem &a, const RenderableItem &b) {
                    return a.material->GetName() < b.material->GetName();
                  });
        break;

      case SortStrategy::ByShader:
        // 按Shader排序（减少状态切换）
        std::sort(
            queue.items.begin(),
            queue.items.end(),
            [](const RenderableItem &a, const RenderableItem &b) {
              return a.material->GetShader()->GetHandle().programId <
                     b.material->GetShader()->GetHandle().programId;
            });
        break;
    }
  }
}

void RenderQueue::SortAll()
{
  SortQueue(QueueType::Opaque);
  SortQueue(QueueType::Transparent);
  SortQueue(QueueType::AlphaTest);
  SortQueue(QueueType::Custom);
}

const std::vector<RenderableItem> &RenderQueue::GetItems(QueueType queueType) const
{
  return GetQueueData(queueType).items;
}

size_t RenderQueue::GetItemCount(QueueType queueType) const
{
  return GetQueueData(queueType).items.size();
}

size_t RenderQueue::GetTotalItemCount() const
{
  return m_OpaqueQueue.items.size() + m_TransparentQueue.items.size() + m_AlphaTestQueue.items.size() +
         m_CustomQueue.items.size();
}

void RenderQueue::SetCustomSortFunction(
    QueueType queueType,
    std::function<bool(const RenderableItem &, const RenderableItem &)> sortFunc)
{
  QueueData &queue = GetQueueData(queueType);
  queue.customSortFunc = sortFunc;
}

void RenderQueue::SetQueueVisibility(QueueType queueType, bool visible)
{
  QueueData &queue = GetQueueData(queueType);
  queue.isVisible = visible;
}

RenderQueue::QueueData &RenderQueue::GetQueueData(QueueType queueType)
{
  switch (queueType) {
    case QueueType::Opaque:
      return m_OpaqueQueue;
    case QueueType::Transparent:
      return m_TransparentQueue;
    case QueueType::AlphaTest:
      return m_AlphaTestQueue;
    case QueueType::Custom:
      return m_CustomQueue;
    default:
      return m_OpaqueQueue;
  }
}

const RenderQueue::QueueData &RenderQueue::GetQueueData(QueueType queueType) const
{
  switch (queueType) {
    case QueueType::Opaque:
      return m_OpaqueQueue;
    case QueueType::Transparent:
      return m_TransparentQueue;
    case QueueType::AlphaTest:
      return m_AlphaTestQueue;
    case QueueType::Custom:
      return m_CustomQueue;
    default:
      return m_OpaqueQueue;
  }
}

}  // namespace mite
