#include "asset_manager.h"
#include "material_loader.h"
#include "texture_loader.h"
#include "model_loader.h"

namespace mite {

AssetManager::~AssetManager()
{
  // 析构时自动清理所有缓存资源
  m_TextureCache.PurgeUnused();
  m_ModelCache.PurgeUnused();
  m_MaterialCache.PurgeUnused();
}

// ===================== 纹理管理 =====================
TextureAssetID AssetManager::LoadTexture(const std::string &path)
{
  std::lock_guard<std::mutex> lock(m_Mutex);
  return LoadTextureInternal(path);
}
TextureAssetID AssetManager::LoadTextureInternal(const std::string &path)
{
  try {
    // 使用新的TextureLoader（支持缓存）
    TextureAssetID textureID = TextureLoader::LoadTexture(m_TextureCache, path);

    if (textureID.IsValid()) {
      LOG_INFO("[AssetManager] Texture loaded and cached: " + path);
      return textureID;
    }
    else {
      LOG_ERROR("[AssetManager] Texture load failed: " + path);
      return TextureAssetID{};
    }
  }
  catch (const std::exception &e) {
    LOG_ERROR("[AssetManager] Texture load exception: " + std::string(e.what()));
    return TextureAssetID{};
  }
}
std::shared_ptr<TextureAsset> AssetManager::GetTexture(TextureAssetID id) const
{
  std::lock_guard<std::mutex> lock(m_Mutex);
  m_TextureCache.AddRefCount(id);  // 增加引用计数
  return m_TextureCache.Get(id);
}
void AssetManager::ReleaseTexture(TextureAssetID id)
{
  std::lock_guard<std::mutex> lock(m_Mutex);
  if (m_TextureCache.Release(id) <= 0) {
    m_TextureCache.ForceRemove(id);
    LOG_DEBUG("[AssetManager] Texture released and removed from cache");
  }
}

// ===================== 模型管理 =====================
ModelAssetID AssetManager::LoadGLTFModel(const std::string &path,
                                         bool flipUVs,
                                         bool generateLODs,
                                         const std::vector<float> &lodLevels)
{
  std::lock_guard<std::mutex> lock(m_Mutex);
  return LoadModelInternal(path, flipUVs, generateLODs, lodLevels, true, false);
}
ModelAssetID AssetManager::LoadObjModel(const std::string &path,
                                        bool flipUVs,
                                        bool generateLODs,
                                        const std::vector<float> &lodLevels)
{
  std::lock_guard<std::mutex> lock(m_Mutex);
  return LoadModelInternal(path, flipUVs, generateLODs, lodLevels, false, true);
}
ModelAssetID AssetManager::LoadModel(const std::string &path,
                                     bool flipUVs,
                                     bool generateLODs,
                                     const std::vector<float> &lodLevels)
{
  std::lock_guard<std::mutex> lock(m_Mutex);
  return LoadModelInternal(path, flipUVs, generateLODs, lodLevels, false, false);
}

ModelAssetID AssetManager::LoadModelInternal(const std::string &path,
                                             bool flipUVs,
                                             bool generateLODs,
                                             const std::vector<float> &lodLevels,
                                             bool isGLTF,
                                             bool isOBJ)
{
  try {
    ModelAssetID modelID;

    if (isGLTF) {
      modelID = ModelLoader::LoadGLTFModel(
          m_ModelCache, m_MaterialCache, m_TextureCache, path, flipUVs, generateLODs, lodLevels);
    }
    else if (isOBJ) {
      modelID = ModelLoader::LoadObjModel(
          m_ModelCache, m_MaterialCache, m_TextureCache, path, flipUVs, generateLODs, lodLevels);
    }
    else {
      modelID = ModelLoader::LoadModel(
          m_ModelCache, m_MaterialCache, m_TextureCache, path, flipUVs, generateLODs, lodLevels);
    }

    if (modelID.IsValid()) {
      LOG_INFO("[AssetManager] Model loaded and cached: " + path);
      return modelID;
    }
    else {
      LOG_ERROR("[AssetManager] Model load failed: " + path);
      return ModelAssetID{};
    }
  }
  catch (const std::exception &e) {
    LOG_ERROR("[AssetManager] Model load exception: " + std::string(e.what()));
    return ModelAssetID{};
  }
}

std::shared_ptr<ModelAsset> AssetManager::GetModel(ModelAssetID id) const
{
  std::lock_guard<std::mutex> lock(m_Mutex);
  m_ModelCache.AddRefCount(id);  // 增加引用计数
  return m_ModelCache.Get(id);
}
void AssetManager::ReleaseModel(ModelAssetID id)
{
  std::lock_guard<std::mutex> lock(m_Mutex);

  // 先获取模型资产，以便释放相关材质引用
  auto model = m_ModelCache.Get(id);
  if (model) {
    // 释放模型引用的所有材质
    for (const auto &materialID : model->materialRefs) {
      ReleaseMaterial(materialID);
    }
  }

  if (m_ModelCache.Release(id) <= 0) {
    m_ModelCache.ForceRemove(id);
    LOG_DEBUG("[AssetManager] Model released and removed from cache");
  }
}

// ===================== 材质管理 =====================
std::shared_ptr<MaterialAsset> AssetManager::GetMaterial(MaterialAssetID id) const
{
  std::lock_guard<std::mutex> lock(m_Mutex);
  return m_MaterialCache.Get(id);
}
void AssetManager::ReleaseMaterial(MaterialAssetID id)
{
  std::lock_guard<std::mutex> lock(m_Mutex);
  if (m_MaterialCache.Release(id) <= 0) {
    m_MaterialCache.ForceRemove(id);
    LOG_DEBUG("[AssetManager] Material released and removed from cache");
  }
}
// ===================== 缓存管理 =====================
size_t AssetManager::PurgeUnusedAssets()
{
  std::lock_guard<std::mutex> lock(m_Mutex);

  size_t texturePurged = m_TextureCache.PurgeUnused();
  size_t modelPurged = m_ModelCache.PurgeUnused();
  size_t materialPurged = m_MaterialCache.PurgeUnused();

  size_t totalPurged = texturePurged + modelPurged + materialPurged;

  if (totalPurged > 0) {
    LOG_INFO("[AssetManager] Purged " + std::to_string(totalPurged) + " unused assets: " +
             std::to_string(texturePurged) + " textures, " + std::to_string(modelPurged) +
             " models, " + std::to_string(materialPurged) + " materials");
  }

  return totalPurged;
}
};
