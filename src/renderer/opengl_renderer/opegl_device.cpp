#include "opengl_renderer/opegl_device.h"

namespace mite {
// ------------------------ 构造函数/析构函数 ------------------------
OpenGLDevice::OpenGLDevice() : IRenderDevice()
{  
  // 创建日志系统
  m_Logger = mite::LoggerSystem::CreateModuleLogger("Mite OpenGL Device");
  m_Logger->trace("Created OpenGL Device");


}

OpenGLDevice::~OpenGLDevice()
{
  // 防御性检查：确保所有资源已释放
  if (!activeTextures_.empty()) {
    LOG_WARN("{} textures not released on shutdown", activeTextures_.size());
    for (GLuint handle : activeTextures_) {
      glDeleteTextures(1, &handle);
    }
  }
  if (!activeMeshsVAO_.empty()) {
    LOG_WARN("{} meshes vao not released on shutdown", activeMeshsVAO_.size());
    for (GLuint vao : activeMeshsVAO_) {
      glDeleteVertexArrays(1, &vao);
    }
  }
  if (!activeMeshsVBO_.empty()) {
    LOG_WARN("{} meshes vbo not released on shutdown", activeMeshsVBO_.size());
    for (GLuint vbo : activeMeshsVBO_) {
      glDeleteBuffers(1, &vbo);
    }
  }
  if (!activeMeshsEBO_.empty()) {
    LOG_WARN("{} meshes ebo not released on shutdown", activeMeshsEBO_.size());
    for (GLuint ebo : activeMeshsEBO_) {
      glDeleteBuffers(1, &ebo);
    }
  }
}

// ------------------------ 纹理操作 ------------------------
TextureGPUHandle OpenGLDevice::CreateTexture(const TextureSourceData &data)
{
  GLuint textureID;
  glGenTextures(1, &textureID);
  glBindTexture(GL_TEXTURE_2D, textureID);

  // 设置纹理参数
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  // 上传纹理数据
  GLenum format = TranslateTextureFormat(data.format);
  GLenum internalFormat = /*!isHDR ? GL_SRGB8_ALPHA8 :*/ format;
  glTexImage2D(GL_TEXTURE_2D,
               0,               // Mipmap级别
               internalFormat,  // 内部格式
               data.width,
               data.height,
               0,                 // 历史遗留参数
               internalFormat,    // 像素数据格式
               GL_UNSIGNED_BYTE,  // 数据类型（HDR需改为GL_FLOAT）
               data.pixelData     // 原始数据指针
  );

  if (data.generateMipmaps) {
    glGenerateMipmap(GL_TEXTURE_2D);
  }

  // 记录活动纹理
  activeTextures_.insert(textureID);

  TextureGPUHandle handle = {static_cast<uintptr_t>(textureID)};
  SetTextureWrapMode(handle, data.wrapMode);
  SetTextureFilterMode(handle, data.filterMode);

  return handle;
}

void OpenGLDevice::DestroyTexture(TextureGPUHandle handle)
{
  if (!handle.apiHandle)
    return;

  GLuint textureID = static_cast<GLuint>(handle.apiHandle);
  glDeleteTextures(1, &textureID);
  activeTextures_.erase(textureID);
}

void OpenGLDevice::BindTexture(TextureGPUHandle handle, uint32_t slot) const
{
  // 渲染时，才需要激活纹理单元
  glActiveTexture(GL_TEXTURE0 + slot);
  glBindTexture(GL_TEXTURE_2D, static_cast<GLuint>(handle.apiHandle));
}
void OpenGLDevice::SetTextureWrapMode(TextureGPUHandle handle, TextureWrapMode mode)
{
  // 初始化纹理时，不需要激活纹理单元
  GLuint glTexture = static_cast<GLuint>(handle.apiHandle);
  glBindTexture(GL_TEXTURE_2D, glTexture);

  GLenum glWrapMode = ConvertWrapMode(mode);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, glWrapMode);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, glWrapMode);

  // 如果是3D纹理则需要设置R轴
  // glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, glWrapMode);
}
void OpenGLDevice::SetTextureFilterMode(TextureGPUHandle handle, TextureFilterMode mode)
{
  GLuint glTexture = static_cast<GLuint>(handle.apiHandle);
  glBindTexture(GL_TEXTURE_2D, glTexture);

  GLenum minFilter, magFilter;
  ConvertFilterMode(mode, minFilter, magFilter);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minFilter);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, magFilter);
}

void OpenGLDevice::GenerateMipmaps(TextureGPUHandle handle)
{
  GLuint glTexture = static_cast<GLuint>(handle.apiHandle);
  glBindTexture(GL_TEXTURE_2D, glTexture);

  glGenerateMipmap(GL_TEXTURE_2D);
}

// ------------------------ 模型操作 ------------------------
ModelGPUHandle OpenGLDevice::CreateModel(const ModelSourceData &data)
{
  ModelGPUHandle modelHandle;

  // 处理所有子网格
  for (auto &subMesh : data.subMeshes) {
    MeshGPUHandle subMeshHandle = CreateSubMesh(subMesh);
    modelHandle.subMeshes.push_back(subMeshHandle);
  }

  return modelHandle;
}

MeshGPUHandle OpenGLDevice::CreateSubMesh(const MeshSourceData &subMesh)
{
  MeshGPUHandle handle;
  GLuint VBO, EBO, VAO;

  // TODO: 
  // 为了减少GPU内存碎片，应当为Model创建统一的VAO内存，
  // 各个MeshGPUHandle通过存放Model的VAO和各自的Offset，
  // 在DrawIndexed函数内通过Offset索引进行绘制。

  // 1. 创建VAO
  glGenVertexArrays(1, &VAO);
  glBindVertexArray(VAO);

  // 2. 创建VBO并上传数据
  glGenBuffers(1, &VBO);
  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferData(GL_ARRAY_BUFFER,
               subMesh.vertexCount * subMesh.layout.stride,
               subMesh.vertexData,
               GL_STATIC_DRAW);

  // 3. 创建EBO并上传数据
  glGenBuffers(1, &EBO);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER,
               subMesh.indexCount * sizeof(uint32_t),
               subMesh.indices,
               GL_STATIC_DRAW);

  // 4. 设置顶点属性指针
  size_t offset = 0;
  for (uint32_t i = 0; i < subMesh.layout.attributes.size(); ++i) {
    auto attr = subMesh.layout.attributes[i];
    glEnableVertexAttribArray(i);

    switch (attr) {
      case VertexAttribute::Position:
        glVertexAttribPointer(
            i, 3, GL_FLOAT, GL_FALSE, subMesh.layout.stride, reinterpret_cast<void *>(offset));
        offset += sizeof(glm::vec3);
        break;
      case VertexAttribute::Normal:
        glVertexAttribPointer(
            i, 3, GL_FLOAT, GL_FALSE, subMesh.layout.stride, reinterpret_cast<void *>(offset));
        offset += sizeof(glm::vec3);
        break;
      case VertexAttribute::TexCoord:
        glVertexAttribPointer(
            i, 2, GL_FLOAT, GL_FALSE, subMesh.layout.stride, reinterpret_cast<void *>(offset));
        offset += sizeof(glm::vec2);
        break;
      case VertexAttribute::Tangent:
        glVertexAttribPointer(
            i, 3, GL_FLOAT, GL_FALSE, subMesh.layout.stride, reinterpret_cast<void *>(offset));
        offset += sizeof(glm::vec3);
        break;
      case VertexAttribute::Bitangent:
        glVertexAttribPointer(
            i, 3, GL_FLOAT, GL_FALSE, subMesh.layout.stride, reinterpret_cast<void *>(offset));
        offset += sizeof(glm::vec3);
        break;
    }
  }

  // 解绑VAO
  glBindVertexArray(0);

  // 填充GPU句柄
  handle.vertexArray = static_cast<uintptr_t>(VAO);
  handle.vertexBuffer = static_cast<uintptr_t>(VBO);
  handle.indexBuffer = static_cast<uintptr_t>(EBO);
  handle.vertexCount = subMesh.vertexCount;
  handle.indexCount = subMesh.indexCount;

  // 记录活动网格（调试用）
  activeMeshsVAO_.insert(VAO);
  activeMeshsVBO_.insert(VBO);
  activeMeshsEBO_.insert(EBO);

  return handle;
}

void OpenGLDevice::DestroyModel(ModelGPUHandle handle)
{
  if (handle.subMeshes.empty())
    return;

  for (auto &subMeshHandle : handle.subMeshes) {
    GLuint vao = static_cast<GLuint>(subMeshHandle.vertexArray);
    GLuint vbo = static_cast<GLuint>(subMeshHandle.vertexBuffer);
    GLuint ebo = static_cast<GLuint>(subMeshHandle.indexBuffer);

    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &vbo);
    glDeleteBuffers(1, &ebo);

    // 从活动模型中移除
    activeMeshsVAO_.erase(vao);
    activeMeshsVBO_.erase(vbo);
    activeMeshsEBO_.erase(ebo);
  }
}

void OpenGLDevice::BindMesh(MeshGPUHandle handle) const
{
  if (handle.vertexArray == 0) {
    LOG_WARN("Attempted to bind invalid mesh (VAO=0)");
    return;
  }

  // 绑定顶点数组对象（VAO）
  GLuint vao = static_cast<GLuint>(handle.vertexArray);
  glBindVertexArray(vao);

  // 注：VAO已包含VBO/EBO的绑定信息，无需重复绑定
}

void OpenGLDevice::DrawIndexed(uint32_t indexCount, uint32_t indexOffset) const
{
  if (indexCount == 0) {
    LOG_WARN("Attempted to draw with indexCount=0");
    return;
  }

  // 执行索引绘制
  glDrawElements(GL_TRIANGLES,                                             // 绘制模式
                 indexCount,                                               // 索引数量
                 GL_UNSIGNED_INT,                                          // 索引类型
                 reinterpret_cast<void *>(indexOffset * sizeof(uint32_t))  // 偏移量
  );

  // 调试用：检查OpenGL错误
  GLenum err = glGetError();
  if (err != GL_NO_ERROR) {
    LOG_ERROR("OpenGL draw error: {}", static_cast<int>(err));
  }
}

// ------------------------ 辅助方法 ------------------------
GLenum OpenGLDevice::TranslateTextureFormat(TextureFormat format)
{
  switch (format) {
    case TextureFormat::RGB8:
      return GL_RGB;
    case TextureFormat::RGBA8:
      return GL_RGBA;
    case TextureFormat::RGB16F:
      return GL_RGB16F;
    case TextureFormat::RGBA16F:
      return GL_RGBA16F;
    default:
      LOG_WARN("Unsupported texture format: {}", static_cast<int>(format));
      return GL_RGBA;  // 默认回退
  }
}

void OpenGLDevice::OnModelLoaded(ModelLoadEvent &e) {
  std::shared_ptr<ModelAsset> model = e.GetModelAsset();

  // 转换 Asset 模块数据为 Renderer 模块的 ModelSourceData
  ModelSourceData rendererData;
  rendererData.modelBboxMin = model->metadata.boundingBoxMin;
  rendererData.modelBboxMax = model->metadata.boundingBoxMax;

  for (const auto &subMesh : model->subMeshes) {
    rendererData.subMeshes.push_back(
        {subMesh.vertexData.data(),
         subMesh.indices.data(),
         static_cast<uint32_t>(subMesh.vertexData.size() / subMesh.layout.stride),
         static_cast<uint32_t>(subMesh.indices.size()),
         subMesh.layout,
         subMesh.boundingBoxMin,
         subMesh.boundingBoxMax});
  }

  // 创建model的GPU资源
  model->handle = CreateModel(rendererData);
}

void OpenGLDevice::OnTextureLoaded(TextureLoadEvent &e) {
  std::shared_ptr<TextureAsset> textureAsset = e.GetTextureAsset();

  // 转换 Asset 模块数据为 Renderer 模块的 ModelSourceData
  TextureSourceData rendererData;
  rendererData.pixelData = textureAsset->textureData.textureData.get();
  rendererData.width = textureAsset->metadata.width;
  rendererData.height = textureAsset->metadata.height;
  rendererData.format = textureAsset->metadata.format;
  rendererData.wrapMode = TextureWrapMode::Repeat;  // 默认值或从配置读取
  rendererData.filterMode = TextureFilterMode::Linear;
  rendererData.generateMipmaps = true;

  textureAsset->handle = CreateTexture(rendererData);
}

void OpenGLDevice::OnMeshDraw(MeshDrawEvent &e) {
  MeshGPUHandle handle = e.GetHandle();

  BindMesh(handle);
  DrawIndexed(handle.indexCount, 0);  // 从索引0开始绘制（由于当前Mesh的VAO创建是独立的，所以Model的每个子Mesh都是从0开始）
}

void OpenGLDevice::OnTextureBind(TextureBindEvent &e)
{
  BindTexture(e.GetHandle(), e.GetSlot());
}

void OpenGLDevice::OnTextureSetWrapMode(TextureWrapModeEvent &e)
{
  SetTextureWrapMode(e.GetHandle(), e.GetMode());
}

void OpenGLDevice::OnTextureSetFilterMode(TextureFilterModeEvent &e)
{
  SetTextureFilterMode(e.GetHandle(), e.GetMode());
}

void OpenGLDevice::OnTextureGenerateMipmaps(TextureGenerateMipmapsEvent &e)
{
  GenerateMipmaps(e.GetHandle());
}

GLenum OpenGLDevice::ConvertWrapMode(TextureWrapMode mode) const
{
  switch (mode) {
    case TextureWrapMode::Repeat:
      return GL_REPEAT;
    case TextureWrapMode::ClampToEdge:
      return GL_CLAMP_TO_EDGE;
    case TextureWrapMode::MirroredRepeat:
      return GL_MIRRORED_REPEAT;
    default:
      assert(false && "Unknown wrap mode");
      return GL_REPEAT;
  }
}
void OpenGLDevice::ConvertFilterMode(TextureFilterMode mode,
                                     GLenum &outMinFilter,
                                     GLenum &outMagFilter) const
{
  switch (mode) {
    case TextureFilterMode::Nearest:
      outMinFilter = GL_NEAREST;
      outMagFilter = GL_NEAREST;
      break;

    case TextureFilterMode::Linear:
      outMinFilter = GL_LINEAR;
      outMagFilter = GL_LINEAR;
      break;

    case TextureFilterMode::Anisotropic:
      outMinFilter = GL_LINEAR_MIPMAP_LINEAR;  // 需要mipmap
      outMagFilter = GL_LINEAR;

      // 设置各向异性过滤（需检查扩展支持）
      if (GL_EXT_texture_filter_anisotropic) {
        float maxAniso = 0.0f;
        glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &maxAniso);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, maxAniso);
      }
      break;

    default:
      assert(false && "Unknown filter mode");
      outMinFilter = GL_LINEAR;
      outMagFilter = GL_LINEAR;
  }
}
};  // namespace mite