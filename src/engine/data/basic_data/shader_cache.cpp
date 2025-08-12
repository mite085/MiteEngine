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
  // 1. 生成唯一缓存键
  const std::string key = GenerateCacheKey(vertexPath, fragmentPath, geometryPath);

  // 2. 线程安全访问缓存
  std::lock_guard<std::mutex> lock(m_Mutex);

  // 3. 检查缓存是否存在且未过期
  if (auto it = m_Cache.find(key); it != m_Cache.end()) {
    if (auto shader = it->second.lock()) {
      return shader;  // 返回缓存的可用Shader
    }
    // 弱引用已失效，移除旧条目
    m_Cache.erase(it);
  }

  // 4. 创建新Shader并加入缓存
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
    throw std::runtime_error("ShaderCache: Failed to compile shader - " + std::string(e.what()));
  }

  m_Cache[key] = shader;  // 存储弱引用
  return shader;
}

void ShaderCache::Clear()
{
  std::lock_guard<std::mutex> lock(m_Mutex);
  m_Cache.clear();  // 弱引用会自动释放，无需额外操作
}

std::string ShaderCache::GenerateCacheKey(const std::string &vertexPath,
                                          const std::string &fragmentPath,
                                          const std::string &geometryPath) const
{
  // 简单拼接路径作为键（确保路径规范化）
  std::string key = vertexPath + "|" + fragmentPath;
  if (!geometryPath.empty()) {
    key += "|" + geometryPath;
  }

  // TODO：对路径进行标准化处理（如统一转为绝对路径）
  // std::filesystem::canonical(vertexPath)

  return key;
}
};
