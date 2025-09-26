#ifndef MITE_MATERIAL_TEMPLATE
#define MITE_MATERIAL_TEMPLATE

#include "material_instance.h"

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
  virtual ~MaterialTemplate() = default;

  // ---- 核心接口 ----
  /**
   * @brief 创建材质实例
   * @return 共享指针管理的MaterialInstance对象
   * @note 实例会继承模板的默认参数，但允许运行时修改
   */
  virtual std::unique_ptr<MaterialInstance> CreateInstance() const = 0;

  /**
   * @brief 应用默认参数到材质实例
   * @param instance 目标材质实例
   * @note 用于初始化或重置实例参数
   */
  virtual void ApplyParameters(MaterialInstance &instance) const = 0;

  // ---- 通用属性 ----
  void SetName(const std::string &name)
  {
    m_Name = name;
  }
  const std::string &GetName() const
  {
    return m_Name;
  }

  /**
   * @brief 获取材质类型标识（用于运行时类型检查）
   * @return 字符串类型标识（如"PBR"、"Phong"）
   */
  virtual std::string GetMaterialType() const = 0;

  /**
   * @brief 获取材质类型标识--静态模板方法
   * @return 字符串类型标识
   */
  template<typename T> static std::string GetMaterialTypeStatic()
  {
    static_assert(std::is_base_of<MaterialTemplate, T>::value, "Must inherit from Material");
    return T::StaticType();
  }

 protected:
  std::string m_Name = "Unnamed_Material";  // 材质名称（用于调试和UI显示）
};
};

#endif
