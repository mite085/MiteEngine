#include "opengl_renderer.h"

namespace mite {
OpenGLRenderer::OpenGLRenderer(std::shared_ptr<AssetManager> assetManager)
    : Renderer(assetManager)
{
}

OpenGLRenderer::~OpenGLRenderer() {
  textures_.clear();
  models_.clear();
}

void OpenGLRenderer::Initialize()
{ 
  // 初始化OpenGL默认状态
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_CULL_FACE);
  defaultFBO_ = 0;  // 默认帧缓冲
}

// ===================== 资源管理 =====================
void OpenGLRenderer::LoadTexture(AssetID id)
{
  if (textures_.find(id) != textures_.end())
    return;

  // 从AssetManager获取元数据
  auto textureAsset = assetManager_->GetTexture(id);
  if (!textureAsset)
    return;

  // 通过OpenGLDevice创建GPU资源
  TextureGPUHandle gpuHandle = IRenderDevice::Current().CreateTexture(*textureAsset);

  // 创建Texture对象封装状态
  textures_.emplace(id, std::make_unique<Texture>(gpuHandle, textureAsset->metadata));
}

void OpenGLRenderer::UnloadTexture(AssetID id)
{
  if (auto it = textures_.find(id); it != textures_.end()) {
    IRenderDevice::Current().DestroyTexture(it->second->GetHandle());
    textures_.erase(it);
  }
}

void OpenGLRenderer::LoadModel(AssetID id)
{
  if (models_.find(id) != models_.end())
    return;

  auto modelAsset = assetManager_->GetModel(id);
  if (!modelAsset)
    return;

  // 创建GPU资源并构建Mesh对象
  ModelGPUHandle gpuHandle = IRenderDevice::Current().CreateModel(*modelAsset);
  models_.emplace(id, std::make_unique<Model>(gpuHandle, modelAsset->metadata));
}
void OpenGLRenderer::UnloadModel(AssetID id)
{
  // 查找目标模型
  auto it = models_.find(id);
  if (it == models_.end()) {
    LOG_WARN("Attempted to unload non-existent model: {}", uuids::to_string(id));
    return;
  }

  // 1. 通过IRenderDevice释放GPU资源
  const ModelGPUHandle &gpuHandle = it->second->GetHandle();
  IRenderDevice::Current().DestroyModel(gpuHandle);

  // 2. 移除本地存储的Mesh对象
  models_.erase(it);

  // 3. 通知AssetManager减少引用计数（可选，根据实际架构决定）
  assetManager_->ReleaseModel(id);

  LOG_DEBUG("Unloaded model: {}", uuids::to_string(id));
}
// ===================== 渲染指令 =====================
void OpenGLRenderer::BeginFrame()
{
  glClearColor(clearColor_.r, clearColor_.g, clearColor_.b, clearColor_.a);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glViewport(0, 0, viewportSize_.x, viewportSize_.y);
}

void OpenGLRenderer::EndFrame() {
  // 确保所有命令提交
  glFlush();

  // 重置OpenGL状态机
  glBindVertexArray(0);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glUseProgram(0);
  glActiveTexture(GL_TEXTURE0);

  // 注意：不包含交换缓冲区的操作，由窗口系统负责
}

void OpenGLRenderer::DrawModel(AssetID modelId, const glm::mat4 &transform)
{
  if (auto it = models_.find(modelId); it != models_.end()) {
    // TODO: 绑定Shader/Uniforms (伪代码)
    //shader_->SetMat4("u_model", transform);

    // 绘制所有子网格
    for (size_t i = 0; i < it->second->GetSubMeshCount(); ++i) {
      // TODO: 绑定材质（关联的纹理等）
      //BindMaterial(modelId, i);

      // 绘制子网格
      it->second->DrawSubMesh(i);
    }
  }
}
}  // namespace mite