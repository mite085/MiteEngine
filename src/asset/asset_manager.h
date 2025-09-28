#ifndef MITE_ASSET_MANAGER
#define MITE_ASSET_MANAGER

#include "asset_cache.h"


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

  // ---- 纹理管理接口 ----
  TextureAssetID LoadTexture(const std::string &path);
  std::shared_ptr<TextureAsset> GetTexture(TextureAssetID id) const;
  void ReleaseTexture(TextureAssetID id);

  // ---- 模型管理接口（格式特化） ----
  ModelAssetID LoadGLTFModel(const std::string &path,
                             bool flipUVs = false,
                             bool generateLODs = false,
                             const std::vector<float> &lodLevels = {1.0f, 0.5f, 0.25f, 0.1f});

  ModelAssetID LoadObjModel(const std::string &path,
                            bool flipUVs = true,  // OBJ通常需要翻转UV
                            bool generateLODs = false,
                            const std::vector<float> &lodLevels = {1.0f, 0.5f, 0.25f, 0.1f});

  ModelAssetID LoadModel(const std::string &path,  // 通用格式加载
                         bool flipUVs = false,
                         bool generateLODs = false,
                         const std::vector<float> &lodLevels = {1.0f, 0.5f, 0.25f, 0.1f});

  std::shared_ptr<ModelAsset> GetModel(ModelAssetID id) const;
  void ReleaseModel(ModelAssetID id);

  // ---- 材质管理接口（主要用于内部，也可外部使用） ----
  MaterialAssetID GetOrCreateMaterial(const std::string &name,
                                      const glm::vec3 &color = glm::vec3(1.0f));
  std::shared_ptr<MaterialAsset> GetMaterial(MaterialAssetID id) const;
  void ReleaseMaterial(MaterialAssetID id);

  // ---- 缓存管理 ----
  size_t PurgeUnusedAssets();  // 清理所有未被引用的资源
  size_t GetTextureCount() const
  {
    return m_TextureCache.Size();
  }
  size_t GetModelCount() const
  {
    return m_ModelCache.Size();
  }
  size_t GetMaterialCount() const
  {
    return m_MaterialCache.Size();
  }

  // ---- 禁用拷贝 ----
  AssetManager(const AssetManager &) = delete;
  AssetManager &operator=(const AssetManager &) = delete;

 private:
  // ---- 内部方法 ----
  TextureAssetID LoadTextureInternal(const std::string &path);
  ModelAssetID LoadModelInternal(const std::string &path,
                                 bool flipUVs,
                                 bool generateLODs,
                                 const std::vector<float> &lodLevels,
                                 bool isGLTF = false,
                                 bool isOBJ = false);

  // ---- 成员变量 ----
  TextureCache m_TextureCache;    // 纹理资源缓存
  ModelCache m_ModelCache;        // 模型资源缓存
  MaterialCache m_MaterialCache;  // 材质资源缓存
  mutable std::mutex m_Mutex;     // 线程安全锁
};
};  // namespace mite

#endif
