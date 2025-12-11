#include "opengl_device.h"
#include "basic_shader/shader_binding_point_manager.h"

namespace mite {
// ------------------------ 构造函数/析构函数 ------------------------
OpenGLDevice::OpenGLDevice() : RenderDevice()
{
  // 创建日志系统
  m_Logger = mite::LoggerSystem::CreateModuleLogger("Mite OpenGL Device");
  m_Logger->trace("Created OpenGL Device");

  // 初始化GLAD（必须在上下文激活后调用）
  if (!gladLoadGL()) {
    m_Logger->critical("Failed to initialize GLAD");
    throw std::runtime_error("GLAD initialization failed");
  }

  // 初始化默认纹理
  InitializeDefaultTextures();

  // 创建全屏四边形
  CreateFullScreenQuad();
}

OpenGLDevice::~OpenGLDevice()
{
  // 清理默认纹理
  CleanupDefaultTextures();

  // 清理全屏四边形
  DestroyFullScreenQuad();

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
  if (!data) {
    LOG_ERROR("Invalid texture source data provided");
    return TextureGPUHandle{0};
  }

  // GL生成纹理ID
  GLuint textureId = 0;
  glGenTextures(1, &textureId);
  if (textureId == 0) {
    LOG_ERROR("Failed to generate OpenGL texture");
    return TextureGPUHandle{0};
  }

  // 绑定纹理
  glBindTexture(static_cast<GLenum>(data->target), textureId);

  // 设置纹理参数
  SetTextureParameters(data);

  // 上传纹理数据
  if (!UploadTextureData(data, textureId)) {
    glDeleteTextures(1, &textureId);
    return TextureGPUHandle{0};
  }

  // 生成Mipmaps（如果需要）
  if (data->generateMipmaps && data->existingMipLevels <= 1) {
    glGenerateMipmap(static_cast<GLenum>(data->target));
  }

  // 解除纹理绑定
  glBindTexture(static_cast<GLenum>(data->target), 0);

  // 检查GLError
  CheckGLError();

  m_Logger->debug("Created texture handle: {}", textureId);

  // 返回纹理句柄
  return TextureGPUHandle{static_cast<uintptr_t>(textureId)};
}

TextureGPUHandle OpenGLDevice::CreateRuntimeTexture(std::shared_ptr<TextureCreateInfo> createInfo)
{
  if (!createInfo) {
    LOG_ERROR("Invalid texture create info provided");
    return TextureGPUHandle{0};
  }

  // 参数验证
  if (createInfo->width == 0 || createInfo->height == 0) {
    LOG_ERROR("Invalid texture dimensions: {}x{}", createInfo->width, createInfo->height);
    return TextureGPUHandle{0};
  }

  // 数组纹理层数验证
  if ((createInfo->target == TextureTarget::TEXTURE_2D_ARRAY ||
       createInfo->target == TextureTarget::TEXTURE_CUBE_MAP_ARRAY ||
       createInfo->target == TextureTarget::TEXTURE_2D_MULTISAMPLE_ARRAY) &&
      createInfo->arrayLayers == 0)
  {
    LOG_ERROR("Array texture target requires arrayLayers > 0, target: {}, layers: {}",
              static_cast<int>(createInfo->target),
              createInfo->arrayLayers);
    return TextureGPUHandle{0};
  }

  // 生成纹理ID
  GLuint textureId = 0;
  glGenTextures(1, &textureId);
  if (textureId == 0) {
    LOG_ERROR("Failed to generate OpenGL texture for runtime texture");
    return TextureGPUHandle{0};
  }

  // 绑定纹理
  glBindTexture(static_cast<GLenum>(createInfo->target), textureId);

  // 设置纹理参数
  glTexParameteri(static_cast<GLenum>(createInfo->target),
                  GL_TEXTURE_WRAP_S,
                  static_cast<GLint>(createInfo->wrapModeS));
  glTexParameteri(static_cast<GLenum>(createInfo->target),
                  GL_TEXTURE_WRAP_T,
                  static_cast<GLint>(createInfo->wrapModeT));
  glTexParameteri(static_cast<GLenum>(createInfo->target),
                  GL_TEXTURE_MIN_FILTER,
                  static_cast<GLint>(createInfo->minFilter));
  glTexParameteri(static_cast<GLenum>(createInfo->target),
                  GL_TEXTURE_MAG_FILTER,
                  static_cast<GLint>(createInfo->magFilter));

  // 获取OpenGL格式
  GLenum internalFormat, format, type;
  if (!GetGLTextureFormats(createInfo->format, internalFormat, format, type)) {
    LOG_ERROR("Unsupported texture format for runtime texture: {}",
              static_cast<int>(createInfo->format));
    glDeleteTextures(1, &textureId);
    return TextureGPUHandle{0};
  }

  // ==================== 修改：根据纹理目标使用不同的分配函数 ====================
  switch (createInfo->target) {
    case TextureTarget::TEXTURE_2D:
    case TextureTarget::TEXTURE_CUBE_MAP:
      // 标准2D纹理和立方体贴图使用glTexImage2D
      if (createInfo->target == TextureTarget::TEXTURE_CUBE_MAP) {
        // 立方体贴图需要为6个面分别分配存储
        for (GLuint face = 0; face < 6; ++face) {
          glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + face,
                       0,
                       internalFormat,
                       createInfo->width,
                       createInfo->height,
                       0,
                       format,
                       type,
                       nullptr);
        }
      }
      else {
        // 普通2D纹理
        glTexImage2D(static_cast<GLenum>(createInfo->target),
                     0,
                     internalFormat,
                     createInfo->width,
                     createInfo->height,
                     0,
                     format,
                     type,
                     nullptr);
      }
      break;
    case TextureTarget::TEXTURE_2D_ARRAY:
    case TextureTarget::TEXTURE_CUBE_MAP_ARRAY:
    case TextureTarget::TEXTURE_3D:
      // 数组纹理和3D纹理使用glTexImage3D
      glTexImage3D(static_cast<GLenum>(createInfo->target),
                   0,
                   internalFormat,
                   createInfo->width,
                   createInfo->height,
                   createInfo->arrayLayers,
                   0,
                   format,
                   type,
                   nullptr);
      break;
    case TextureTarget::TEXTURE_2D_MULTISAMPLE:
      // 多重采样2D纹理
      glTexImage2DMultisample(static_cast<GLenum>(createInfo->target),
                              4,  // 采样数 createInfo->samples，未实现，默认为4平衡性能
                              internalFormat,
                              createInfo->width,
                              createInfo->height,
                              GL_TRUE);  // 固定采样位置
      break;
    case TextureTarget::TEXTURE_2D_MULTISAMPLE_ARRAY:
      // 多重采样数组纹理
      glTexImage3DMultisample(static_cast<GLenum>(createInfo->target),
                              4,  // 采样数 createInfo->samples，未实现，默认为4平衡性能
                              internalFormat,
                              createInfo->width,
                              createInfo->height,
                              createInfo->arrayLayers,
                              GL_TRUE);  // 固定采样位置
      break;
    default:
      LOG_ERROR("Unsupported texture target for runtime texture: {}",
                static_cast<int>(createInfo->target));
      glDeleteTextures(1, &textureId);
      return TextureGPUHandle{0};
  }

  // 生成Mipmaps（如果需要）
  if (createInfo->generateMipmaps) {
    glGenerateMipmap(static_cast<GLenum>(createInfo->target));
  }

  // 解除绑定
  glBindTexture(static_cast<GLenum>(createInfo->target), 0);

  // 记录活动纹理
  m_ActiveTextures.insert(textureId);

  // 检查GLError
  CheckGLError();

  m_Logger->debug("Created runtime texture: ID={}, size={}x{}, layers={}, format={}, target={}",
                  textureId,
                  createInfo->width,
                  createInfo->height,
                  createInfo->arrayLayers,
                  static_cast<int>(createInfo->format),
                  static_cast<int>(createInfo->target));

  return TextureGPUHandle{static_cast<uintptr_t>(textureId)};
}

void OpenGLDevice::DestroyTexture(TextureGPUHandle handle)
{
  if (!handle.apiHandle)
    return;

  GLuint textureID = static_cast<GLuint>(handle.apiHandle);
  glDeleteTextures(1, &textureID);
  m_ActiveTextures.erase(textureID);
}

void OpenGLDevice::BindRuntimeTexture(RuntimeTextureType type,
                                      TextureGPUHandle textureHandle,
                                      TextureTarget target) const
{
  uint32_t textureUnit = BindingPointManager::Get().GetRuntimeTextureBinding(type);
  if (textureUnit != UINT32_MAX) {
    glActiveTexture(GL_TEXTURE0 + textureUnit);
    if (textureHandle.apiHandle != 0) {
      // 绑定有效纹理
      glBindTexture(static_cast<GLenum>(target), static_cast<GLuint>(textureHandle.apiHandle));
    }
    else {
      // 绑定默认纹理
      glBindTexture(GL_TEXTURE_2D, m_WhiteTexture);

      LOG_WARN("Failed to bind runtime texture: invalid texture handle, bind to default texture");
    }
  }
  else {
    LOG_ERROR("Failed to bind runtime texture: binding point not allocated");
  }
}
void OpenGLDevice::BindExternalTexture(ExternalTextureType type,
                                       TextureGPUHandle textureHandle,
                                       TextureTarget target) const
{
  uint32_t textureUnit = BindingPointManager::Get().GetExternalTextureBinding(type);
  if (textureUnit != UINT32_MAX) {
    glActiveTexture(GL_TEXTURE0 + textureUnit);
    if (textureHandle.apiHandle != 0) {
      // 绑定有效纹理
      glBindTexture(static_cast<GLenum>(target), static_cast<GLuint>(textureHandle.apiHandle));
    }
    else {
      // 绑定默认纹理
      glBindTexture(GL_TEXTURE_2D, m_WhiteTexture);
    }
  }
  else {
    LOG_ERROR("Failed to bind external texture: binding point not allocated");
  }
}

void OpenGLDevice::BindFrameBufferDepthLayer(std::shared_ptr<FrameBuffer> fbo,
                                             uint32_t layer) const
{
  if (!fbo)
    return;

  // 获取深度纹理附件
  auto depthTexture = fbo->GetDepthAttachment();
  if (!depthTexture || !depthTexture->IsValid()) {
    LOG_ERROR("Failed to bind frame buffer layer: invalid depth attachment");
    return;
  }
  GLuint textureID = static_cast<GLuint>(depthTexture->GetHandle().apiHandle);

  // 使用glFramebufferTextureLayer绑定到特定层
  glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, textureID, 0, layer);
}
void OpenGLDevice::BindFramebufferDepthCubeFace(std::shared_ptr<FrameBuffer> fbo,
                                                uint32_t layer,
                                                uint32_t face) const
{
  if (!fbo)
    return;

  // 获取深度纹理附件
  auto depthTexture = fbo->GetDepthAttachment();
  if (!depthTexture || !depthTexture->IsValid()) {
    LOG_ERROR("Failed to bind frame buffer layer: invalid depth attachment");
    return;
  }

  GLuint textureID = static_cast<GLuint>(depthTexture->GetHandle().apiHandle);

  // 对于立方体贴图数组，使用glFramebufferTextureLayer绑定到特定层和面
  // 注意：立方体贴图数组的层索引计算为 layer * 6 + face
  uint32_t arrayLayer = layer * 6 + face;
  glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, textureID, 0, arrayLayer);
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

  // 8. 记录LOD信息
  m_Logger->debug("Created model with for: {}", data->path);

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
  if (!mesh || !mesh->GetVertexCount()) {
    m_Logger->error("Invalid Mesh in binding mesh by opengl device.");
    return;
  }

  ModelGPUHandle modelHandle = mesh->GetModelHandle();
  MeshSection meshSection = mesh->GetSection();

  // 1. 参数有效性检查
  if (modelHandle.vertexArray == 0) {
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
  GLuint vao = static_cast<GLuint>(modelHandle.vertexArray);
  glBindVertexArray(vao);

  // 3. 验证缓冲区是否有效
  if (modelHandle.vertexBuffer == 0 || modelHandle.indexBuffer == 0) {
    m_Logger->error("Model buffers not initialized (VBO={}, EBO={})",
                    modelHandle.vertexBuffer,
                    modelHandle.indexBuffer);
  }

  // 4. 绑定缓冲区（VAO已包含这些信息，但显式绑定更安全）
  glBindBuffer(GL_ARRAY_BUFFER, static_cast<GLuint>(modelHandle.vertexBuffer));
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, static_cast<GLuint>(modelHandle.indexBuffer));

  // 5. 存储当前绑定的MeshSection（供后续Draw调用使用）
  // 注意：这需要OpenGLDevice有成员变量存储当前状态，或者使用其他状态管理机制
  // m_CurrentMeshSection = &meshSection;
  // m_CurrentModelHandle = modelHandle;

  // 6. 调试信息
  m_Logger->trace("Bound mesh: VAO={}, VBO={}, EBO={}, indexOffset={}, vertexOffset={}",
                  vao,
                  modelHandle.vertexBuffer,
                  modelHandle.indexBuffer,
                  meshSection.indexOffset,
                  meshSection.vertexOffset);
}

void OpenGLDevice::DrawMeshLOD(std::shared_ptr<Mesh> mesh, uint32_t lodLevel) const
{
  // 直接从Mesh对象获取指定LOD级别的MeshSection
  const MeshSection *targetSection = &mesh->GetSection(lodLevel);

  // 绘制指定LOD级别的网格
  DrawIndexed(
      targetSection->indexCount, targetSection->indexOffset, GL_TRIANGLES, GL_UNSIGNED_INT);
}

void OpenGLDevice::DrawIndexed(uint32_t indexCount,
                               uint32_t indexOffset,
                               GLenum mode,
                               GLenum indexType) const
{
  // 1. 参数验证
  if (indexCount == 0) {
    // index count = 0时，直接返回即可。仅检测，不执行draw call，
    // 基本无资源消耗。但绑定材质可能存在消耗
    // 所以仅记录trace，不记录warn
    m_Logger->trace("Attempted to draw with indexCount = 0");
    return;
  }

  // 2. 详细的状态检查
  GLint currentVAO = 0;
  glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &currentVAO);
  if (currentVAO == 0) {
    m_Logger->error("DrawIndexed Failed: Invalid VAO");
    return;
  }

  GLint currentProgram = 0;
  glGetIntegerv(GL_CURRENT_PROGRAM, &currentProgram);
  if (currentProgram == 0) {
    m_Logger->error("DrawIndexed Failed: Invalid Shader");
    return;
  }

  GLint elementBuffer = 0;
  glGetIntegerv(GL_ELEMENT_ARRAY_BUFFER_BINDING, &elementBuffer);
  if (elementBuffer == 0) {
    m_Logger->error("DrawIndexed Failed: Invalid EBO");
    return;
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
}

// ------------------------ FrameBuffer 操作 ------------------------

std::shared_ptr<FrameBuffer> OpenGLDevice::CreateFrameBuffer(const FrameBufferSpec &spec)
{
  // 创建FrameBuffer对象
  auto framebuffer = std::make_shared<FrameBuffer>(spec);

  // 记录FBO ID
  m_ActiveFBOs.insert(framebuffer->GetID());

  m_Logger->debug("Created framebuffer ({}x{})", spec.width, spec.height);
  return framebuffer;
}

void OpenGLDevice::DestroyFrameBuffer(std::shared_ptr<FrameBuffer> framebuffer)
{
  if (!framebuffer)
    return;

  // 从活动集合中移除
  m_ActiveFBOs.erase(framebuffer->GetID());

  // 实际销毁操作由FrameBuffer析构函数处理
  m_Logger->debug("Destroyed framebuffer");
}

void OpenGLDevice::CreateFullScreenQuad()
{
  // 全屏四边形顶点数据 (位置, UV)
  float quadVertices[] = {// 位置          // UV
                          -1.0f, 1.0f,  0.0f, 1.0f, -1.0f, -1.0f, 0.0f, 0.0f,
                          1.0f,  -1.0f, 1.0f, 0.0f, -1.0f, 1.0f,  0.0f, 1.0f,
                          1.0f,  -1.0f, 1.0f, 0.0f, 1.0f,  1.0f,  1.0f, 1.0f};

  // 创建VAO和VBO
  glGenVertexArrays(1, &m_ScreenQuadVAO);
  glGenBuffers(1, &m_ScreenQuadVBO);

  // 绑定顶点数据
  glBindVertexArray(m_ScreenQuadVAO);
  glBindBuffer(GL_ARRAY_BUFFER, m_ScreenQuadVBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);

  // 位置属性
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)0);

  // UV属性
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)(2 * sizeof(float)));

  glBindVertexArray(0);
}

void OpenGLDevice::DrawFullScreenQuad()
{
  glBindVertexArray(m_ScreenQuadVAO);
  glDrawArrays(GL_TRIANGLES, 0, 6);
  glBindVertexArray(0);
}

void OpenGLDevice::DestroyFullScreenQuad()
{
  // 清理全屏四边形
  if (m_ScreenQuadVAO != 0) {
    glDeleteVertexArrays(1, &m_ScreenQuadVAO);
    m_ScreenQuadVAO = 0;
  }
  if (m_ScreenQuadVBO != 0) {
    glDeleteBuffers(1, &m_ScreenQuadVBO);
    m_ScreenQuadVBO = 0;
  }
}

// ------------------------ 辅助方法 ------------------------

void OpenGLDevice::OnModelLoaded(ModelLoadEvent &e)
{
  // 1. 创建GPU资源
  ModelGPUHandle modelHandle = CreateModel(e.GetModelSourceData());

  // 2. 更新ModelAsset（值传递，避免modelHandle脱离作用域）
  e.GetModelAsset()->handle = modelHandle;

  // 3. 标记事件已消费，阻断传播
  e.SetResult(EventResult::Consumed);
}

void OpenGLDevice::OnTextureLoaded(TextureLoadEvent &e)
{
  //  1. 创建GPU资源
  TextureGPUHandle textureHandle = CreateTexture(e.GetTextureSourceData());

  // 2. 更新TextureAsset
  e.GetTextureAsset()->instance.gpuHandle = textureHandle;

  // 3. 标记事件已消费，阻断传播
  e.SetResult(EventResult::Consumed);
}

void OpenGLDevice::OnRuntimeTextureCreate(RuntimeTextureCreateEvent &e)
{
  //  1. 创建GPU资源
  TextureGPUHandle textureHandle = CreateRuntimeTexture(e.GetTextureCreateInfo());

  // 2. 调用回调函数传回Handle
  e.GetCallback()(textureHandle);

  // 3. 标记事件已消费，阻断传播
  e.SetResult(EventResult::Consumed);
}

void OpenGLDevice::OnRuntimeTextureDestroyRequest(RuntimeTextureDestroyRequestEvent &e)
{
  DestroyTexture(e.GetTextureGPUHandle());

  // 标记事件已消费，阻断传播
  e.SetResult(EventResult::Consumed);
}

void OpenGLDevice::InitializeDefaultTextures()
{
  // 创建1x1白色/黑色纹理作为通用默认纹理
  m_WhiteTexture = CreateWhite1x1Texture();
  m_BlackTexture = CreateBlack1x1Texture();

  m_Logger->info("Initialized default textures");
}
void OpenGLDevice::CleanupDefaultTextures()
{
  if (m_WhiteTexture != 0) {
    glDeleteTextures(1, &m_WhiteTexture);
    m_WhiteTexture = 0;
  }
  if (m_BlackTexture != 0) {
    glDeleteTextures(1, &m_BlackTexture);
    m_BlackTexture = 0;
  }
  m_Logger->debug("Cleaned up default textures");
}
GLuint OpenGLDevice::CreateWhite1x1Texture()
{
  GLuint textureId;
  glGenTextures(1, &textureId);
  glBindTexture(GL_TEXTURE_2D, textureId);

  // 1x1 白色像素
  unsigned char whitePixel[] = {255, 255, 255, 255};
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, whitePixel);

  // 设置合理的默认参数
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

  glBindTexture(GL_TEXTURE_2D, 0);

  return textureId;
}

GLuint OpenGLDevice::CreateBlack1x1Texture()
{
  GLuint textureId;
  glGenTextures(1, &textureId);
  glBindTexture(GL_TEXTURE_2D, textureId);

  // 1x1 白色像素
  unsigned char blackPixel[] = {0, 0, 0, 255};
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, blackPixel);

  // 设置合理的默认参数
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

  glBindTexture(GL_TEXTURE_2D, 0);

  return textureId;
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

  // 预计算每个属性的偏移量
  std::unordered_map<VertexAttribute, uint32_t> attributeOffsets;
  uint32_t currentOffset = 0;

  // 按照layout中声明的顺序计算偏移量
  for (const auto &attr : layout.attributes) {
    attributeOffsets[attr] = currentOffset;

    switch (attr) {
      case VertexAttribute::Position:
      case VertexAttribute::Normal:
      case VertexAttribute::Tangent:
      case VertexAttribute::Bitangent:
        currentOffset += sizeof(glm::vec3);
        break;
      case VertexAttribute::TexCoord:
        currentOffset += sizeof(glm::vec2);
        break;
      default:
        m_Logger->warn("Unknown vertex attribute type in offset calculation: {}",
                       static_cast<int>(attr));
        break;
    }
  }

  // 验证计算的总偏移量与声明的stride一致
  if (currentOffset != stride) {
    m_Logger->error("Calculated offset {} doesn't match layout stride {}", currentOffset, stride);
  }

  // 按照枚举顺序设置顶点属性（使用固定location）
  for (uint32_t i = 0; i < static_cast<uint32_t>(VertexAttribute::Count); ++i) {
    VertexAttribute attr = static_cast<VertexAttribute>(i);

    // 检查该属性是否存在于当前layout中
    auto offsetIt = attributeOffsets.find(attr);
    if (offsetIt == attributeOffsets.end()) {
      // 该属性不存在，禁用对应的顶点属性数组
      glDisableVertexAttribArray(i);
      continue;
    }

    // 获取偏移量
    const uint32_t offset = offsetIt->second;
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
        break;
      case VertexAttribute::Normal:
        glVertexAttribPointer(i, 3, GL_FLOAT, GL_FALSE, stride, (void *)(uintptr_t)offset);
        break;
      case VertexAttribute::TexCoord:
        glVertexAttribPointer(i, 2, GL_FLOAT, GL_FALSE, stride, (void *)(uintptr_t)offset);
        break;
      case VertexAttribute::Tangent:
        glVertexAttribPointer(i, 3, GL_FLOAT, GL_FALSE, stride, (void *)(uintptr_t)offset);
        break;
      case VertexAttribute::Bitangent:
        glVertexAttribPointer(i, 3, GL_FLOAT, GL_FALSE, stride, (void *)(uintptr_t)offset);
        break;
      default:
        m_Logger->warn("Unknown vertex attribute type: {}", static_cast<int>(attr));
        glDisableVertexAttribArray(i);
        break;
    }

    // 对于Instanced实例化渲染，可以在此设置divisor
    // glVertexAttribDivisor(i, 0);
  }
}

void OpenGLDevice::SetTextureParameters(std::shared_ptr<TextureSourceData> data)
{
  // 设置包装模式
  glTexParameteri(
      static_cast<GLenum>(data->target), GL_TEXTURE_WRAP_S, static_cast<GLint>(data->wrapModeS));
  glTexParameteri(
      static_cast<GLenum>(data->target), GL_TEXTURE_WRAP_T, static_cast<GLint>(data->wrapModeT));

  // 设置过滤模式
  glTexParameteri(static_cast<GLenum>(data->target),
                  GL_TEXTURE_MIN_FILTER,
                  static_cast<GLint>(data->minFilter));
  glTexParameteri(static_cast<GLenum>(data->target),
                  GL_TEXTURE_MAG_FILTER,
                  static_cast<GLint>(data->magFilter));

  // 设置各向异性过滤（如果支持）
  if (GLAD_GL_EXT_texture_filter_anisotropic) {
    GLfloat maxAnisotropy;
    glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &maxAnisotropy);
    glTexParameterf(
        static_cast<GLenum>(data->target), GL_TEXTURE_MAX_ANISOTROPY_EXT, maxAnisotropy);
  }
}
bool OpenGLDevice::UploadTextureData(std::shared_ptr<TextureSourceData> data, GLuint textureId)
{
  // 获取OpenGL格式信息
  GLenum internalFormat, format, type;
  if (!GetGLTextureFormats(data->format, internalFormat, format, type)) {
    LOG_ERROR("Unsupported texture format: {}", static_cast<int>(data->format));
    return false;
  }

  try {
    // 上传纹理数据
    glTexImage2D(static_cast<GLenum>(data->target),
                 0,
                 internalFormat,
                 data->width,
                 data->height,
                 0,
                 format,
                 type,
                 data->pixelData.data());

    // 检查OpenGL错误
    GLenum error = glGetError();
    if (error != GL_NO_ERROR) {
      LOG_ERROR("OpenGL error during texture upload: {}", error);
      return false;
    }

    return true;
  }
  catch (const std::exception &e) {
    LOG_ERROR("Exception during texture upload: {}", e.what());
    return false;
  }
}
bool OpenGLDevice::GetGLTextureFormats(TextureFormat textureFormat,
                                       GLenum &internalFormat,
                                       GLenum &format,
                                       GLenum &type)
{
  switch (textureFormat) {
    // 8位无符号归一化格式
    case TextureFormat::R8:
      internalFormat = GL_R8;
      format = GL_RED;
      type = GL_UNSIGNED_BYTE;
      break;
    case TextureFormat::RG8:
      internalFormat = GL_RG8;
      format = GL_RG;
      type = GL_UNSIGNED_BYTE;
      break;
    case TextureFormat::RGB8:
      internalFormat = GL_RGB8;
      format = GL_RGB;
      type = GL_UNSIGNED_BYTE;
      break;
    case TextureFormat::RGBA8:
      internalFormat = GL_RGBA8;
      format = GL_RGBA;
      type = GL_UNSIGNED_BYTE;
      break;
    // sRGB格式（伽马校正）
    case TextureFormat::SRGB8:
      internalFormat = GL_SRGB8;
      format = GL_RGB;
      type = GL_UNSIGNED_BYTE;
      break;
    case TextureFormat::SRGB8_ALPHA8:
      internalFormat = GL_SRGB8_ALPHA8;
      format = GL_RGBA;
      type = GL_UNSIGNED_BYTE;
      break;
    // 深度/模板格式
    case TextureFormat::DEPTH_COMPONENT16:
      internalFormat = GL_DEPTH_COMPONENT16;
      format = GL_DEPTH_COMPONENT;
      type = GL_UNSIGNED_SHORT;
      break;
    case TextureFormat::DEPTH_COMPONENT24:
      internalFormat = GL_DEPTH_COMPONENT24;
      format = GL_DEPTH_COMPONENT;
      type = GL_UNSIGNED_INT_24_8;
      break;
    case TextureFormat::DEPTH_COMPONENT32:
      internalFormat = GL_DEPTH_COMPONENT32;
      format = GL_DEPTH_COMPONENT;
      type = GL_UNSIGNED_INT;
      break;
    case TextureFormat::STENCIL_INDEX1:
      internalFormat = GL_STENCIL_INDEX1;
      format = GL_STENCIL_INDEX;
      type = GL_UNSIGNED_BYTE;
      break;
    case TextureFormat::STENCIL_INDEX4:
      internalFormat = GL_STENCIL_INDEX4;
      format = GL_STENCIL_INDEX;
      type = GL_UNSIGNED_BYTE;
      break;
    case TextureFormat::STENCIL_INDEX8:
      internalFormat = GL_STENCIL_INDEX8;
      format = GL_STENCIL_INDEX;
      type = GL_UNSIGNED_BYTE;
      break;
    case TextureFormat::STENCIL_INDEX16:
      internalFormat = GL_STENCIL_INDEX16;
      format = GL_STENCIL_INDEX;
      type = GL_UNSIGNED_SHORT;
      break;
    case TextureFormat::DEPTH24_STENCIL8:
      internalFormat = GL_DEPTH24_STENCIL8;
      format = GL_DEPTH_STENCIL;
      type = GL_UNSIGNED_INT_24_8;
      break;
    // 高精度浮点格式（HDR/GBuffer专用）
    case TextureFormat::RGB16F:
      internalFormat = GL_RGB16F;
      format = GL_RGB;
      type = GL_HALF_FLOAT;
      break;
    case TextureFormat::RGBA16F:
      internalFormat = GL_RGBA16F;
      format = GL_RGBA;
      type = GL_HALF_FLOAT;
      break;
    case TextureFormat::RGB32F:
      internalFormat = GL_RGB32F;
      format = GL_RGB;
      type = GL_FLOAT;
      break;
    case TextureFormat::RGBA32F:
      internalFormat = GL_RGBA32F;
      format = GL_RGBA;
      type = GL_FLOAT;
      break;
    default:
      return false;  // 未知格式
  }
  return true;
}
void OpenGLDevice::CheckGLError(std::string debugName)
{
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

    LOG_ERROR("OpenGL Error in {}: {} ({})", debugName, err, errorStr);
  }
}
};  // namespace mite