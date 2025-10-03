#include "shader_cache.h"

namespace mite {
ShaderCache &ShaderCache::Get()
{
  static ShaderCache instance;  // 线程安全的单例（C++11保证）
  return instance;
}

std::shared_ptr<OpenGLShader> ShaderCache::GetOpenGLShader(const std::string &vertexPath,
                                                           const std::string &fragmentPath,
                                                           const std::string &geometryPath)
{
  // 1. 生成唯一缓存键（使用规范化路径）
  const std::string key = GenerateCacheKey(vertexPath, fragmentPath, geometryPath);

  // 2. 线程安全访问缓存
  std::lock_guard<std::mutex> lock(m_Mutex);

  // 3. 检查缓存是否存在且未过期
  if (auto it = m_Cache.find(key); it != m_Cache.end()) {
    if (auto shader = it->second.lock()) {
      LOG_DEBUG("ShaderCache: Using cached shader - {}", key);
      return shader;
    }
    // 弱引用已失效，移除旧条目
    LOG_DEBUG("ShaderCache: Removing expired shader - {}", key);
    m_Cache.erase(it);
  }

  // 4. 创建新Shader并加入缓存
  LOG_DEBUG("ShaderCache: Compiling new shader - {}", key);
  auto shader = std::make_shared<OpenGLShader>();
  try {
    if (geometryPath.empty()) {
      shader->LoadFromFile(vertexPath.c_str(), fragmentPath.c_str());
    }
    else {
      shader->LoadFromFile(vertexPath.c_str(), fragmentPath.c_str(), geometryPath.c_str());
    }
  }
  catch (const std::exception &e) {
    LOG_ERROR("ShaderCache: Failed to compile shader {} - {}", key, e.what());
    throw std::runtime_error("ShaderCache: Failed to compile shader - " + std::string(e.what()));
  }
  m_Cache[key] = shader;
  LOG_DEBUG("ShaderCache: Successfully cached shader - {}", key);
  return shader;
}

void ShaderCache::Clear()
{
  std::lock_guard<std::mutex> lock(m_Mutex);
  size_t count = m_Cache.size();
  m_Cache.clear();
  LOG_DEBUG("ShaderCache: Cleared {} shader entries", count);
}

std::string ShaderCache::GenerateCacheKey(const std::string &vertexPath,
                                          const std::string &fragmentPath,
                                          const std::string &geometryPath) const
{
  // 使用filesystem规范化路径，确保路径一致性
  std::filesystem::path vPath(vertexPath);
  std::filesystem::path fPath(fragmentPath);

  // 构建规范化路径键
  std::string key = std::filesystem::weakly_canonical(vPath).string() + "|" +
                    std::filesystem::weakly_canonical(fPath).string();

  if (!geometryPath.empty()) {
    std::filesystem::path gPath(geometryPath);
    key += "|" + std::filesystem::weakly_canonical(gPath).string();
  }
  return key;
}
};
