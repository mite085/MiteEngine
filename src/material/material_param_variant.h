#ifndef MITE_MATERIAL_PARAM_VARIANT
#define MITE_MATERIAL_PARAM_VARIANT

#include "headers/headers.h"

namespace mite {
/**
 * @brief 材质参数变体类型（支持所有Shader可用的Uniform类型）
 * @职责：
 * 1. 类型安全地存储材质参数值
 * 2. 提供便捷的类型检查和访问接口
 * 3. 支持GLSL标准Uniform类型
 */
class MaterialParameterVariant {
 public:
  // ---- 支持的参数类型 ----
  using VariantType = std::variant<bool,                    // bool (自动转换为int)
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
                                   std::string              // 纹理路径（特殊处理）
                                   >;

  // ---- 构造函数（支持隐式转换）----
  MaterialParameterVariant() = default;

  template<typename T> MaterialParameterVariant(T &&value) : m_data(std::forward<T>(value))
  {
    static_assert(std::is_constructible_v<VariantType, T>, "Invalid param type");
  }



  // ---- 类型查询 ----
  /**
   * @brief 获取当前存储的类型枚举
   */
  enum class Type {
    Bool,
    Int,
    UInt,
    Float,
    Vec2,
    Vec3,
    Vec4,
    Mat3,
    Mat4,
    IntArray,
    FloatArray,
    Vec3Array,
    String,
    Empty
  };
  Type GetType() const;

  /**
   * @brief 检查是否持有特定类型
   */
  template<typename T> bool Is() const
  {
    return std::holds_alternative<T>(m_data);
  }

  // ---- 值获取（安全版）----
  /**
   * @brief 尝试获取值（失败返回false）
   */
  template<typename T> bool TryGet(T &out) const
  {
    if (const T *ptr = std::get_if<T>(&m_data)) {
      out = *ptr;
      return true;
    }
    return false;
  }

  // ---- 值获取（非安全版）----
  /**
   * @brief 强制获取值（类型不匹配时抛出异常）
   * @throws std::bad_variant_access
   */
  template<typename T> const T &Get() const
  {
    return std::get<T>(m_data);
  }

  // ---- 辅助方法 ----
  /**
   * @brief 获取类型名称（调试用）
   */
  std::string GetTypeName() const;

  /**
   * @brief 转换为Shader兼容的字符串表示（如"vec3(1.0, 0.0, 0.0)"）
   */
  std::string ToShaderString() const;

 private:
  VariantType m_data;
};

};

#endif
