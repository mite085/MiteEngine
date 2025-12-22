#include "material_param_variant.h"

namespace mite {
UniformVariant::Type UniformVariant::GetType() const {
  static const std::array<Type, 14> typeMap = {
      Type::None,          // monostate
      Type::Bool,          // bool
      Type::Int,           // int
      Type::UInt,          // uint
      Type::Float,         // float
      Type::Vector2,       // vec2
      Type::Vector3,       // vec3
      Type::Vector4,       // vec4
      Type::Matrix3,       // mat3
      Type::Matrix4,       // mat4
      Type::IntArray,      // int[]
      Type::FloatArray,    // float[]
      Type::Vector3Array,  // vec3[]
      Type::Texture,       // TextureAssetID
  };
  return typeMap[m_Data.index()];
}
float UniformVariant::GetFloat(float defaultValue) const {
  if (Is<float>()) return Get<float>();
  if (Is<int>()) return static_cast<float>(Get<int>());
  return defaultValue;
}
int UniformVariant::GetInt(int defaultValue) const {
  if (Is<int>()) return Get<int>();
  if (Is<bool>()) return Get<bool>() ? 1 : 0;
  return defaultValue;
}
std::string UniformVariant::GetTypeName() const

{
  static const char *typeNames[] = {
      "None",     "Bool",       "Int",          "UInt",    "Float",
      "Vector2",  "Vector3",    "Vector4",      "Matrix3", "Matrix4",
      "IntArray", "FloatArray", "Vector3Array", "Texture"};
  return typeNames[static_cast<int>(GetType())];
}
std::string UniformVariant::ToShaderString() const {
  // 定义一个局部辅助函数来处理递归调用
  auto ToStringHelper = [](const auto &arg) -> std::string {
    using T = std::decay_t<decltype(arg)>;

    if constexpr (std::is_same_v<T, bool>) {
      return arg ? "true" : "false";
    } else if constexpr (std::is_same_v<T, int> ||
                         std::is_same_v<T, unsigned int>) {
      return std::to_string(arg);
    } else if constexpr (std::is_same_v<T, float>) {
      // 保留3位小数
      std::string s = std::to_string(arg);
      s.erase(s.find_last_not_of('0') + 1, std::string::npos);
      if (s.back() == '.') s += "0";
      return s;
    } else if constexpr (std::is_same_v<T, glm::vec2>) {
      return "vec2(" + std::to_string(arg.x) + ", " + std::to_string(arg.y) +
             ")";
    } else if constexpr (std::is_same_v<T, glm::vec3>) {
      return "vec3(" + std::to_string(arg.x) + ", " + std::to_string(arg.y) +
             ", " + std::to_string(arg.z) + ")";
    } else if constexpr (std::is_same_v<T, glm::vec4>) {
      return "vec4(" + std::to_string(arg.x) + ", " + std::to_string(arg.y) +
             ", " + std::to_string(arg.z) + ", " + std::to_string(arg.w) + ")";
    } else {
      throw std::runtime_error("Unsupported type for shader string conversion");
    }
  };

  return std::visit(ToStringHelper, m_Data);
}
};  // namespace mite