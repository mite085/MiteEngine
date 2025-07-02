#include "opengl_renderer/opegl_device.h"

namespace mite {
// ------------------------ 构造函数/析构函数 ------------------------
OpenGLDevice::OpenGLDevice() {}

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
TextureGPUHandle OpenGLDevice::CreateTexture(const TextureAsset &texture)
{
  auto meta = texture.metadata;

  // 检查是否可用
  if (texture.textureData.textureData == nullptr) {
    LOG_ERROR("Invalid texture: {}", meta.path);
    return {static_cast<uintptr_t>(0)};
  }
  auto data = texture.textureData.textureData.get();

  GLuint textureID;
  glGenTextures(1, &textureID);
  glBindTexture(GL_TEXTURE_2D, textureID);

  // 设置纹理参数
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  // 上传纹理数据
  GLenum format = TranslateTextureFormat(meta.format);
  GLenum internalFormat = !meta.isHDR ? GL_SRGB8_ALPHA8 : format;
  glTexImage2D(GL_TEXTURE_2D,
               0,
               internalFormat,
               meta.width,
               meta.height,
               0,
               format,
               GL_UNSIGNED_BYTE,
               data);

  glGenerateMipmap(GL_TEXTURE_2D);

  // 记录活动纹理
  activeTextures_.insert(textureID);

  TextureGPUHandle handle = {static_cast<uintptr_t>(textureID)};
  SetTextureWrapMode(handle, TextureWrapMode::Repeat);
  SetTextureFilterMode(handle, TextureFilterMode::Linear);

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
  glActiveTexture(GL_TEXTURE0 + slot);
  glBindTexture(GL_TEXTURE_2D, static_cast<GLuint>(handle.apiHandle));
}
void OpenGLDevice::SetTextureWrapMode(TextureGPUHandle handle, TextureWrapMode mode)
{
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

// ------------------------ 模型操作 ------------------------
ModelGPUHandle OpenGLDevice::CreateModel(const ModelAsset &model)
{
  ModelGPUHandle modelHandle;

  // 处理所有子网格
  for (auto &subMesh : model.subMeshes) {
    MeshGPUHandle subMeshHandle = CreateSubMesh(subMesh);
    modelHandle.subMeshes.push_back(subMeshHandle);
  }

  return modelHandle;
}

MeshGPUHandle OpenGLDevice::CreateSubMesh(const MeshData &subMesh)
{
  MeshGPUHandle handle;
  GLuint VBO, EBO, VAO;

  // 1. 创建VAO
  glGenVertexArrays(1, &VAO);
  glBindVertexArray(VAO);

  // 2. 创建VBO并上传数据
  glGenBuffers(1, &VBO);
  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferData(
      GL_ARRAY_BUFFER, subMesh.vertexData.size(), subMesh.vertexData.data(), GL_STATIC_DRAW);

  // 3. 创建EBO并上传数据
  glGenBuffers(1, &EBO);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER,
               subMesh.indices.size() * sizeof(uint32_t),
               subMesh.indices.data(),
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
  handle.vertexCount = static_cast<uint32_t>(subMesh.vertexData.size()) / subMesh.layout.stride;
  handle.indexCount = static_cast<uint32_t>(subMesh.indices.size());

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