#include "opengl_renderer/opegl_device.h"

namespace mite {
// ------------------------ 构造函数/析构函数 ------------------------
OpenGLDevice::OpenGLDevice() : IRenderDevice()
{
  // 创建日志系统
  m_Logger = mite::LoggerSystem::CreateModuleLogger("Mite OpenGL Device");
  m_Logger->trace("Created OpenGL Device");

  // 初始化GLAD（必须在上下文激活后调用）
  if (!gladLoadGL()) {
    m_Logger->critical("Failed to initialize GLAD");
    throw std::runtime_error("GLAD initialization failed");
  }
}

OpenGLDevice::~OpenGLDevice()
{
  // 防御性检查：确保所有资源已释放
  CleanupResources();
  m_Logger->info("OpenGLDevice destroyed");
}

void OpenGLDevice::CleanupResources()
{
  // 清理纹理
  if (!m_ActiveTextures.empty()) {
    m_Logger->warn("{} textures not released on shutdown", m_ActiveTextures.size());
    for (GLuint tex : m_ActiveTextures) {
      glDeleteTextures(1, &tex);
    }
  }
  // 清理缓冲区对象
  if (!m_ActiveVAOs.empty()) {
    m_Logger->warn("{} VAOs not released on shutdown", m_ActiveVAOs.size());
    for (GLuint vao : m_ActiveVAOs) {
      glDeleteVertexArrays(1, &vao);
    }
  }
  if (!m_ActiveVBOs.empty()) {
    m_Logger->warn("{} VBOs not released on shutdown", m_ActiveVBOs.size());
    for (GLuint vbo : m_ActiveVBOs) {
      glDeleteBuffers(1, &vbo);
    }
  }
  if (!m_ActiveEBOs.empty()) {
    m_Logger->warn("{} EBOs not released on shutdown", m_ActiveEBOs.size());
    for (GLuint ebo : m_ActiveEBOs) {
      glDeleteBuffers(1, &ebo);
    }
  }
  // 清理FrameBuffer对象
  if (!m_ActiveFBOs.empty()) {
    m_Logger->warn("{} FBOs not released on shutdown", m_ActiveFBOs.size());
    for (GLuint fbo : m_ActiveFBOs) {
      glDeleteFramebuffers(1, &fbo);
    }
  }
  m_Logger->info("Cleaned up {} GPU resources",
                 m_ActiveTextures.size() + m_ActiveVAOs.size() + m_ActiveVBOs.size() +
                     m_ActiveEBOs.size() + m_ActiveFBOs.size());
}

// ------------------------ 纹理操作 ------------------------
TextureGPUHandle OpenGLDevice::CreateTexture(std::shared_ptr<TextureSourceData> data)
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
  GLenum format = TranslateTextureFormat(data->format);
  GLenum internalFormat = /*!isHDR ? GL_SRGB8_ALPHA8 :*/ format;
  glTexImage2D(GL_TEXTURE_2D,
               0,               // Mipmap级别
               internalFormat,  // 内部格式
               data->width,
               data->height,
               0,                 // 历史遗留参数
               internalFormat,    // 像素数据格式
               GL_UNSIGNED_BYTE,  // 数据类型（HDR需改为GL_FLOAT）
               data->pixelData    // 原始数据指针
  );

  if (data->generateMipmaps) {
    glGenerateMipmap(GL_TEXTURE_2D);
  }

  // 记录活动纹理
  m_ActiveTextures.insert(textureID);

  TextureGPUHandle handle;
  handle.path = data->path;
  handle.apiHandle = static_cast<uintptr_t>(textureID);

  // 应用指定的包装和过滤模式
  SetTextureWrapMode(handle, data->wrapMode);
  SetTextureFilterMode(handle, data->filterMode);

  m_Logger->debug("Created texture handle: {}", data->path);
  return handle;
}

void OpenGLDevice::DestroyTexture(TextureGPUHandle handle)
{
  if (!handle.apiHandle)
    return;

  GLuint textureID = static_cast<GLuint>(handle.apiHandle);
  glDeleteTextures(1, &textureID);
  m_ActiveTextures.erase(textureID);
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
ModelGPUHandle OpenGLDevice::CreateModel(std::shared_ptr<ModelSourceData> data)
{
  // 0. 创建临时VAO等对象
  GLuint VBO, EBO, VAO;

  // 1. 创建VAO
  glGenVertexArrays(1, &VAO);
  glBindVertexArray(VAO);

  // 2. 创建VBO并上传数据
  glGenBuffers(1, &VBO);
  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferData(GL_ARRAY_BUFFER,
               data->mergedVertexData.size(),
               data->mergedVertexData.data(),
               GL_STATIC_DRAW);

  // 3. 创建EBO并上传数据
  glGenBuffers(1, &EBO);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER,
               data->mergedIndices.size() * sizeof(uint32_t),
               data->mergedIndices.data(),
               GL_STATIC_DRAW);

  // 4. 设置顶点属性指针(基于统一的layout)
  SetVertexAttributes(data->layout);

  // 5. 解绑VAO
  glBindVertexArray(0);

  // 6. 填充GPU句柄
  ModelGPUHandle handle;
  handle.path = data->path;
  handle.bboxMax = data->modelBboxMax;
  handle.bboxMin = data->modelBboxMin;
  handle.vertexArray = static_cast<uintptr_t>(VAO);
  handle.vertexBuffer = static_cast<uintptr_t>(VBO);
  handle.indexBuffer = static_cast<uintptr_t>(EBO);

  // 7. 记录活动网格（调试用）
  m_ActiveVAOs.insert(VAO);
  m_ActiveVBOs.insert(VBO);
  m_ActiveEBOs.insert(EBO);

  // 8. 保存ModelSourceData创建时生成的MeshSections
  handle.subMeshes = std::move(data->sections);

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
    m_ActiveVAOs.erase(vao);
  }

  // 2. 删除顶点缓冲区(VBO)
  if (handle.vertexBuffer != 0) {
    GLuint vbo = static_cast<GLuint>(handle.vertexBuffer);
    glDeleteBuffers(1, &vbo);

    // 从活动资源中移除
    m_ActiveVBOs.erase(vbo);
  }

  // 3. 删除索引缓冲区(EBO)
  if (handle.indexBuffer != 0) {
    GLuint ebo = static_cast<GLuint>(handle.indexBuffer);
    glDeleteBuffers(1, &ebo);

    // 从活动资源中移除
    m_ActiveEBOs.erase(ebo);
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
  // m_Logger->debug("Bound mesh: VAO={}, VBO={}, EBO={}, indexOffset={}, vertexOffset={}",
  //                vao,
  //                modelHandle->vertexBuffer,
  //                modelHandle->indexBuffer,
  //                meshSection.indexOffset,
  //                meshSection.vertexOffset);
}

void OpenGLDevice::DrawIndexed(uint32_t indexCount,
                               uint32_t indexOffset,
                               GLenum mode,
                               GLenum indexType,
                               bool enableDepthTest) const
{
  // 1. 参数验证
  if (indexCount == 0) {
    m_Logger->warn("Attempted to draw with indexCount = 0");
    return;
  }

  // 2. 详细的状态检查
  GLint currentVAO = 0;
  glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &currentVAO);
  if (currentVAO == 0) {
    m_Logger->error("DrawIndexed Failed：Invalid VAO");
    return;
  }

  GLint currentProgram = 0;
  glGetIntegerv(GL_CURRENT_PROGRAM, &currentProgram);
  if (currentProgram == 0) {
    m_Logger->error("DrawIndexed Failed：Invalid Shader");
    return;
  }

  GLint elementBuffer = 0;
  glGetIntegerv(GL_ELEMENT_ARRAY_BUFFER_BINDING, &elementBuffer);
  if (elementBuffer == 0) {
    m_Logger->error("DrawIndexed Failed：Invalid EBO");
    return;
  }

  // 3. 设置深度测试状态
  if (enableDepthTest) {
    glEnable(GL_DEPTH_TEST);
  }
  else {
    glDisable(GL_DEPTH_TEST);
  }

  // 4. 计算索引偏移量
  void *indicesPtr = nullptr;
  size_t typeSize = 0;
  switch (indexType) {
    case GL_UNSIGNED_BYTE:
      typeSize = sizeof(GLubyte);
      break;
    case GL_UNSIGNED_SHORT:
      typeSize = sizeof(GLushort);
      break;
    case GL_UNSIGNED_INT:
      typeSize = sizeof(GLuint);
      break;
    default:
      m_Logger->error("Invalid Index Type: {}", indexType);
      return;
  }
  indicesPtr = reinterpret_cast<void *>(indexOffset * typeSize);

  // 5. 执行绘制命令
  glDrawElements(mode, indexCount, indexType, indicesPtr);

  // 6. 增强的错误检查
  GLenum err = glGetError();
  if (err != GL_NO_ERROR) {
    const char *errorStr = "";
    switch (err) {
      case GL_INVALID_ENUM:
        errorStr = "GL_INVALID_ENUM";
        break;
      case GL_INVALID_VALUE:
        errorStr = "GL_INVALID_VALUE";
        break;
      case GL_INVALID_OPERATION:
        errorStr = "GL_INVALID_OPERATION";
        break;
      case GL_INVALID_FRAMEBUFFER_OPERATION:
        errorStr = "GL_INVALID_FRAMEBUFFER_OPERATION";
        break;
      case GL_OUT_OF_MEMORY:
        errorStr = "GL_OUT_OF_MEMORY";
        break;
      default:
        errorStr = "Unknown Error";
    }

    m_Logger->error(
        "OpenGL Draw Error: {} ({})\n"
        "More infomation:\n"
        "- Mode: {}\n"
        "- Index Count: {}\n"
        "- Index Type: {}\n"
        "- VAO: {}\n"
        "- Shader: {}\n"
        "- EBO: {}",
        err,
        errorStr,
        mode,
        indexCount,
        indexType,
        currentVAO,
        currentProgram,
        elementBuffer);
  }
}

// ------------------------ FrameBuffer 操作 ------------------------

FrameBuffer::Ptr OpenGLDevice::CreateFrameBuffer(const FrameBufferSpec &spec)
{
  // 创建FrameBuffer对象
  auto framebuffer = std::make_shared<FrameBuffer>(spec);

  // 记录FBO ID
  m_ActiveFBOs.insert(framebuffer->GetID());

  m_Logger->debug("Created framebuffer ({}x{})", spec.width, spec.height);
  return framebuffer;
}

void OpenGLDevice::DestroyFrameBuffer(FrameBuffer::Ptr framebuffer)
{
  if (!framebuffer)
    return;

  // 从活动集合中移除
  m_ActiveFBOs.erase(framebuffer->GetID());

  // 实际销毁操作由FrameBuffer析构函数处理
  m_Logger->debug("Destroyed framebuffer");
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

bool OpenGLDevice::OnModelLoaded(ModelLoadEvent &e)
{
  // 1. 创建GPU资源
  ModelGPUHandle modelHandle = CreateModel(e.GetModelSourceData());

  // 2. 更新ModelAsset
  e.GetModelGPUHandle() = std::make_shared<ModelGPUHandle>(modelHandle);

  // 标记事件已处理，阻断传播
  e.Handled();
  return e.handled;
}

bool OpenGLDevice::OnTextureLoaded(TextureLoadEvent &e)
{
  //  1. 创建GPU资源
  TextureGPUHandle textureHandle = CreateTexture(e.GetTextureSourceData());

  // 2. 更新TextureAsset
  e.GetTextureHandle() = std::make_shared<TextureGPUHandle>(textureHandle);

  // 标记事件已处理，阻断传播
  e.Handled();
  return e.handled;
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