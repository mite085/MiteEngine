#include "asset_manager.h"

namespace mite {
AssetManager::AssetManager(IRenderDevice &renderDevice) : m_RenderDevice(renderDevice) {}
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

    // 2. 委托IRenderDevice创建GPU资源
    TextureGPUHandle gpuHandle = m_RenderDevice.CreateTexture(metadata, pixelData.get());

    // 3. 缓存资源
    AssetID id = UUIDGenerator::Generate(path.c_str());
    auto tex = std::make_shared<TextureAsset>(TextureAsset{id, metadata, gpuHandle});
    m_TextureCache.Store(tex);
  }
  catch (const std::exception &e) {
    LOG_ERROR("[AssetManager] Texture load failed: {}", e.what());
  }
}

void AssetManager::ReleaseTexture(TextureAsset handle)
{
  std::lock_guard<std::mutex> lock(mutex_);
  if (m_TextureCache.Release(handle.id) <= 0) {
    // 如果引用归零，通知RenderDevice释放GPU资源
    if (auto tex = m_TextureCache.Get(handle.id)) {
      m_RenderDevice.DestroyTexture(tex->gpuHandle);
    }
    m_TextureCache.ForceRemove(handle.id);
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
    ModelMetadata metadata = ModelLoader::LoadModel(path);

    // 2. 委托IRenderDevice创建GPU资源
    ModelGPUHandle gpuHandle = m_RenderDevice.CreateModel(metadata);

    // 3. 缓存资源
    AssetID id = UUIDGenerator::Generate(path.c_str());
    auto modelMeta = std::make_shared<ModelAsset>(ModelAsset{id, metadata, gpuHandle});
    m_ModelCache.Store(modelMeta);
  }
  catch (const std::exception &e) {
    LOG_ERROR("[AssetManager] Model load failed: {}", e.what());
  }
}

void AssetManager::ReleaseModel(ModelAsset handle)
{
  std::lock_guard<std::mutex> lock(mutex_);
  if (m_ModelCache.Release(handle.id) <= 0) {
    if (auto model = m_ModelCache.Get(handle.id)) {
      m_RenderDevice.DestroyModel(model->gpuHandle);
    }
    m_ModelCache.ForceRemove(handle.id);
  }
}

};
