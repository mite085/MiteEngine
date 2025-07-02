#ifndef MITE_ASSET_MANAGER
#define MITE_ASSET_MANAGER

#include "asset_cache.h"
#include "model_loader.h"
#include "render_device.h"
#include "texture_loader.h"

namespace mite {
/**
 * 资产管理器（资源加载与生命周期管理）
 * 职责：
 * 1. 加载磁盘资源并转换为引擎中间格式
 * 2. 通过IRenderDevice委托GPU资源创建
 * 3. 维护资源缓存和引用计数
 */
class AssetManager {
 public:
  explicit AssetManager(IRenderDevice &renderDevice);
  ~AssetManager();

  // ---- 核心接口 ----
  std::shared_ptr<TextureAsset> LoadTexture(const std::string &path);
  void ReleaseTexture(TextureAsset asset);

  std::shared_ptr<ModelAsset> LoadModel(const std::string &path);
  void ReleaseModel(ModelAsset asset);

  // ---- 禁用拷贝 ----
  AssetManager(const AssetManager &) = delete;
  AssetManager &operator=(const AssetManager &) = delete;

 private:
  // ---- 内部方法 ----
  void LoadTextureInternalToCache(const std::string &path);
  void LoadModelInternalToCache(const std::string &path);

  // ---- 成员变量 ----
  IRenderDevice &m_RenderDevice;  // 渲染设备抽象接口
  TextureCache m_TextureCache;    // 纹理资源缓存
  ModelCache m_ModelCache;        // 模型资源缓存
  mutable std::mutex mutex_;     // 线程安全锁
};
};  // namespace mite

#endif
