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
    m_Logger->warn("{} textures not released on shutdown", activeTextures_.size());
    for (GLuint handle : activeTextures_) {
      glDeleteTextures(1, &handle);
    }
  }
  if (!activeModelsVAO_.empty()) {
    m_Logger->warn("{} meshes vao not released on shutdown", activeModelsVAO_.size());
    for (GLuint vao : activeModelsVAO_) {
      glDeleteVertexArrays(1, &vao);
    }
  }
  if (!activeModelsVBO_.empty()) {
    m_Logger->warn("{} meshes vbo not released on shutdown", activeModelsVBO_.size());
    for (GLuint vbo : activeModelsVBO_) {
      glDeleteBuffers(1, &vbo);
    }
  }
  if (!activeModelsEBO_.empty()) {
    m_Logger->warn("{} meshes ebo not released on shutdown", activeModelsEBO_.size());
    for (GLuint ebo : activeModelsEBO_) {
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
  // 0. 创建临时VAO等对象
  GLuint VBO, EBO, VAO;

  // 1. 创建VAO
  glGenVertexArrays(1, &VAO);
  glBindVertexArray(VAO);

  // 2. 创建VBO并上传数据
  glGenBuffers(1, &VBO);
  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferData(
      GL_ARRAY_BUFFER, data.mergedVertexData.size(), data.mergedVertexData.data(), GL_STATIC_DRAW);

  // 3. 创建EBO并上传数据
  glGenBuffers(1, &EBO);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER,
               data.mergedIndices.size() * sizeof(uint32_t),
               data.mergedIndices.data(),
               GL_STATIC_DRAW);

  // 4. 设置顶点属性指针(基于统一的layout)
  SetVertexAttributes(data.layout);

  // 5. 解绑VAO
  glBindVertexArray(0);

  // 6. 填充GPU句柄
  ModelGPUHandle handle;
  handle.bboxMax = data.modelBboxMax;
  handle.bboxMin = data.modelBboxMin;
  handle.vertexArray = static_cast<uintptr_t>(VAO);
  handle.vertexBuffer = static_cast<uintptr_t>(VBO);
  handle.indexBuffer = static_cast<uintptr_t>(EBO);

  // 7. 记录活动网格（调试用）
  activeModelsVAO_.insert(VAO);
  activeModelsVBO_.insert(VBO);
  activeModelsEBO_.insert(EBO);

  // 8. 保存ModelSourceData创建时生成的MeshSections
  handle.subMeshes = std::move(data.sections);

  return handle;
}

void OpenGLDevice::DestroyModel(ModelGPUHandle handle)
{
  // 防御性检查
  if (handle.vertexArray == 0 && handle.vertexBuffer == 0 && handle.indexBuffer == 0) {
    m_Logger->warn("Attempted to destroy invalid ModelGPUHandle (all handles are 0)");
    return;
  }

  // 1. 删除顶点数组对象(VAO)
  if (handle.vertexArray != 0) {
    GLuint vao = static_cast<GLuint>(handle.vertexArray);
    glDeleteVertexArrays(1, &vao);

    // 从活动资源中移除
    activeModelsVAO_.erase(vao);
  }

  // 2. 删除顶点缓冲区(VBO)
  if (handle.vertexBuffer != 0) {
    GLuint vbo = static_cast<GLuint>(handle.vertexBuffer);
    glDeleteBuffers(1, &vbo);

    // 从活动资源中移除
    activeModelsVBO_.erase(vbo);
  }

  // 3. 删除索引缓冲区(EBO)
  if (handle.indexBuffer != 0) {
    GLuint ebo = static_cast<GLuint>(handle.indexBuffer);
    glDeleteBuffers(1, &ebo);

    // 从活动资源中移除
    activeModelsEBO_.erase(ebo);
  }

  // 4. 调试日志
  m_Logger->debug("Destroyed model resources: VAO={}, VBO={}, EBO={}",
                  handle.vertexArray,
                  handle.vertexBuffer,
                  handle.indexBuffer);

  // 5. 清空句柄(防御性编程)
  handle.vertexArray = 0;
  handle.vertexBuffer = 0;
  handle.indexBuffer = 0;
}

void OpenGLDevice::BindMesh(std::shared_ptr<Mesh> mesh) const
{
  std::shared_ptr<ModelGPUHandle> modelHandle = mesh->GetModelHandle();
  MeshSection meshSection = mesh->GetSection();

  // 1. 参数有效性检查
  if (!modelHandle) {
    m_Logger->warn("Attempt to bind mesh with null model handle");
    return;
  }

  if (modelHandle->vertexArray == 0) {
    m_Logger->warn("Attempt to bind mesh with invalid VAO (handle=0)");
    return;
  }

  if (meshSection.indexCount == 0 || meshSection.vertexCount == 0) {
    m_Logger->warn("Attempt to bind empty mesh section (indices={}, vertices={})",
                   meshSection.indexCount,
                   meshSection.vertexCount);
    return;
  }

  // 2. 绑定整个模型的VAO
  GLuint vao = static_cast<GLuint>(modelHandle->vertexArray);
  glBindVertexArray(vao);

  // 3. 验证缓冲区是否有效
  if (modelHandle->vertexBuffer == 0 || modelHandle->indexBuffer == 0) {
    m_Logger->error("Model buffers not initialized (VBO={}, EBO={})",
                    modelHandle->vertexBuffer,
                    modelHandle->indexBuffer);
  }

  // 4. 绑定缓冲区（VAO已包含这些信息，但显式绑定更安全）
  glBindBuffer(GL_ARRAY_BUFFER, static_cast<GLuint>(modelHandle->vertexBuffer));
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, static_cast<GLuint>(modelHandle->indexBuffer));

  // 5. 存储当前绑定的MeshSection（供后续Draw调用使用）
  // 注意：这需要OpenGLDevice有成员变量存储当前状态，或者使用其他状态管理机制
  // m_CurrentMeshSection = &meshSection;
  // m_CurrentModelHandle = modelHandle;

  // 6. 调试信息
  m_Logger->debug("Bound mesh: VAO={}, VBO={}, EBO={}, indexOffset={}, vertexOffset={}",
                  vao,
                  modelHandle->vertexBuffer,
                  modelHandle->indexBuffer,
                  meshSection.indexOffset,
                  meshSection.vertexOffset);
}

void OpenGLDevice::DrawIndexed(uint32_t indexCount, uint32_t indexOffset) const
{
  if (indexCount == 0) {
    m_Logger->warn("Attempted to draw with indexCount=0");
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
    m_Logger->error("OpenGL draw error: {}", static_cast<int>(err));
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
      m_Logger->warn("Unsupported texture format: {}", static_cast<int>(format));
      return GL_RGBA;  // 默认回退
  }
}

void OpenGLDevice::OnModelLoaded(ModelLoadEvent &e)
{
  std::shared_ptr<ModelAsset> model = e.GetModelAsset();

  // 1. 准备合并所有子网格数据
  ModelSourceData rendererData;
  rendererData.modelBboxMin = model->metadata.boundingBoxMin;
  rendererData.modelBboxMax = model->metadata.boundingBoxMax;
  rendererData.layout = model->subMeshData.empty() ? VertexLayout{} : model->subMeshData[0].layout;

  // 2. 合并顶点和索引数据
  size_t totalVertexBytes = 0;
  size_t totalIndices = 0;

  // 预计算总大小
  for (const auto &subMesh : model->subMeshData) {
    totalVertexBytes += subMesh.vertexData.size();
    totalIndices += subMesh.indices.size();
  }

  // 预分配空间
  rendererData.mergedVertexData.reserve(totalVertexBytes);
  rendererData.mergedIndices.reserve(totalIndices);

  // 3. 实际合并数据并记录MeshSection
  uint32_t vertexOffset = 0;
  uint32_t indexOffset = 0;

  for (const auto &subMesh : model->subMeshData) {
    // 添加顶点数据
    size_t prevVertexSize = rendererData.mergedVertexData.size();
    rendererData.mergedVertexData.insert(
        rendererData.mergedVertexData.end(), subMesh.vertexData.begin(), subMesh.vertexData.end());

    // 添加索引数据(需要调整偏移)
    size_t prevIndexSize = rendererData.mergedIndices.size();
    rendererData.mergedIndices.insert(
        rendererData.mergedIndices.end(), subMesh.indices.begin(), subMesh.indices.end());

    // 计算顶点数(基于stride)
    uint32_t vertexCount = static_cast<uint32_t>(subMesh.vertexData.size() /
                                                 subMesh.layout.stride);

    // 记录并保存MeshSection，由CreateModel步骤交付给ModelGPUHandle
    rendererData.sections.emplace_back(MeshSection{vertexOffset,
                                                   indexOffset,
                                                   vertexCount,
                                                   static_cast<uint32_t>(subMesh.indices.size()),
                                                   subMesh.boundingBoxMin,
                                                   subMesh.boundingBoxMax});

    // 更新偏移量
    vertexOffset = static_cast<uint32_t>(rendererData.mergedVertexData.size() /
                                         subMesh.layout.stride);
    indexOffset = static_cast<uint32_t>(rendererData.mergedIndices.size());
  }

  // 4. 创建GPU资源
  ModelGPUHandle modelHandle = CreateModel(rendererData);

  // 5. 更新ModelAsset
  model->handle = std::make_shared<ModelGPUHandle>(modelHandle);
}

void OpenGLDevice::OnTextureLoaded(TextureLoadEvent &e)
{
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
      m_Logger->error("Unknown wrap mode");
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
      m_Logger->error("Unknown filter mode");
      outMinFilter = GL_LINEAR;
      outMagFilter = GL_LINEAR;
  }
}

void OpenGLDevice::SetVertexAttributes(const VertexLayout &layout)
{
  // 确保VAO已绑定
  GLint currentVAO;
  glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &currentVAO);
  if (currentVAO == 0) {
    m_Logger->error("No VAO bound when setting vertex attributes");
    return;
  }

  // 计算总步长并验证
  const uint32_t stride = layout.stride;
  if (stride == 0) {
    m_Logger->error("Invalid vertex layout: stride is zero");
    return;
  }

  // 设置每个顶点属性
  uint32_t offset = 0;
  for (uint32_t i = 0; i < layout.attributes.size(); ++i) {
    const auto &attr = layout.attributes[i];

    // 启用顶点属性数组
    glEnableVertexAttribArray(i);

    // 根据属性类型设置指针
    switch (attr) {
      case VertexAttribute::Position:
        glVertexAttribPointer(i,                         // 属性位置
                              3,                         // 分量数量 (vec3)
                              GL_FLOAT,                  // 数据类型
                              GL_FALSE,                  // 是否标准化
                              stride,                    // 步长
                              (void *)(uintptr_t)offset  // 偏移量
        );
        offset += sizeof(glm::vec3);
        break;

      case VertexAttribute::Normal:
        glVertexAttribPointer(i, 3, GL_FLOAT, GL_FALSE, stride, (void *)(uintptr_t)offset);
        offset += sizeof(glm::vec3);
        break;

      case VertexAttribute::TexCoord:
        glVertexAttribPointer(i, 2, GL_FLOAT, GL_FALSE, stride, (void *)(uintptr_t)offset);
        offset += sizeof(glm::vec2);
        break;

      case VertexAttribute::Tangent:
        glVertexAttribPointer(i, 3, GL_FLOAT, GL_FALSE, stride, (void *)(uintptr_t)offset);
        offset += sizeof(glm::vec3);
        break;

      case VertexAttribute::Bitangent:
        glVertexAttribPointer(i, 3, GL_FLOAT, GL_FALSE, stride, (void *)(uintptr_t)offset);
        offset += sizeof(glm::vec3);
        break;

      default:
        m_Logger->warn("Unknown vertex attribute type: {}", static_cast<int>(attr));
        break;
    }

    // 对于Instanced渲染，可以在此设置divisor
    // glVertexAttribDivisor(i, 0);
  }

  // 验证偏移量与声明的stride一致
  if (offset != stride) {
    m_Logger->error("Vertex attribute offset {} doesn't match layout stride {}", offset, stride);
  }
}
};  // namespace mite