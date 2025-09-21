#ifndef MITE_MATERIAL_PARAM_VARIANT
#define MITE_MATERIAL_PARAM_VARIANT

#include "basic_data/texture.h"
#include "headers/headers.h"

namespace mite {
/**
 * @brief 统一材质参数变体类型
 * @特点：
 * 1. 使用std::variant实现类型安全存储
 * 2. 支持所有GLSL标准Uniform类型
 * 3. 自动内存管理（特别是数组类型）
 * 4. 提供便捷的类型检查和访问接口
 */
class UniformVariant {
 public:
  // ---- 支持的参数类型 ----
  using VariantType = std::variant<std::monostate,          // 空状态（替代None）
                                   bool,                    // bool (自动转换为int)
                                   int,                     // int
                                   unsigned int,            // uint
                                   float,                   // float
                                   glm::vec2,               // vec2
                                   glm::vec3,               // vec3
                                   glm::vec4,               // vec4
                                   glm::mat3,               // mat3
                                   glm::mat4,               // mat4
                                   std::vector<int>,        // int[]
                                   std::vector<float>,      // float[]
                                   std::vector<glm::vec3>,  // vec3[]
                                   TextureGPUHandle         // 纹理类型
                                   >;

  // ---- 类型枚举 ----
  enum class Type {
    None,
    Bool,
    Int,
    UInt,
    Float,
    Vector2,
    Vector3,
    Vector4,
    Matrix3,
    Matrix4,
    IntArray,
    FloatArray,
    Vector3Array,
    Texture
  };

  // ---- 构造函数 ----
  UniformVariant() = default;

  // 通用构造函数（支持所有variant类型）
  template<typename T> UniformVariant(T &&value) : m_Data(std::forward<T>(value))
  {
    static_assert(std::is_constructible_v<VariantType, T>, "Invalid uniform type");
  }

  // ---- 类型查询 ----
  Type GetType() const;

  // 类型检查
  template<typename T> bool Is() const
  {
    return std::holds_alternative<T>(m_Data);
  }

  // ---- 值获取（安全版）----
  template<typename T> bool TryGet(T &out) const
  {
    if (const T *ptr = std::get_if<T>(&m_Data)) {
      out = *ptr;
      return true;
    }
    return false;
  }

  // ---- 值获取（非安全版）----
  template<typename T> const T &Get() const
  {
    return std::get<T>(m_Data);
  }

  const VariantType &GetVariant() const
  {
    return m_Data;
  }

  // ---- 转换为旧UniformValue兼容接口 ----
  // 用于MaterialInstance的调用

  // 获取float值（失败返回默认值）
  float GetFloat(float defaultValue = 0.0f) const;

  // 获取int值（失败返回默认值）
  int GetInt(int defaultValue = 0) const;

  // 获取数组指针和长度（兼容旧接口）
  template<typename T> std::pair<const T *, size_t> GetArray() const;

  // ---- 辅助方法 ----
  std::string GetTypeName() const;

  /**
   * @brief 变量转换为Shader的string工具
   *
   * 注意：
   * 原则上这个方法应当由Renderer模块负责，Material模块
   * 使用该方法可以更方便的从MaterialTemplate材质模板中
   * 派生新的材质。
   */
  std::string ToShaderString() const;

 private:
  VariantType m_Data;
};

template<typename T> inline std::pair<const T *, size_t> UniformVariant::GetArray() const
{
  if constexpr (std::is_same_v<T, int>) {
    if (Is<std::vector<int>>()) {
      const auto &vec = Get<std::vector<int>>();
      return {vec.data(), vec.size()};
    }
  }
  else if constexpr (std::is_same_v<T, float>) {
    if (Is<std::vector<float>>()) {
      const auto &vec = Get<std::vector<float>>();
      return {vec.data(), vec.size()};
    }
  }
  else if constexpr (std::is_same_v<T, glm::vec3>) {
    if (Is<std::vector<glm::vec3>>()) {
      const auto &vec = Get<std::vector<glm::vec3>>();
      return {vec.data(), vec.size()};
    }
  }
  return {nullptr, 0};
}
};  // namespace mite

#endif
