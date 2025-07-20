#ifndef MITE_ASSET_CACHE
#define MITE_ASSET_CACHE

#include "headers/headers.h"
#include "basic_type/asset_type.h"

namespace mite {
/**
 * 资源缓存核心类（线程安全）
 * 职责：
 * 1. 管理所有已加载资源的生命周期（模型/纹理等）
 * 2. 实现引用计数自动释放
 * 3. 支持LRU缓存淘汰策略
 * 
 * 注意：
 * AssetCache缓存的对象均为shared_ptr，其他任何
 * 模块均将AssetPtr作为临时变量调用，所以当cache_
 * 哈希表线程安全的删除AssetPtr时，外部不可能存在
 * 其他AssetPtr，所以会触发shared_ptr自动释放资源。
 */
template<typename AssetType> class AssetCache {
 public:
  using AssetPtr = std::shared_ptr<AssetType>;

  /**
   * @brief 添加资源到缓存
   * @param asset 资源数据指针
   * @return 是否缓存成功（若id已存在则失败）
   */
  bool Store(AssetPtr asset);

  /**
   * @brief 获取缓存资源
   * @param id 资源ID
   * @return 资源指针（不存在返回nullptr）
   */
  AssetPtr Get(AssetID id) const;

  /**
   * @brief 释放资源引用
   * @param id 资源ID
   * @return 当前剩余引用计数（-1表示资源不存在）
   */
  int Release(AssetID id);

  /**
   * @brief 获取当前资源引用计数
   */
  int GetRefCount(AssetID id) const;

  /**
   * @brief 增加当前资源引用计数
   */
  void AddRefCount(AssetID id);

  /**
   * @brief 清理所有未被引用的资源
   * @return 被释放的资源数量
   */
  size_t PurgeUnused();

  /**
   * @brief 强制移除资源（无视引用计数）
   * @return 是否成功移除
   */
  bool ForceRemove(AssetID id);

 private:
  // ---- 内部数据结构 ----
  struct CachedAsset {
    AssetPtr data;
    int refCount = 0;
    typename std::list<AssetID>::iterator lruIt;  // 用于LRU链表
  };

  // ---- 成员变量 ----
  mutable std::mutex mutex_;
  std::unordered_map<AssetID, CachedAsset> cache_;

  // LRU实现：（Least recently used，最近最少使用）
  // 该算法根据数据的历史访问记录来进行淘汰数据，
  // 确保缓存占用小，且被重复访问的效率高.
  mutable std::list<AssetID> lruList_;  // 最近使用顺序
  mutable size_t maxSize_ = 1000;       // 最大缓存数量
};

// 常用缓存类型别名
using TextureCache = AssetCache<TextureAsset>;
using ModelCache = AssetCache<ModelAsset>;
};  // namespace mite

#endif
