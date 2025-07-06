#include "material_param_variant.h"

namespace mite {
MaterialParameterVariant::Type MaterialParameterVariant::GetType() const
{
  if (m_data.valueless_by_exception())
    return Type::Empty;

  // 类型映射表
  switch (m_data.index()) {
    case 0:
      return Type::Bool;
    case 1:
      return Type::Int;
    case 2:
      return Type::UInt;
    case 3:
      return Type::Float;
    case 4:
      return Type::Vec2;
    case 5:
      return Type::Vec3;
    case 6:
      return Type::Vec4;
    case 7:
      return Type::Mat3;
    case 8:
      return Type::Mat4;
    case 9:
      return Type::IntArray;
    case 10:
      return Type::FloatArray;
    case 11:
      return Type::Vec3Array;
    case 12:
      return Type::String;
    default:
      return Type::Empty;
  }
}

std::string MaterialParameterVariant::GetTypeName() const
{
  // 类型名称映射表
  switch (GetType()) {
    case Type::Bool:
      return "bool";
    case Type::Int:
      return "int";
    case Type::UInt:
      return "uint";
    case Type::Float:
      return "float";
    case Type::Vec2:
      return "vec2";
    case Type::Vec3:
      return "vec3";
    case Type::Vec4:
      return "vec4";
    case Type::Mat3:
      return "mat3";
    case Type::Mat4:
      return "mat4";
    case Type::IntArray:
      return "int[]";
    case Type::FloatArray:
      return "float[]";
    case Type::Vec3Array:
      return "vec3[]";
    case Type::String:
      return "string";
    default:
      return "empty";
  }
}

std::string MaterialParameterVariant::ToShaderString() const
{
  return std::visit(
      [](auto &&arg) -> std::string {
        using T = std::decay_t<decltype(arg)>;

        if constexpr (std::is_same_v<T, bool>) {
          return arg ? "true" : "false";
        }
        else if constexpr (std::is_same_v<T, int> || std::is_same_v<T, unsigned int>) {
          return std::to_string(arg);
        }
        else if constexpr (std::is_same_v<T, float>) {
          // 保留3位小数
          std::string s = std::to_string(arg);
          s.erase(s.find_last_not_of('0') + 1, std::string::npos);
          if (s.back() == '.')
            s += "0";
          return s;
        }
        else if constexpr (std::is_same_v<T, glm::vec2>) {
          return "vec2(" + ToShaderString(arg.x) + ", " + ToShaderString(arg.y) + ")";
        }
        else if constexpr (std::is_same_v<T, glm::vec3>) {
          return "vec3(" + ToShaderString(arg.x) + ", " + ToShaderString(arg.y) + ", " +
                 ToShaderString(arg.z) + ")";
        }
        else if constexpr (std::is_same_v<T, glm::vec4>) {
          return "vec4(" + ToShaderString(arg.x) + ", " + ToShaderString(arg.y) + ", " +
                 ToShaderString(arg.z) + ", " + ToShaderString(arg.w) + ")";
        }
        else if constexpr (std::is_same_v<T, std::string>) {
          return "\"" + arg + "\"";  // 纹理路径特殊处理
        }
        else {
          throw std::runtime_error("Unsupported type for shader string conversion");
        }
      },
      m_data);
}
};
