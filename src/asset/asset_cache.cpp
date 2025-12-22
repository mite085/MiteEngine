#include "asset_cache.h"

namespace mite {
// ------------------------ 模板特化实现 ------------------------
template <typename AssetType>
bool AssetCache<AssetType>::Store(AssetPtr asset) {
  std::lock_guard<std::mutex> lock(m_Mutex);
  AssetIDType id = asset->GetID();
  // 检查是否已存在
  if (m_Cache.find(id) != m_Cache.end()) {
    return false;  // 已存在的资源需要显式更新
  }
  // 添加新资源（初始引用计数为0，由调用方决定是否增加）
  auto [it, success] =
      m_Cache.emplace(id, CachedAsset{asset, 0, m_LruList.end()});
  if (success && m_MaxSize > 0) {
    // 更新LRU列表
    m_LruList.push_front(id);
    it->second.lruIt = m_LruList.begin();
    // 检查是否超过最大缓存限制
    if (m_LruList.size() > m_MaxSize) {
      AssetIDType idToRemove = m_LruList.back();
      auto removeIt = m_Cache.find(idToRemove);
      if (removeIt != m_Cache.end() && removeIt->second.refCount == 0) {
        m_Cache.erase(removeIt);
      }
      m_LruList.pop_back();
    }
  }
  return success;
}

template <typename AssetType>
typename AssetCache<AssetType>::AssetPtr AssetCache<AssetType>::Get(
    const AssetIDType &id) const {
  std::lock_guard<std::mutex> lock(m_Mutex);
  auto it = m_Cache.find(id);
  if (it == m_Cache.end()) {
    return nullptr;
  }
  // 更新LRU访问记录（如果启用）
  if (m_MaxSize > 0) {
    m_LruList.splice(m_LruList.begin(), m_LruList, it->second.lruIt);
    it->second.lruIt = m_LruList.begin();
  }
  return it->second.data;
}

template <typename AssetType>
int AssetCache<AssetType>::Release(const AssetIDType &id) {
  std::lock_guard<std::mutex> lock(m_Mutex);
  auto it = m_Cache.find(id);
  if (it == m_Cache.end()) {
    return -1;  // 资源不存在
  }
  // 减少引用计数
  int newCount = --(it->second.refCount);

  // 如果未启用LRU且引用归零，立即删除
  if (newCount <= 0 && m_MaxSize == 0) {
    m_Cache.erase(it);
  }
  return newCount;
}

template <typename AssetType>
int AssetCache<AssetType>::GetRefCount(const AssetIDType &id) const {
  std::lock_guard<std::mutex> lock(m_Mutex);
  auto it = m_Cache.find(id);
  return it != m_Cache.end() ? it->second.refCount : -1;
}

template <typename AssetType>
void AssetCache<AssetType>::AddRefCount(const AssetIDType &id) const {
  std::lock_guard<std::mutex> lock(m_Mutex);
  auto it = m_Cache.find(id);
  if (it != m_Cache.end()) {
    it->second.refCount++;
  }
}

template <typename AssetType>
size_t AssetCache<AssetType>::PurgeUnused() {
  std::lock_guard<std::mutex> lock(m_Mutex);
  size_t count = 0;
  for (auto it = m_Cache.begin(); it != m_Cache.end();) {
    if (it->second.refCount <= 0) {
      if (m_MaxSize > 0) {
        m_LruList.erase(it->second.lruIt);  // 从LRU列表移除
      }
      it = m_Cache.erase(it);
      count++;
    } else {
      ++it;
    }
  }
  return count;
}

template <typename AssetType>
bool AssetCache<AssetType>::ForceRemove(const AssetIDType &id) {
  std::lock_guard<std::mutex> lock(m_Mutex);
  auto it = m_Cache.find(id);
  if (it == m_Cache.end()) {
    return false;
  }
  if (m_MaxSize > 0) {
    m_LruList.erase(it->second.lruIt);  // 从LRU列表移除
  }
  m_Cache.erase(it);
  return true;
}
template <typename AssetType>
size_t AssetCache<AssetType>::Size() const {
  std::lock_guard<std::mutex> lock(m_Mutex);
  return m_Cache.size();
}
template <typename AssetType>
void AssetCache<AssetType>::SetMaxSize(size_t maxSize) {
  std::lock_guard<std::mutex> lock(m_Mutex);
  m_MaxSize = maxSize;

  // 如果新的大小小于当前大小，需要清理LRU列表
  if (m_MaxSize > 0 && m_LruList.size() > m_MaxSize) {
    while (m_LruList.size() > m_MaxSize) {
      AssetIDType idToRemove = m_LruList.back();
      auto removeIt = m_Cache.find(idToRemove);
      if (removeIt != m_Cache.end() && removeIt->second.refCount == 0) {
        m_Cache.erase(removeIt);
      }
      m_LruList.pop_back();
    }
  }
}

// 显式实例化模板（确保链接器能找到实现）
template class AssetCache<TextureAsset>;
template class AssetCache<MaterialAsset>;
template class AssetCache<ModelAsset>;
};  // namespace mite