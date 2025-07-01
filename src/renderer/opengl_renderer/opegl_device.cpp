#include "opengl_renderer/opegl_device.h"

namespace mite {
// ------------------------ 构造函数/析构函数 ------------------------
OpenGLDevice::OpenGLDevice()
{
  // 初始化OpenGL扩展（不由此处负责）
  // if (glfwInit() != GLFW_TRUE) {
  //  LOG_ERROR("Failed to initialize GLFW");
  //  throw std::runtime_error("GLFW init failed");
  //}
  // LOG_INFO("OpenGL device initialized.");
}

OpenGLDevice::~OpenGLDevice()
{
  // 防御性检查：确保所有资源已释放
  if (!activeTextures_.empty()) {
    LOG_WARN("{} textures not released on shutdown", activeTextures_.size());
    for (auto &[id, _] : activeTextures_) {
      glDeleteTextures(1, &id);
    }
  }
  if (!activeModels_.empty()) {
    LOG_WARN("{} models not released on shutdown", activeModels_.size());
    for (auto &[id, _] : activeModels_) {
      glDeleteBuffers(1, &id);
    }
  }
}

// ------------------------ 纹理操作 ------------------------
TextureGPUHandle OpenGLDevice::CreateTexture(const TextureMetadata &meta, const void *data)
{
  GLuint textureID;
  glGenTextures(1, &textureID);
  glBindTexture(GL_TEXTURE_2D, textureID);

  // 设置纹理参数（TODO: 可根据meta扩展）
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

  // 记录活动纹理（调试用）
  activeTextures_[textureID] = meta;

  return {static_cast<uintptr_t>(textureID)};
}

void OpenGLDevice::DestroyTexture(TextureGPUHandle handle)
{
  if (!handle.apiHandle)
    return;

  GLuint textureID = static_cast<GLuint>(handle.apiHandle);
  glDeleteTextures(1, &textureID);
  activeTextures_.erase(textureID);
}

// ------------------------ 模型操作 ------------------------
ModelGPUHandle OpenGLDevice::CreateModel(const ModelMetadata &meta)
{
  if (meta.subMeshes.empty()) {
    LOG_ERROR("Model has no submeshes");
    return {};
  }

  // TODO：只处理第一个子网格（多子网格需扩展）
  const SubMeshData &subMesh = meta.subMeshes[0];

  ModelGPUHandle handle;
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

  // 4. 设置顶点属性指针（基于layout描述）
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
        // 其他属性处理...
    }
  }

  // 解绑VAO（安全做法）
  glBindVertexArray(0);

  // 记录GPU资源
  handle.vertexBuffer = static_cast<uintptr_t>(VBO);
  handle.indexBuffer = static_cast<uintptr_t>(EBO);
  handle.vertexCount = uint32_t(subMesh.vertexData.size()) / subMesh.layout.stride;
  handle.indexCount = uint32_t(subMesh.indices.size());

  // 调试追踪
  activeModels_[VAO] = meta;

  return handle;
}

void OpenGLDevice::DestroyModel(ModelGPUHandle handle)
{
  if (!handle.vertexBuffer || !handle.indexBuffer)
    return;

  GLuint vbo = static_cast<GLuint>(handle.vertexBuffer);
  GLuint ebo = static_cast<GLuint>(handle.indexBuffer);

  glDeleteBuffers(1, &vbo);
  glDeleteBuffers(1, &ebo);

  // TODO：实际项目中需要同时删除关联的VAO
  // 此处简化处理，需根据实际架构调整
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
};  // namespace mite