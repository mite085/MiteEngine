#ifndef MITE_ASSET_MANAGER
#define MITE_ASSET_MANAGER

#include "texture_loader.h"
#include "model_loader.h"
#include "render_device.h"

namespace mite {
// 前向声明（避免包含渲染API头文件）
class IRenderDevice;

/**
 * 资产管理器（资源加载与生命周期管理）
 * 职责：
 * 1. 加载磁盘资源并转换为引擎中间格式
 * 2. 通过IRenderDevice委托GPU资源创建
 * 3. 维护资源缓存和引用计数
 */
class AssetManager {
 public:
  explicit AssetManager(IRenderDevice *renderDevice);
  ~AssetManager();

  // ---- 核心接口 ----
  TextureHandle LoadTexture(const std::string &path);
  void ReleaseTexture(TextureHandle handle);

  ModelHandle LoadModel(const std::string &path);
  void ReleaseModel(ModelHandle handle);

  // ---- 查询接口 ----
  const TextureMetadata *GetTextureMetadata(TextureHandle handle) const;
  const ModelMetadata *GetModelMetadata(ModelHandle handle) const;

  // 禁用拷贝
  AssetManager(const AssetManager &) = delete;
  AssetManager &operator=(const AssetManager &) = delete;

 private:
  // ---- 内部数据结构 ----
  struct TextureAsset {
    std::shared_ptr<TextureMetadata> metadata;
    TextureGPUHandle gpuHandle;
    int refCount = 0;
  };

  struct ModelAsset {
    std::shared_ptr<ModelMetadata> metadata;
    ModelGPUHandle gpuHandle;
    int refCount = 0;
  };

  // ---- 内部方法 ----
  TextureHandle LoadTextureInternal(const std::string &path);
  ModelHandle LoadModelInternal(const std::string &path);

  // ---- 成员变量 ----
  IRenderDevice *renderDevice_;  // 渲染设备抽象接口
  std::unordered_map<AssetID, TextureAsset> textureCache_;
  std::unordered_map<AssetID, ModelAsset> modelCache_;
  mutable std::mutex cacheMutex_;  // 线程安全锁
};

};

#endif
