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
  std::lock_guard<std::mutex> lock(cacheMutex_);

  // 清理所有残留资源（理论上引用计数应已归零）
  for (auto &[id, tex] : textureCache_) {
    renderDevice_->DestroyTexture(tex.gpuHandle);
  }
  for (auto &[id, model] : modelCache_) {
    renderDevice_->DestroyModel(model.gpuHandle);
  }
}

// ===================== 纹理管理 =====================
TextureHandle AssetManager::LoadTexture(const std::string &path)
{
  const AssetID id = UUIDGenerator::Generate();
  std::lock_guard<std::mutex> lock(cacheMutex_);

  // 检查缓存
  auto texIt = textureCache_.find(id);
  if (texIt != textureCache_.end()) {
    texIt->second.refCount++;
    return {id, texIt->second.gpuHandle};
  }

  // 实际加载
  return LoadTextureInternal(path);
}

TextureHandle AssetManager::LoadTextureInternal(const std::string &path)
{
  try {
    // 1. 使用TextureLoader加载原始数据
    auto [metadata, pixelData] = TextureLoader::LoadTextureData(path);

    // 2. 填充通用纹理描述
    TextureMetadata apiMetadata;
    apiMetadata.path = path;
    apiMetadata.width = metadata.width;
    apiMetadata.height = metadata.height;
    apiMetadata.format = metadata.channels == 4 ? TextureFormat::RGBA8 : TextureFormat::RGB8;

    // 3. 委托IRenderDevice创建GPU资源
    TextureGPUHandle gpuHandle = renderDevice_->CreateTexture(apiMetadata, pixelData.get());

    // 4. 缓存资源
    const AssetID id = UUIDGenerator::Generate();
    textureCache_[id] = TextureAsset{
        std::make_shared<TextureMetadata>(std::move(apiMetadata)), gpuHandle, 1};

    return {id, gpuHandle};
  }
  catch (const std::exception &e) {
    // 日志记录实际错误
    LOG_ERROR("Texture load failed: {}", e.what());
    return {};
  }
}

void AssetManager::ReleaseTexture(TextureHandle handle)
{
  std::lock_guard<std::mutex> lock(cacheMutex_);
  auto it = textureCache_.find(handle.id);
  if (it == textureCache_.end()) {
    LOG_WARN("Attempted to release unknown texture: {}", to_string(handle.id));
    return;
  }

  if (--it->second.refCount <= 0) {
    renderDevice_->DestroyTexture(it->second.gpuHandle);
    textureCache_.erase(it);
  }
}

// ===================== 模型管理 =====================
ModelHandle AssetManager::LoadModel(const std::string &path)
{
  const AssetID id = UUIDGenerator::Generate();
  std::lock_guard<std::mutex> lock(cacheMutex_);

  auto modelIt = modelCache_.find(id);
  if (modelIt != modelCache_.end()) {
    modelIt->second.refCount++;
    return {id, modelIt->second.gpuHandle};
  }

  return LoadModelInternal(path);
}

ModelHandle AssetManager::LoadModelInternal(const std::string &path)
{
  try {
    // 1. 使用ModelLoader加载模型数据
    ModelMetadata metadata = ModelLoader::LoadModel(path);

    // 2. 委托IRenderDevice创建GPU资源
    ModelGPUHandle gpuHandle = renderDevice_->CreateModel(metadata);

    // 3. 缓存资源
    const AssetID id = UUIDGenerator::Generate();
    modelCache_[id] = ModelAsset{
        std::make_shared<ModelMetadata>(std::move(metadata)), gpuHandle, 1};

    return {id, gpuHandle};
  }
  catch (const std::exception &e) {
    LOG_ERROR("Model load failed: {}", e.what());
    return {};
  }
}

void AssetManager::ReleaseModel(ModelHandle handle)
{

  std::lock_guard<std::mutex> lock(cacheMutex_);
  auto it = modelCache_.find(handle.id);
  if (it == modelCache_.end()) {
    LOG_WARN("Attempted to release unknown model: {}", to_string(handle.id));
    return;
  }

  if (--it->second.refCount <= 0) {
    renderDevice_->DestroyModel(it->second.gpuHandle);
    modelCache_.erase(it);
  }
}

// ===================== 元数据查询 =====================
const TextureMetadata *AssetManager::GetTextureMetadata(TextureHandle handle) const
{
  std::lock_guard<std::mutex> lock(cacheMutex_);
  auto it = textureCache_.find(handle.id);
  return it != textureCache_.end() ? it->second.metadata.get() : nullptr;
}

const ModelMetadata *AssetManager::GetModelMetadata(ModelHandle handle) const
{
  std::lock_guard<std::mutex> lock(cacheMutex_);
  auto it = modelCache_.find(handle.id);
  return it != modelCache_.end() ? it->second.metadata.get() : nullptr;
}
};
