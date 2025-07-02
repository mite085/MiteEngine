#include "asset_manager.h"

namespace mite {
// ------------------------ 构造函数 ------------------------
AssetManager::AssetManager(IRenderDevice *renderDevice) : renderDevice_(renderDevice)
{
  if (!renderDevice_) {
    throw std::invalid_argument("RenderDevice cannot be null!");
  }
}

AssetManager::~AssetManager()
{
  // 析构时自动清理所有缓存资源
  textureCache_.PurgeUnused();
  modelCache_.PurgeUnused();
}

// ===================== 纹理管理 =====================
std::shared_ptr<TextureAsset> AssetManager::LoadTexture(const std::string &path)
{
  const AssetID id = UUIDGenerator::Generate(path.c_str());
  std::lock_guard<std::mutex> lock(mutex_);

  auto cachedTex = textureCache_.Get(id);

  // 缓存不存在则执行加载
  if (!cachedTex) {
    LoadTextureInternalToCache(path);
    cachedTex = textureCache_.Get(id);
  }

  textureCache_.AddRefCount(id);  // 增加引用计数
  return cachedTex;
  
}

void AssetManager::LoadTextureInternalToCache(const std::string &path)
{
  try {
    // 1. 使用TextureLoader加载原始数据
    auto [metadata, pixelData] = TextureLoader::LoadTextureData(path);

    // 2. 委托IRenderDevice创建GPU资源
    TextureGPUHandle gpuHandle = renderDevice_->CreateTexture(metadata, pixelData.get());

    // 3. 缓存资源
    const AssetID id = UUIDGenerator::Generate(path.c_str());
    auto tex = std::make_shared<TextureAsset>(id, metadata, gpuHandle);
    textureCache_.Store(tex);
  }
  catch (const std::exception &e) {
    LOG_ERROR("[AssetManager] Texture load failed: {}", e.what());
  }
}

void AssetManager::ReleaseTexture(TextureAsset handle)
{
  std::lock_guard<std::mutex> lock(mutex_);
  if (textureCache_.Release(handle.id) <= 0) {
    // 如果引用归零，通知RenderDevice释放GPU资源
    if (auto tex = textureCache_.Get(handle.id)) {
      renderDevice_->DestroyTexture(tex->gpuHandle);
    }
    textureCache_.ForceRemove(handle.id);
  }
}

// ===================== 模型管理 =====================
std::shared_ptr<ModelAsset> AssetManager::LoadModel(const std::string &path)
{
  const AssetID id = UUIDGenerator::Generate(path.c_str());
  std::lock_guard<std::mutex> lock(mutex_);

  auto cachedModel = modelCache_.Get(id);

  // 缓存不存在则执行加载
  if (!cachedModel) {
    LoadModelInternalToCache(path);
    cachedModel = modelCache_.Get(id);
  }

  modelCache_.AddRefCount(id);  // 增加引用计数
  return cachedModel;
}

void AssetManager::LoadModelInternalToCache(const std::string &path)
{
  try {
    // 1. 使用ModelLoader加载模型数据
    ModelMetadata metadata = ModelLoader::LoadModel(path);

    // 2. 委托IRenderDevice创建GPU资源
    ModelGPUHandle gpuHandle = renderDevice_->CreateModel(metadata);

    // 3. 缓存资源
    const AssetID id = UUIDGenerator::Generate(path.c_str());
    auto modelMeta = std::make_shared<ModelAsset>(id, metadata, gpuHandle);
    modelCache_.Store(modelMeta);
  }
  catch (const std::exception &e) {
    LOG_ERROR("[AssetManager] Model load failed: {}", e.what());
  }
}

void AssetManager::ReleaseModel(ModelAsset handle)
{
  std::lock_guard<std::mutex> lock(mutex_);
  if (modelCache_.Release(handle.id) <= 0) {
    if (auto model = modelCache_.Get(handle.id)) {
      renderDevice_->DestroyModel(model->gpuHandle);
    }
    modelCache_.ForceRemove(handle.id);
  }
}

};
