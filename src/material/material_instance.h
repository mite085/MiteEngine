#ifndef MITE_MATERIAL_INSTANCE
#define MITE_MATERIAL_INSTANCE

#include "shader.h"
#include "texture.h"

namespace mite {
// Uniform值类型擦除存储
struct UniformValue {
  enum class Type {
    None,
    Float,
    Int,
    Vector2,
    Vector3,
    Vector4,
    Matrix3,
    Matrix4,
    IntArray,
    FloatArray,
    Vector3Array
  };

  Type type;
  union {
    float asFloat;
    int asInt;
    glm::vec2 asVec2;
    glm::vec3 asVec3;
    glm::vec4 asVec4;
    glm::mat3 asMat3;
    glm::mat4 asMat4;
    std::pair<int *, size_t> asIntArray;  // <ptr, count> pair
    std::pair<float *, size_t> asFloatArray;
    std::pair<glm::vec3 *, size_t> asVec3Array;
  };

  // ---- 构造函数 ----
  UniformValue() : type(Type::None), asFloat(0.0) {}
  UniformValue(float v) : type(Type::Float), asFloat(v) {}
  UniformValue(int v) : type(Type::Int), asInt(v) {}
  UniformValue(const glm::vec2 &v) : type(Type::Vector2), asVec2(v) {}
  UniformValue(const glm::vec3 &v) : type(Type::Vector3), asVec3(v) {}
  UniformValue(const glm::vec4 &v) : type(Type::Vector4), asVec4(v) {}
  UniformValue(const glm::mat3 &v) : type(Type::Matrix3), asMat4(v) {}
  UniformValue(const glm::mat4 &v) : type(Type::Matrix4), asMat4(v) {}
  UniformValue(const std::pair<int *, size_t> &v);
  UniformValue(const std::pair<float *, size_t> &v);
  UniformValue(const std::pair<glm::vec3 *, size_t> &v);

  // ---- 析构函数 ----
  ~UniformValue();

  // ---- 禁用拷贝（避免浅拷贝问题）----
  UniformValue(const UniformValue &) = delete;
  UniformValue &operator=(const UniformValue &) = delete;
  // ---- 移动构造 ----
  UniformValue(UniformValue &&other) noexcept;
  UniformValue &operator=(UniformValue &&other) noexcept;
  
};

/**
 * @brief 材质实例（运行时绑定具体Shader和参数）
 * @note 职责：
 * 1. 关联Material逻辑参数与具体Shader
 * 2. 管理Uniform状态和纹理绑定
 * 3. 提供渲染前的Apply接口
 */
class MaterialInstance {
 public:
  explicit MaterialInstance(std::shared_ptr<Shader> shader);

  // ---- 参数设置 ----
  void SetFloat(const std::string &name, float value);
  void SetInt(const std::string &name, int value);
  void SetVector2(const std::string &name, const glm::vec2 &value);
  void SetVector3(const std::string &name, const glm::vec3 &value);
  void SetVector4(const std::string &name, const glm::vec4 &value);
  void SetMatrix3(const std::string &name, const glm::mat3 &value);
  void SetMatrix4(const std::string &name, const glm::mat4 &value);

  // ---- 参数数组设置 ----
  void SetIntArray(const std::string &name, const int *values, size_t count);
  void SetFloatArray(const std::string &name, const float *values, size_t count);
  void SetVector3Array(const std::string &name, const glm::vec3 *values, size_t count);

  // ---- 纹理设置 ----
  void SetTexture(const std::string &name, std::shared_ptr<Texture> texture);
  void SetTextureArray(const std::string &name,
                       const std::vector<std::shared_ptr<Texture>> &textures);

  // ---- 状态控制 ----
  /**
   * @brief 应用材质到渲染管线（绑定Shader+上传Uniforms+绑定纹理）
   * @param overrideShader 可选覆盖Shader（用于特殊渲染效果）
   */
  void Apply(Shader *overrideShader = nullptr) const;

  // ---- 属性访问 ----
  std::shared_ptr<Shader> GetShader() const
  {
    return m_Shader;
  }
  const auto &GetTextures() const
  {
    return m_Textures;
  }

 private:
  std::shared_ptr<Shader> m_Shader;                                      // 关联的Shader程序
  std::unordered_map<std::string, UniformValue> m_Uniforms;              // Uniform值存储
  std::unordered_map<std::string, std::shared_ptr<Texture>> m_Textures;  // 纹理绑定
  std::unordered_map<std::string, std::vector<std::shared_ptr<Texture>>>
      m_TextureArrays;  // 纹理数组
};
};  // namespace mite

#endif
