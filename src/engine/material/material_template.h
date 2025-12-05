#ifndef MITE_MATERIAL_TEMPLATE
#define MITE_MATERIAL_TEMPLATE

#include "basic_instance/material_instance.h"
#include "basic_type/asset_type.h"
#include "basic_type/material_type.h"

namespace mite {
/**
 * @brief 材质模板的抽象基类
 * @note 职责：
 * 1. 定义材质的通用接口（创建实例、应用参数）
 * 2. 提供基础属性（名称、类型标识等）
 * 3. 派生类需实现具体材质类型的逻辑（如PBR、Phong）
 */
class MaterialTemplate {
 public:
  MaterialTemplate();
  virtual ~MaterialTemplate() = default;

  // ---- 核心接口 ----
  /**
   * @brief 创建空的材质实例
   * @return 材质实例
   * @note 实例直接调用ApplyDefaultParams
   */
  std::shared_ptr<MaterialInstance> CreateInstance();
  /**
   * @brief 从源数据创建材质实例
   * @param sourceData 材质源数据（包含所有渲染所需信息）
   * @return 共享指针管理的MaterialInstance对象
   * @note 实例会直接从sourceData解析参数，不调用ApplyDefaultParams
   */
  virtual std::shared_ptr<MaterialInstance> CreateInstance(
      const MaterialSourceData &sourceData) const;
  /**
   * @brief 获取材质类型标识（用于运行时类型检查）
   * @return 类型标识
   */
  virtual MaterialType GetMaterialType() const = 0;
  virtual std::string GetMaterialTypeName() const = 0;
  /**
   * @brief 获取材质类型标识--静态模板方法
   * @return 字符串类型标识
   */
  template<typename T> static MaterialType GetMaterialTypeStatic()
  {
    static_assert(std::is_base_of<MaterialTemplate, T>::value, "Must inherit from Material");
    return T::StaticType();
  }

  // ---- 通用属性 ----
  void SetName(const std::string &name);
  const std::string &GetName() const;

 protected:
  // ---- 通用数据获取工具方法（供派生类使用） ----
  /**
   * @brief 从源数据获取参数值（带默认值）
   * @tparam T 参数类型
   * @param sourceData 源数据
   * @param key 参数键名
   * @param defaultValue 默认值
   * @return 参数值或默认值
   */
  template<typename T>
  static T GetParameter(const MaterialSourceData &sourceData,
                        const std::string &key,
                        const T &defaultValue)
  {
    auto it = sourceData.parameters.find(key);
    if (it != sourceData.parameters.end() && it->second.Is<T>()) {
      return it->second.Get<T>();
    }
    return defaultValue;
  }
  /**
   * @brief 从源数据获取纹理槽位
   * @param sourceData 源数据
   * @param slotName 纹理槽位名称
   * @return 纹理槽位指针，如果不存在返回nullptr
   */
  static const TextureGPUSlot *GetTextureSlot(const MaterialSourceData &sourceData,
                                              const std::string &slotName);
  /**
   * @brief 检查源数据是否包含特定参数
   * @param sourceData 源数据
   * @param key 参数键名
   * @return 是否包含该参数
   */
  static bool HasParameter(const MaterialSourceData &sourceData, const std::string &key);
  /**
   * @brief 检查源数据是否包含特定纹理槽位
   * @param sourceData 源数据
   * @param slotName 纹理槽位名称
   * @return 是否包含该纹理槽位
   */
  static bool HasTextureSlot(const MaterialSourceData &sourceData, const std::string &slotName);

  /**
   * @brief 获取渲染属性（供派生类使用）
   */
  static AlphaMode GetAlphaMode(const MaterialSourceData &sourceData);
  static float GetAlphaCutoff(const MaterialSourceData &sourceData);
  static bool IsDoubleSided(const MaterialSourceData &sourceData);
  /**
   * @brief 材质数据填充方法（供派生类使用）
   */
  static void FillMaterialDataFromSource(MaterialUniformBuffer &materialData,
                                         const MaterialSourceData &sourceData);
  /**
   * @brief 使用默认值初始化材质实例
   */
  static void InitializeMaterialInstance(std::shared_ptr<MaterialInstance> instance,
                                  const MaterialSourceData &sourceData);
  /**
   * @brief 纹理设置方法
   */
  static void SetupMaterialTextures(std::shared_ptr<MaterialInstance> instance,
                                    const MaterialSourceData &sourceData);

  // ---- 默认值创建 ----
  /**
   * @brief 创建默认值
   */
  MaterialSourceData CreateDefaultSourceData() const;

  virtual glm::vec4 GetDefaultBaseColor() const { return glm::vec4(0.8f, 0.8f, 0.8f, 1.0f); }
  virtual float GetDefaultMetallic() const { return 0.0f; }
  virtual float GetDefaultRoughness() const { return 1.0f; }
  virtual float GetDefaultAO() const { return 1.0f; }
  virtual glm::vec3 GetDefaultEmissionColor() const { return glm::vec3(0.0f); }
  virtual float GetDefaultEmissionIntensity() const { return 0.0f; }
  virtual float GetDefaultNormalScale() const { return 1.0f; }
  virtual float GetDefaultAlphaCutoff() const { return 0.5f; }
  virtual bool GetDefaultDoubleSided() const { return false; }
  virtual AlphaMode GetDefaultAlphaMode() const { return AlphaMode::OPAQUE; }  // ALPHA_MODE_OPAQUE

 protected:
  std::string m_Name = "Unnamed_Material";  // 材质名称（用于调试和UI显示）
  uint32_t m_DefaultInstanceCounter = 0;
};
};  // namespace mite

#endif
