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

  /**
   * @brief LoadModel 加载模型文件
   * @param path 模型文件路径
   * @param flipUVs 是否翻转UV垂直坐标（适配OpenGL坐标系）
   * @param generateLODs 是否生成多级LOD
   * @param lodLevels LOD级别配置（每个级别的简化比例）
   * @return 包含模型元数据和所有子网格数据的结构体
   * @return 
   */
  AssetID LoadModel(const std::string &path,
                    bool flipUVs = true,
                    bool generateLODs = false,
                    const std::vector<float> &lodLevels = {1.0f, 0.5f, 0.25f, 0.1f});
  std::shared_ptr<ModelAsset> GetModel(AssetID id) const;
  void ReleaseModel(AssetID id);

  // ---- 禁用拷贝 ----
  AssetManager(const AssetManager &) = delete;
  AssetManager &operator=(const AssetManager &) = delete;

 private:
  // ---- 内部方法 ----
  void LoadTextureInternalToCache(const std::string &path);
  void LoadModelInternalToCache(const std::string &path,
                                bool flipUVs,
                                bool generateLODs,
                                const std::vector<float> &lodLevels);

  // ---- 成员变量 ----
  TextureCache m_TextureCache;  // 纹理资源缓存
  ModelCache m_ModelCache;      // 模型资源缓存
  mutable std::mutex m_Mutex;    // 线程安全锁
};
};  // namespace mite

#endif
