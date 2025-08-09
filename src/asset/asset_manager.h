#ifndef MITE_ASSET_MANAGER
#define MITE_ASSET_MANAGER

#include "asset_cache.h"
#include "model_loader.h"
#include "texture_loader.h"

namespace mite {
/**
 * 资产管理器（资源加载与生命周期管理）
 * 职责：
 * 1. 加载磁盘资源并转换为引擎中间格式
 * 2. 维护资源缓存和引用计数
 */
class AssetManager {
 public:
  AssetManager() = default;
  ~AssetManager();

  // ---- 核心接口 ----
  AssetID LoadTexture(const std::string &path);
  std::shared_ptr<TextureAsset> GetTexture(AssetID id) const;
  void ReleaseTexture(AssetID id);

  AssetID LoadModel(const std::string &path);
  std::shared_ptr<ModelAsset> GetModel(AssetID id) const;
  void ReleaseModel(AssetID id);

  // ---- 禁用拷贝 ----
  AssetManager(const AssetManager &) = delete;
  AssetManager &operator=(const AssetManager &) = delete;

 private:
  // ---- 内部方法 ----
  void LoadTextureInternalToCache(const std::string &path);
  void LoadModelInternalToCache(const std::string &path);

  // ---- 成员变量 ----
  TextureCache m_TextureCache;  // 纹理资源缓存
  ModelCache m_ModelCache;      // 模型资源缓存
  mutable std::mutex mutex_;    // 线程安全锁
};
};  // namespace mite

#endif
