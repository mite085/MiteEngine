#include "asset_cache.h"

namespace mite {
// ------------------------ 模板特化实现 ------------------------
template<typename AssetType> bool AssetCache<AssetType>::Store(AssetPtr asset)
{
  std::lock_guard<std::mutex> lock(mutex_);

  AssetID id = asset->id;

  // 检查是否已存在
  if (cache_.find(id) != cache_.end()) {
    return false;  // 已存在的资源需要显式更新
  }

  // 添加新资源（初始引用计数为0，由调用方决定是否增加）
  auto [it, success] = cache_.emplace(id,
                                      CachedAsset{asset, 0, lruList_.end()});  // 初始不在LRU列表中

  // 更新LRU列表（如果启用）
  if (success && maxSize_ > 0) {
    lruList_.push_front(id);
    it->second.lruIt = lruList_.begin();

    // 检查是否超过最大缓存限制
    if (lruList_.size() > maxSize_) {
      AssetID idToRemove = lruList_.back();
      if (cache_[idToRemove].refCount == 0) {
        cache_.erase(idToRemove);
      }
      lruList_.pop_back();
    }
  }

  return success;
}

template<typename AssetType>
typename AssetCache<AssetType>::AssetPtr AssetCache<AssetType>::Get(AssetID id) const
{
  std::lock_guard<std::mutex> lock(mutex_);
  auto it = cache_.find(id);
  if (it == cache_.end()) {
    return nullptr;
  }

  // 更新LRU访问记录（如果启用）
  if (maxSize_ > 0) {
    lruList_.splice(lruList_.begin(), lruList_, it->second.lruIt);
  }

  return it->second.data;
}

template<typename AssetType> int AssetCache<AssetType>::Release(AssetID id)
{
  std::lock_guard<std::mutex> lock(mutex_);
  auto it = cache_.find(id);
  if (it == cache_.end()) {
    return -1;  // 资源不存在
  }

  // 减少引用计数（但不立即删除，等待PurgeUnused()）
  int newCount = --(it->second.refCount);
  if (newCount <= 0 && maxSize_ == 0) {
    // 如果未启用LRU且引用归零，立即删除
    cache_.erase(it);
  }

  return newCount;
}

template<typename AssetType> int AssetCache<AssetType>::GetRefCount(AssetID id) const
{
  std::lock_guard<std::mutex> lock(mutex_);
  auto it = cache_.find(id);
  return it != cache_.end() ? it->second.refCount : -1;
}

template<typename AssetType> void AssetCache<AssetType>::AddRefCount(AssetID id)
{
  std::lock_guard<std::mutex> lock(mutex_);
  auto it = cache_.find(id);
  if (it != cache_.end())
    it->second.refCount++;
}

template<typename AssetType> size_t AssetCache<AssetType>::PurgeUnused()
{
  std::lock_guard<std::mutex> lock(mutex_);
  size_t count = 0;

  for (auto it = cache_.begin(); it != cache_.end();) {
    if (it->second.refCount <= 0) {
      if (maxSize_ > 0) {
        lruList_.erase(it->second.lruIt);  // 从LRU列表移除
      }
      it = cache_.erase(it);
      count++;
    }
    else {
      ++it;
    }
  }

  return count;
}

template<typename AssetType> bool AssetCache<AssetType>::ForceRemove(AssetID id)
{
  std::lock_guard<std::mutex> lock(mutex_);
  auto it = cache_.find(id);
  if (it == cache_.end()) {
    return false;
  }

  if (maxSize_ > 0) {
    lruList_.erase(it->second.lruIt);  // 从LRU列表移除
  }
  cache_.erase(it);
  return true;
}

// 显式实例化模板（确保链接器能找到实现）
template class AssetCache<TextureAsset>;
template class AssetCache<ModelAsset>;
};  // namespace mite