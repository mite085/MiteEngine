#include "asset_manager.h"

namespace mite {

AssetManager::~AssetManager()
{
  // 析构时自动清理所有缓存资源
  m_TextureCache.PurgeUnused();
  m_ModelCache.PurgeUnused();
}

// ===================== 纹理管理 =====================
std::shared_ptr<TextureAsset> AssetManager::LoadTexture(const std::string &path)
{
  AssetID id = UUIDGenerator::Generate(path.c_str());
  std::lock_guard<std::mutex> lock(mutex_);

  auto cachedTex = m_TextureCache.Get(id);

  // 缓存不存在则执行加载
  if (!cachedTex) {
    LoadTextureInternalToCache(path);
    cachedTex = m_TextureCache.Get(id);
  }

  m_TextureCache.AddRefCount(id);  // 增加引用计数
  return cachedTex;
  
}

void AssetManager::LoadTextureInternalToCache(const std::string &path)
{
  try {
    // 1. 使用TextureLoader加载原始数据
    auto [metadata, pixelData] = TextureLoader::LoadTextureData(path);

    // 2. 缓存资源（pixeldata的所有权转让给TextureAsset）
    AssetID id = UUIDGenerator::Generate(path.c_str());
    auto tex = std::make_shared<TextureAsset>(TextureAsset{id, metadata, {std::move(pixelData)}});
    m_TextureCache.Store(tex);
  }
  catch (const std::exception &e) {
    LOG_ERROR("[AssetManager] Texture load failed: {}", e.what());
  }
}

void AssetManager::ReleaseTexture(AssetID id)
{
  std::lock_guard<std::mutex> lock(mutex_);
  if (m_TextureCache.Release(id) <= 0) {
    m_TextureCache.ForceRemove(id);
  }
}

// ===================== 模型管理 =====================
std::shared_ptr<ModelAsset> AssetManager::LoadModel(const std::string &path)
{
  AssetID id = UUIDGenerator::Generate(path.c_str());
  std::lock_guard<std::mutex> lock(mutex_);

  auto cachedModel = m_ModelCache.Get(id);

  // 缓存不存在则执行加载
  if (!cachedModel) {
    LoadModelInternalToCache(path);
    cachedModel = m_ModelCache.Get(id);
  }

  m_ModelCache.AddRefCount(id);  // 增加引用计数
  return cachedModel;
}

void AssetManager::LoadModelInternalToCache(const std::string &path)
{
  try {
    // 1. 使用ModelLoader加载模型数据
   std::shared_ptr<ModelAsset> model = ModelLoader::LoadModel(path);

    // 2. 缓存资源
    AssetID id = UUIDGenerator::Generate(path.c_str());
    m_ModelCache.Store(model);
  }
  catch (const std::exception &e) {
    LOG_ERROR("[AssetManager] Model load failed: {}", e.what());
  }
}

void AssetManager::ReleaseModel(AssetID id)
{
  std::lock_guard<std::mutex> lock(mutex_);
  if (m_ModelCache.Release(id) <= 0) {
    m_ModelCache.ForceRemove(id);
  }
}

};
